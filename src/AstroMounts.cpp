/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Mounts
*/

#include "AstroMounts.h"
#include <math.h>

AstroMount::AstroMount(Astro_MountType mountType, aposi_t positionIndex)
    : AstroObject(AstroIdentity(mountType, positionIndex)), _mountType(mountType), _observer(), _targetId(Astro_Target_M42),
      _primaryAxis(ASTRO_MOUNT_AXIS_RATE_DEGPS), _secondaryAxis(ASTRO_MOUNT_AXIS_RATE_DEGPS), _stowPrimary(0.0), _stowSecondary(0.0), _tracking(false),
      _axisTargetCallback(nullptr), _axisTargetContext(nullptr)
{ ; }

AstroMount::AstroMount(const AstroObjectData *dataIn)
    : AstroObject(dataIn), _mountType(dataIn ? (Astro_MountType)dataIn->objType : Astro_MountType_Unknown), _observer(), _targetId(Astro_Target_M42),
      _primaryAxis(ASTRO_MOUNT_AXIS_RATE_DEGPS), _secondaryAxis(ASTRO_MOUNT_AXIS_RATE_DEGPS), _stowPrimary(0.0), _stowSecondary(0.0), _tracking(false),
      _axisTargetCallback(nullptr), _axisTargetContext(nullptr)
{ ; }

void AstroMount::setObserver(const AstroObserver &observer)
{
    _observer = observer;
}

void AstroMount::setTarget(Astro_TargetId targetId)
{
    _targetId = targetId;
}

void AstroMount::setAxisRates(double primaryDegreesPerSecond, double secondaryDegreesPerSecond)
{
    _primaryAxis.maxRateDegreesPerSecond = primaryDegreesPerSecond;
    _secondaryAxis.maxRateDegreesPerSecond = secondaryDegreesPerSecond;
}

void AstroMount::setStowPosition(double primaryDegrees, double secondaryDegrees)
{
    _stowPrimary = primaryDegrees;
    _stowSecondary = secondaryDegrees;
}

void AstroMount::setAxisTargetCallback(AxisTargetCallback callback, void *context)
{
    _axisTargetCallback = callback;
    _axisTargetContext = context;
}

void AstroMount::stow()
{
    _tracking = false;
    _primaryAxis.targetDegrees = _stowPrimary;
    _secondaryAxis.targetDegrees = _stowSecondary;
}

void AstroMount::track()
{
    _tracking = true;
}

void AstroMount::updateTarget(int64_t unixTime)
{
    const AstroTargetData *target = astroLib.checkoutTargetData(_targetId);
    if (!target) { return; }
    AstroEquatorialCoordinates equatorial = target->getCoordinates(unixTime);
    astroLib.returnTargetData(target);

    if (_mountType == Astro_MountType_AltAz) {
        AstroHorizontalCoordinates horizontal = astroEquatorialToHorizontal(equatorial, _observer, unixTime);
        _primaryAxis.targetDegrees = horizontal.azimuthDegrees;
        _secondaryAxis.targetDegrees = horizontal.altitudeDegrees;
    } else {
        double localSidereal = astroLocalSiderealDegrees(unixTime, _observer.longitudeDegrees);
        _primaryAxis.targetDegrees = astroNormalizeSignedDegrees(localSidereal - equatorial.rightAscensionHours * 15.0);
        _secondaryAxis.targetDegrees = equatorial.declinationDegrees;
    }
}

void AstroMount::moveAxis(AstroAxisState *axis, double elapsedSeconds)
{
    double delta = axis->targetDegrees - axis->positionDegrees;
    double maxStep = axis->maxRateDegreesPerSecond * elapsedSeconds;
    if (fabs(delta) <= maxStep) {
        axis->positionDegrees = axis->targetDegrees;
    } else {
        axis->positionDegrees += delta > 0.0 ? maxStep : -maxStep;
    }
}

void AstroMount::update(int64_t unixTime, double elapsedSeconds)
{
    if (_tracking) { updateTarget(unixTime); }
    if (_axisTargetCallback) {
        _axisTargetCallback(_axisTargetContext, 0, _primaryAxis.targetDegrees);
        _axisTargetCallback(_axisTargetContext, 1, _secondaryAxis.targetDegrees);
    }
    moveAxis(&_primaryAxis, elapsedSeconds);
    moveAxis(&_secondaryAxis, elapsedSeconds);
}

bool AstroMount::isAligned(double toleranceDegrees) const
{
    return fabs(_primaryAxis.targetDegrees - _primaryAxis.positionDegrees) <= toleranceDegrees &&
           fabs(_secondaryAxis.targetDegrees - _secondaryAxis.positionDegrees) <= toleranceDegrees;
}
