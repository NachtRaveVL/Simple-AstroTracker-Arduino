/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Camera
*/

#include "Astruino.h"

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


AstroCamera::AstroCamera(AstroObjInterface *parent, int mode)
    : AstroSubObject(parent), _mode((int8_t)mode), _intervalMillis(0), _exposureMillis(0),
      _shutterPulseMillis(0), _observing(false), _lastCapture(0), _shutter(parent ? parent : this)
{ ; }

AstroCamera::AstroCamera(AstroObjInterface *parent, const AstroCameraSubData *dataIn)
    : AstroSubObject(parent), _mode(dataIn ? dataIn->type : Unknown),
      _intervalMillis(dataIn ? dataIn->intervalMillis : 0), _exposureMillis(dataIn ? dataIn->exposureMillis : 0),
      _shutterPulseMillis(dataIn ? dataIn->shutterPulseMillis : 0), _observing(false), _lastCapture(0),
      _shutter(parent ? parent : this)
{
    if (dataIn) { _shutter.initObject(dataIn->shutterName); }
}

void AstroCamera::setMode(int mode)
{
    int8_t nextMode = mode == Interval || mode == Exposure ? (int8_t)mode : (int8_t)Unknown;
    if (_mode != nextMode) {
        stopObservation();
        _mode = nextMode;
        bumpRevisionIfNeeded();
    }
}

void AstroCamera::setInterval(millis_t intervalMillis)
{
    if (_intervalMillis != intervalMillis) {
        _intervalMillis = intervalMillis;
        _lastCapture = 0;
        bumpRevisionIfNeeded();
    }
}

void AstroCamera::setExposureTime(millis_t exposureMillis)
{
    if (_exposureMillis != exposureMillis) {
        _exposureMillis = exposureMillis;
        _lastCapture = 0;
        bumpRevisionIfNeeded();
    }
}

void AstroCamera::setShutterPulseTime(millis_t shutterPulseMillis)
{
    if (_shutterPulseMillis != shutterPulseMillis) {
        _shutterPulseMillis = shutterPulseMillis;
        _lastCapture = 0;
        bumpRevisionIfNeeded();
    }
}

bool AstroCamera::ready() const
{
    return _shutter.isSet() &&
           ((_mode == Interval && _intervalMillis && _shutterPulseMillis) ||
            (_mode == Exposure && _exposureMillis));
}

void AstroCamera::startObservation()
{
    if (!ready() || _observing) { return; }
    _observing = true;
    _lastCapture = 0;
}

void AstroCamera::stopObservation()
{
    if (!_observing && !_shutter.isActivated()) { return; }
    _observing = false;
    _lastCapture = 0;
    _shutter.disableActivation();
}

void AstroCamera::triggerShutter(millis_t duration)
{
    if (!duration || !_shutter.isSet()) { return; }
    _shutter.setupActivation(duration);
    _shutter.enableActivation();
}

void AstroCamera::update()
{
    _shutter.updateIfNeeded();
    if (!_observing || !ready() || _shutter.isActivated()) { return; }

    millis_t now = nzMillis();
    if (_mode == Interval) {
        if (!_lastCapture || (millis_t)(now - _lastCapture) >= _intervalMillis) {
            triggerShutter(_shutterPulseMillis);
            _lastCapture = now;
        }
    } else if (_mode == Exposure) {
        if (!_lastCapture) {
            triggerShutter(_exposureMillis);
            _lastCapture = now;
        } else {
            _observing = false;
        }
    }
}

void AstroCamera::unresolveAny(AstroObject *object)
{
    _shutter.unresolveIf(object);
    AstroSubObject::unresolveAny(object);
}

void AstroCamera::saveToData(AstroCameraSubData *dataOut) const
{
    if (!dataOut) { return; }
    dataOut->type = _mode;
    dataOut->intervalMillis = _intervalMillis;
    dataOut->exposureMillis = _exposureMillis;
    dataOut->shutterPulseMillis = _shutterPulseMillis;
    if (_shutter.isSet()) {
        strncpy(dataOut->shutterName, _shutter.getKeyString().c_str(), ASTRO_NAME_MAXSIZE - 1);
        dataOut->shutterName[ASTRO_NAME_MAXSIZE - 1] = '\0';
    }
}


AstroCameraSubData::AstroCameraSubData()
    : AstroSubData(AstroCamera::Unknown), shutterName{0}, intervalMillis(0), exposureMillis(0), shutterPulseMillis(0)
{ ; }

void AstroCameraSubData::toJSONObject(JsonObject &objectOut) const
{
    AstroSubData::toJSONObject(objectOut);
    if (shutterName[0]) { objectOut[SFP(AStr_Key_ShutterName)] = shutterName; }
    if (intervalMillis) { objectOut[SFP(AStr_Key_IntervalMillis)] = intervalMillis; }
    if (exposureMillis) { objectOut[SFP(AStr_Key_ExposureMillis)] = exposureMillis; }
    if (shutterPulseMillis) { objectOut[SFP(AStr_Key_ShutterPulseMillis)] = shutterPulseMillis; }
}

void AstroCameraSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroSubData::fromJSONObject(objectIn);
    const char *shutter = objectIn[SFP(AStr_Key_ShutterName)] | nullptr;
    if (shutter) { strncpy(shutterName, shutter, ASTRO_NAME_MAXSIZE - 1); shutterName[ASTRO_NAME_MAXSIZE - 1] = '\0'; }
    intervalMillis = objectIn[SFP(AStr_Key_IntervalMillis)] | intervalMillis;
    exposureMillis = objectIn[SFP(AStr_Key_ExposureMillis)] | exposureMillis;
    shutterPulseMillis = objectIn[SFP(AStr_Key_ShutterPulseMillis)] | shutterPulseMillis;
}
