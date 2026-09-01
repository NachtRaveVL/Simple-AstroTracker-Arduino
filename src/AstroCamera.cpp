/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Observation Devices
*/

#include "Astruino.h"

AstroObject *newObservationDeviceObjectFromData(const AstroObservationDeviceData *dataIn)
{
    if (dataIn && !isValidType(dataIn->id.object.idType)) return nullptr;
    ASTRO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(AStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData() && dataIn->id.object.idType == (aid_t)AstroIdentity::ObservationDevice) {
        switch (dataIn->id.object.classType) {
            case (aid_t)AstroObservationDevice::CameraTrigger:
                return new AstroCameraTrigger(dataIn);
            default: break;
        }
    }

    return nullptr;
}

AstroCover::AstroCover(AstroObjInterface *parent)
    : AstroSubObject(parent), _position(0.0f), _target(0.0f), _travelRate(ASTRO_COVER_TRAVEL_RATE),
      _travelTimeout(ASTRO_COVER_TRAVEL_TIMEOUT_SECS), _travelElapsed(0.0),
      _openLimitActive(false), _closedLimitActive(false), _faulted(false),
      _actuator(parent ? parent : this), _openSensor(parent ? parent : this), _closedSensor(parent ? parent : this), _lastUpdate(nzMillis())
{ ; }

void AstroCover::open()
{
    if (_faulted) { return; }
    if (!isFPEqual(_target, 1.0f)) { _travelElapsed = 0.0; }
    _target = 1.0f;
}

void AstroCover::close()
{
    if (_faulted) { return; }
    if (!isFPEqual(_target, 0.0f)) { _travelElapsed = 0.0; }
    _target = 0.0f;
}

void AstroCover::stop()
{
    _target = _position;
    _travelElapsed = 0.0;
    applyActuatorPower(0.0f);
}

void AstroCover::setTravelRate(float fractionPerSecond)
{
    _travelRate = fractionPerSecond > 0.0f ? fractionPerSecond : 0.0f;
}

void AstroCover::setTravelTimeout(double seconds)
{
    _travelTimeout = seconds > 0.0 ? seconds : 0.0;
}

void AstroCover::setPosition(float position)
{
    _position = constrain(position, 0.0f, 1.0f);
    _target = _position;
    _travelElapsed = 0.0;
}

void AstroCover::clearFault()
{
    _faulted = false;
    _travelElapsed = 0.0;
    _lastUpdate = nzMillis();
}

bool AstroCover::isOpen() const
{
    if (_openSensor.isSet()) { return _openLimitActive; }
    return _position >= 1.0f - FLT_EPSILON;
}

bool AstroCover::isClosed() const
{
    if (_closedSensor.isSet()) { return _closedLimitActive; }
    return _position <= FLT_EPSILON;
}

bool AstroCover::isMoving() const
{
    return !_faulted && !isFPEqual(_target, _position);
}

bool AstroCover::pollLimitSensor(AstroSensorAttachment &sensor, bool *activeOut)
{
    if (!activeOut || !sensor.isSet()) { return false; }
    sensor.updateIfNeeded(true);
    const AstroSingleMeasurement &measurement = sensor.getMeasurement();
    if (!measurement.isSet()) { return false; }
    *activeOut = measurement.value > 0.5;
    return true;
}

void AstroCover::applyActuatorPower(float power)
{
    if (!_actuator.isSet()) { return; }
    if (isFPEqual(power, 0.0f)) {
        _actuator.disableActivation();
        return;
    }
    _actuator.setupActivation(power, (millis_t)-1, false);
    _actuator.enableActivation();
}

void AstroCover::update()
{
    const millis_t now = nzMillis();
    const double elapsedSeconds = _lastUpdate ? (double)(now - _lastUpdate) / 1000.0 : 0.0;
    _lastUpdate = now;

    bool openLimit = false;
    bool closedLimit = false;
    const bool hasOpenLimit = pollLimitSensor(_openSensor, &openLimit);
    const bool hasClosedLimit = pollLimitSensor(_closedSensor, &closedLimit);
    if (hasOpenLimit) { _openLimitActive = openLimit; }
    if (hasClosedLimit) { _closedLimitActive = closedLimit; }

    if (_openLimitActive && _closedLimitActive) {
        _faulted = true;
        applyActuatorPower(0.0f);
        return;
    }
    if (_openLimitActive) { _position = 1.0f; }
    else if (_closedLimitActive) { _position = 0.0f; }

    if (_faulted || isFPEqual(_target, _position)) {
        _travelElapsed = 0.0;
        applyActuatorPower(0.0f);
        return;
    }

    const float direction = _target > _position ? 1.0f : -1.0f;
    if ((direction > 0.0f && _openLimitActive) || (direction < 0.0f && _closedLimitActive)) {
        _position = direction > 0.0f ? 1.0f : 0.0f;
        _target = _position;
        _travelElapsed = 0.0;
        applyActuatorPower(0.0f);
        return;
    }

    _travelElapsed += elapsedSeconds;
    if (_travelTimeout > 0.0 && _travelElapsed >= _travelTimeout) {
        _faulted = true;
        applyActuatorPower(0.0f);
        return;
    }

    applyActuatorPower(direction);
    if (_travelRate <= 0.0f || elapsedSeconds <= 0.0) { return; }

    const float step = _travelRate * elapsedSeconds;
    if (direction > 0.0f) { _position = min(_target, _position + step); }
    else { _position = max(_target, _position - step); }

    if (isFPEqual(_target, _position)) {
        _position = _target;
        _travelElapsed = 0.0;
        applyActuatorPower(0.0f);
    }
}

void AstroCover::unresolveAny(AstroObject *object)
{
    _actuator.unresolveIf(object);
    _openSensor.unresolveIf(object);
    _closedSensor.unresolveIf(object);
    AstroSubObject::unresolveAny(object);
}

AstroCameraTrigger::AstroCameraTrigger(TriggerCallback callback, void *context, aposi_t positionIndex)
    : AstroObject(AstroIdentity(AstroIdentity::ObservationDevice, 0, positionIndex)), AstroObservationDevice(CameraTrigger),
      _callback(callback), _context(context), _ready(true), _capturing(false)
{ ; }

AstroCameraTrigger::AstroCameraTrigger(const AstroObservationDeviceData *dataIn)
    : AstroObject(dataIn), AstroObservationDevice(dataIn ? dataIn->id.object.classType : Unknown),
      _callback(nullptr), _context(nullptr), _ready(true), _capturing(false)
{ ; }

void AstroCameraTrigger::startObservation()
{
    if (!_ready || _capturing) { return; }
    _capturing = true;
    if (_callback) { _callback(_context, true); }
}

void AstroCameraTrigger::stopObservation()
{
    if (!_capturing) { return; }
    _capturing = false;
    if (_callback) { _callback(_context, false); }
}

void AstroCameraTrigger::setReady(bool ready)
{
    _ready = ready;
    if (!_ready) { stopObservation(); }
}

void AstroCameraTrigger::setTriggerCallback(TriggerCallback callback, void *context)
{
    _callback = callback;
    _context = context;
}

AstroData *AstroCameraTrigger::allocateData() const
{
    return _allocateDataForObjType((aid_t)AstroIdentity::ObservationDevice, (aid_t)classType);
}

void AstroCameraTrigger::saveToData(AstroData *dataOut)
{
    AstroObject::saveToData(dataOut);
    if (dataOut) { dataOut->id.object.classType = (aid_t)classType; }
}


AstroObservationDeviceData::AstroObservationDeviceData()
    : AstroObjectData()
{
    _size = sizeof(*this);
    id.object.idType = (aid_t)AstroIdentity::ObservationDevice;
    id.object.objType = 0;
    id.object.posIndex = aposi_none;
    id.object.classType = (aid_t)AstroObservationDevice::CameraTrigger;
}
