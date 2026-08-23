/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Mounts
*/

#include "Astruino.h"

AstroMount *newMountObjectFromData(const AstroMountData *dataIn)
{
    if (!dataIn) { return nullptr; }

    switch (dataIn->id.object.classType) {
        case (aid_t)AstroMount::Mount:
            return new AstroMount(dataIn);
        default:
            return nullptr;
    }
}

AstroMount::AstroMount(Astro_MountType mountType, aposi_t positionIndex)
    : AstroObject(AstroIdentity(mountType, positionIndex)), classType(Mount), _mountType(mountType), _observer(), _targetType(Astro_TargetType_M42),
      _primaryAxis(ASTRO_MOUNT_AXIS_RATE_DEGPS), _secondaryAxis(ASTRO_MOUNT_AXIS_RATE_DEGPS),
      _parkPrimary(0.0), _parkSecondary(0.0), _guidePrimary(0.0), _guideSecondary(0.0),
      _tracking(false), _parking(false), _parked(true), _limitHit(false), _primaryDriver(this, 0), _secondaryDriver(this, 1),
      _lastUpdate(0)
{ ; }

AstroMount::AstroMount(const AstroMountData *dataIn)
    : AstroObject(dataIn), classType(dataIn ? (decltype(Mount))dataIn->id.object.classType : Unknown),
      _mountType(dataIn ? (Astro_MountType)dataIn->id.object.objType : Astro_MountType_Unknown), _observer(),
      _targetType(dataIn ? dataIn->targetType : Astro_TargetType_M42),
      _primaryAxis(dataIn ? dataIn->primaryAxisRate : ASTRO_MOUNT_AXIS_RATE_DEGPS),
      _secondaryAxis(dataIn ? dataIn->secondaryAxisRate : ASTRO_MOUNT_AXIS_RATE_DEGPS),
      _parkPrimary(dataIn ? dataIn->parkPrimary : 0.0), _parkSecondary(dataIn ? dataIn->parkSecondary : 0.0),
      _guidePrimary(0.0), _guideSecondary(0.0), _tracking(false), _parking(false), _parked(true), _limitHit(false),
      _primaryDriver(this, 0), _secondaryDriver(this, 1), _lastUpdate(0)
{
    if (dataIn) {
        _primaryAxis.minimumDegrees = dataIn->primaryMinimum;
        _primaryAxis.maximumDegrees = dataIn->primaryMaximum;
        _primaryAxis.limitsEnabled = dataIn->primaryLimitsEnabled;
        _secondaryAxis.minimumDegrees = dataIn->secondaryMinimum;
        _secondaryAxis.maximumDegrees = dataIn->secondaryMaximum;
        _secondaryAxis.limitsEnabled = dataIn->secondaryLimitsEnabled;
    }
}

void AstroMount::setObserver(const AstroObserver &observer)
{
    _observer = observer;
}

void AstroMount::setTarget(Astro_TargetType targetType)
{
    _targetType = targetType;
    clearGuideOffset();
}

void AstroMount::setAxisRates(double primaryDegreesPerSecond, double secondaryDegreesPerSecond)
{
    _primaryAxis.maxRateDegreesPerSecond = primaryDegreesPerSecond > 0.0 ? primaryDegreesPerSecond : 0.0;
    _secondaryAxis.maxRateDegreesPerSecond = secondaryDegreesPerSecond > 0.0 ? secondaryDegreesPerSecond : 0.0;
}

void AstroMount::setAxisPosition(uint8_t axisIndex, double positionDegrees)
{
    AstroAxisState *axis = axisIndex == 0 ? &_primaryAxis : axisIndex == 1 ? &_secondaryAxis : nullptr;
    if (!axis) { return; }

    axis->positionDegrees = (_mountType == Astro_MountType_AltAzimuth && axisIndex == 0)
        ? wrapBy360<double>(positionDegrees) : positionDegrees;

    if (!_parking && !_tracking) { _parked = isAtParkPosition(); }
}

void AstroMount::setAxisLimits(uint8_t axisIndex, double minimumDegrees, double maximumDegrees)
{
    AstroAxisState *axis = axisIndex == 0 ? &_primaryAxis : axisIndex == 1 ? &_secondaryAxis : nullptr;
    if (!axis || maximumDegrees < minimumDegrees) { return; }

    axis->minimumDegrees = minimumDegrees;
    axis->maximumDegrees = maximumDegrees;
    axis->limitsEnabled = true;
    _limitHit = (_primaryAxis.limitsEnabled && !_primaryAxis.withinLimits(_primaryAxis.positionDegrees)) ||
                (_secondaryAxis.limitsEnabled && !_secondaryAxis.withinLimits(_secondaryAxis.positionDegrees));
}

void AstroMount::clearAxisLimits(uint8_t axisIndex)
{
    AstroAxisState *axis = axisIndex == 0 ? &_primaryAxis : axisIndex == 1 ? &_secondaryAxis : nullptr;
    if (!axis) { return; }

    axis->limitsEnabled = false;
    _limitHit = (_primaryAxis.limitsEnabled && !_primaryAxis.withinLimits(_primaryAxis.positionDegrees)) ||
                (_secondaryAxis.limitsEnabled && !_secondaryAxis.withinLimits(_secondaryAxis.positionDegrees));
}

void AstroMount::setParkPosition(double primaryDegrees, double secondaryDegrees)
{
    _parkPrimary = primaryDegrees;
    _parkSecondary = secondaryDegrees;
}

void AstroMount::setAxisDriver(uint8_t axisIndex, SharedPtr<AstroAxisDriver> driver)
{
    if (axisIndex == 0) { _primaryDriver.setObject(driver); }
    else if (axisIndex == 1) { _secondaryDriver.setObject(driver); }
}

SharedPtr<AstroAxisDriver> AstroMount::getAxisDriver(uint8_t axisIndex) const
{
    return axisIndex == 0 ? _primaryDriver.getObject() : axisIndex == 1 ? _secondaryDriver.getObject() : nullptr;
}

void AstroMount::park()
{
    _tracking = false;
    if (_parked || _parking) { return; }

    _parking = true;
    _limitHit = false;
    clearGuideOffset();
    applyAxisTarget(0, _parkPrimary);
    applyAxisTarget(1, _parkSecondary);
}

void AstroMount::unpark()
{
    _parking = false;
    _parked = false;
    _limitHit = false;
}

void AstroMount::track()
{
    if (!_parked && !_parking && !_limitHit) { _tracking = true; }
}

void AstroMount::pulseGuide(uint8_t axisIndex, Astro_DirectionMode direction, uint32_t durationMillis, double rateMultiple)
{
    if (axisIndex > 1 || direction == Astro_DirectionMode_Stop || rateMultiple <= 0.0) { return; }

    double offset = ASTRO_MOUNT_SIDEREAL_RATE_DEGPS * rateMultiple * ((double)durationMillis / 1000.0);
    if (direction == Astro_DirectionMode_Reverse) { offset = -offset; }

    if (axisIndex == 0) { _guidePrimary += offset; }
    else { _guideSecondary += offset; }
}

void AstroMount::clearGuideOffset(uint8_t axisIndex)
{
    if (axisIndex == 0 || axisIndex == 0xff) { _guidePrimary = 0.0; }
    if (axisIndex == 1 || axisIndex == 0xff) { _guideSecondary = 0.0; }
}

bool AstroMount::applyAxisTarget(uint8_t axisIndex, double targetDegrees)
{
    AstroAxisState *axis = axisIndex == 0 ? &_primaryAxis : axisIndex == 1 ? &_secondaryAxis : nullptr;
    if (!axis) { return false; }

    if (axis->limitsEnabled && !axis->withinLimits(targetDegrees)) {
        targetDegrees = targetDegrees < axis->minimumDegrees ? axis->minimumDegrees : axis->maximumDegrees;
        _limitHit = true;
        _tracking = false;
    }

    axis->targetDegrees = targetDegrees;
    return !_limitHit;
}

void AstroMount::updateTarget(int64_t unixTime, double elapsedSeconds)
{
    if (_mountType == Astro_MountType_SingleAxis) {
        applyAxisTarget(0, _primaryAxis.targetDegrees + (ASTRO_MOUNT_SIDEREAL_RATE_DEGPS * elapsedSeconds) + _guidePrimary);
        _guidePrimary = 0.0;
        return;
    }

    const AstroTargetsLibData *target = AstroTargetsLib.checkoutTargetsData(_targetType);
    if (!target) { return; }
    AstroEquatorialCoordinates equatorial = target->getCoordinates(unixTime);
    AstroTargetsLib.returnTargetsData(target);

    if (_mountType == Astro_MountType_AltAzimuth) {
        AstroHorizontalCoordinates horizontal = astroEquatorialToHorizontal(equatorial, _observer, unixTime);
        applyAxisTarget(0, wrapBy360<double>(horizontal.azimuthDegrees + _guidePrimary));
        applyAxisTarget(1, horizontal.altitudeDegrees + _guideSecondary);
    } else {
        double localSidereal = astroLocalSiderealDegrees(unixTime, _observer.longitudeDegrees);
        applyAxisTarget(0, wrapBy180Neg180<double>(localSidereal - equatorial.rightAscensionHours * 15.0 + _guidePrimary));
        applyAxisTarget(1, equatorial.declinationDegrees + _guideSecondary);
    }
}

bool AstroMount::isAtParkPosition(double toleranceDegrees) const
{
    double primaryDelta = _parkPrimary - _primaryAxis.positionDegrees;
    if (_mountType == Astro_MountType_AltAzimuth) { primaryDelta = wrapBy180Neg180<double>(primaryDelta); }

    if (fabs(primaryDelta) > toleranceDegrees) { return false; }
    return _mountType == Astro_MountType_SingleAxis ||
           fabs(_parkSecondary - _secondaryAxis.positionDegrees) <= toleranceDegrees;
}

bool AstroMount::updateAxisPosition(uint8_t axisIndex)
{
    AstroAxisState *axis = axisIndex == 0 ? &_primaryAxis : axisIndex == 1 ? &_secondaryAxis : nullptr;
    SharedPtr<AstroAxisDriver> driver = getAxisDriver(axisIndex);
    if (!axis || !driver) { return false; }

    double positionDegrees = 0.0;
    if (!driver->getPositionDegrees(&positionDegrees)) { return false; }

    setAxisPosition(axisIndex, positionDegrees);
    if (axis->limitsEnabled && !axis->withinLimits(axis->positionDegrees)) {
        _limitHit = true;
        _tracking = false;
        _parking = false;
        _parked = false;
    }
    return true;
}

void AstroMount::moveAxis(AstroAxisState *axis, double elapsedSeconds, bool wrappedAxis)
{
    if (!axis || elapsedSeconds <= 0.0 || axis->maxRateDegreesPerSecond <= 0.0) { return; }

    double delta = axis->targetDegrees - axis->positionDegrees;
    if (wrappedAxis) { delta = wrapBy180Neg180<double>(delta); }
    double maxStep = axis->maxRateDegreesPerSecond * elapsedSeconds;

    if (fabs(delta) <= maxStep) {
        axis->positionDegrees = axis->targetDegrees;
    } else {
        axis->positionDegrees += delta > 0.0 ? maxStep : -maxStep;
        if (wrappedAxis) { axis->positionDegrees = wrapBy360<double>(axis->positionDegrees); }
    }
}

void AstroMount::update()
{
    const millis_t now = nzMillis();
    const double elapsedSeconds = _lastUpdate ? (double)(now - _lastUpdate) / 1000.0 : 0.0;
    _lastUpdate = now;

    if (_tracking) { updateTarget((int64_t)unixNow(), elapsedSeconds); }
    if (_limitHit) { return; }

    SharedPtr<AstroAxisDriver> primaryDriver = getAxisDriver(0);
    SharedPtr<AstroAxisDriver> secondaryDriver = getAxisDriver(1);
    if (primaryDriver) {
        primaryDriver->setTargetDegrees(_primaryAxis.targetDegrees);
        primaryDriver->update();
    }
    if (secondaryDriver && _mountType != Astro_MountType_SingleAxis) {
        secondaryDriver->setTargetDegrees(_secondaryAxis.targetDegrees);
        secondaryDriver->update();
    }

    bool primaryFeedback = updateAxisPosition(0);
    bool secondaryFeedback = _mountType == Astro_MountType_SingleAxis ? true : updateAxisPosition(1);
    if (_limitHit) { return; }

    if (!primaryFeedback) { moveAxis(&_primaryAxis, elapsedSeconds, _mountType == Astro_MountType_AltAzimuth); }
    if (!secondaryFeedback && _mountType != Astro_MountType_SingleAxis) { moveAxis(&_secondaryAxis, elapsedSeconds); }

    if (_parking && isAtParkPosition()) {
        _parking = false;
        _parked = true;
    } else if (_parked && !isAtParkPosition()) {
        _parked = false;
    }
}

bool AstroMount::isAligned(double toleranceDegrees) const
{
    double primaryDelta = _primaryAxis.targetDegrees - _primaryAxis.positionDegrees;
    if (_mountType == Astro_MountType_AltAzimuth) { primaryDelta = wrapBy180Neg180<double>(primaryDelta); }

    if (fabs(primaryDelta) > toleranceDegrees) { return false; }
    return _mountType == Astro_MountType_SingleAxis ||
           fabs(_secondaryAxis.targetDegrees - _secondaryAxis.positionDegrees) <= toleranceDegrees;
}


AstroData *AstroMount::allocateData() const
{
    return _allocateDataForObjType((aid_t)AstroIdentity::Mount, (aid_t)classType);
}

void AstroMount::saveToData(AstroData *dataOut)
{
    AstroObject::saveToData(dataOut);
    if (!dataOut) { return; }

    AstroMountData *mountData = static_cast<AstroMountData *>(dataOut);
    mountData->id.object.classType = (aid_t)classType;
    mountData->targetType = _targetType;
    mountData->primaryAxisRate = _primaryAxis.maxRateDegreesPerSecond;
    mountData->secondaryAxisRate = _secondaryAxis.maxRateDegreesPerSecond;
    mountData->parkPrimary = _parkPrimary;
    mountData->parkSecondary = _parkSecondary;
    mountData->primaryMinimum = _primaryAxis.minimumDegrees;
    mountData->primaryMaximum = _primaryAxis.maximumDegrees;
    mountData->secondaryMinimum = _secondaryAxis.minimumDegrees;
    mountData->secondaryMaximum = _secondaryAxis.maximumDegrees;
    mountData->primaryLimitsEnabled = _primaryAxis.limitsEnabled;
    mountData->secondaryLimitsEnabled = _secondaryAxis.limitsEnabled;
}


AstroMountData::AstroMountData()
    : AstroObjectData(), targetType(Astro_TargetType_M42),
      primaryAxisRate(ASTRO_MOUNT_AXIS_RATE_DEGPS), secondaryAxisRate(ASTRO_MOUNT_AXIS_RATE_DEGPS),
      parkPrimary(0.0), parkSecondary(0.0), primaryMinimum(0.0), primaryMaximum(0.0),
      secondaryMinimum(0.0), secondaryMaximum(0.0), primaryLimitsEnabled(false), secondaryLimitsEnabled(false)
{
    _size = sizeof(*this);
    id.object.idType = (aid_t)AstroIdentity::Mount;
    id.object.objType = (aid_t)Astro_MountType_Unknown;
    id.object.posIndex = aposi_none;
    id.object.classType = (aid_t)AstroMount::Mount;
}

void AstroMountData::toJSONObject(JsonObject &objectOut) const
{
    AstroObjectData::toJSONObject(objectOut);
    objectOut["targetType"] = (int)targetType;
    objectOut["primaryAxisRate"] = primaryAxisRate;
    objectOut["secondaryAxisRate"] = secondaryAxisRate;
    objectOut["parkPrimary"] = parkPrimary;
    objectOut["parkSecondary"] = parkSecondary;
    if (primaryLimitsEnabled) {
        objectOut["primaryMinimum"] = primaryMinimum;
        objectOut["primaryMaximum"] = primaryMaximum;
        objectOut["primaryLimitsEnabled"] = primaryLimitsEnabled;
    }
    if (secondaryLimitsEnabled) {
        objectOut["secondaryMinimum"] = secondaryMinimum;
        objectOut["secondaryMaximum"] = secondaryMaximum;
        objectOut["secondaryLimitsEnabled"] = secondaryLimitsEnabled;
    }
}

void AstroMountData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroObjectData::fromJSONObject(objectIn);
    targetType = (Astro_TargetType)(objectIn["targetType"] | (int)targetType);
    primaryAxisRate = objectIn["primaryAxisRate"] | primaryAxisRate;
    secondaryAxisRate = objectIn["secondaryAxisRate"] | secondaryAxisRate;
    parkPrimary = objectIn["parkPrimary"] | parkPrimary;
    parkSecondary = objectIn["parkSecondary"] | parkSecondary;
    primaryMinimum = objectIn["primaryMinimum"] | primaryMinimum;
    primaryMaximum = objectIn["primaryMaximum"] | primaryMaximum;
    secondaryMinimum = objectIn["secondaryMinimum"] | secondaryMinimum;
    secondaryMaximum = objectIn["secondaryMaximum"] | secondaryMaximum;
    primaryLimitsEnabled = objectIn["primaryLimitsEnabled"] | primaryLimitsEnabled;
    secondaryLimitsEnabled = objectIn["secondaryLimitsEnabled"] | secondaryLimitsEnabled;
}
