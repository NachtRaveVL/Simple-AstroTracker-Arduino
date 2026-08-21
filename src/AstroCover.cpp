/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Cover
*/

#include "Astruino.h"

AstroCover::AstroCover(aposi_t positionIndex)
    : AstroObject(AstroIdentity(AstroIdentity::Cover, 0, positionIndex)), _position(0.0f), _target(0.0f),
      _travelRate(ASTRO_COVER_TRAVEL_RATE), _travelTimeout(ASTRO_COVER_TRAVEL_TIMEOUT_SECS), _travelElapsed(0.0),
      _openLimitActive(false), _closedLimitActive(false), _faulted(false), _actuator(this), _openSensor(this), _closedSensor(this), _lastUpdate(0)
{ ; }

AstroCover::AstroCover(const AstroObjectData *dataIn)
    : AstroObject(dataIn), _position(0.0f), _target(0.0f),
      _travelRate(ASTRO_COVER_TRAVEL_RATE), _travelTimeout(ASTRO_COVER_TRAVEL_TIMEOUT_SECS), _travelElapsed(0.0),
      _openLimitActive(false), _closedLimitActive(false), _faulted(false), _actuator(this), _openSensor(this), _closedSensor(this), _lastUpdate(0)
{ ; }

void AstroCover::open()
{
    if (!isFPEqual(_target, 1.0f)) { _travelElapsed = 0.0; }
    _target = 1.0f;
}

void AstroCover::close()
{
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
}

void AstroCover::clearFault()
{
    _faulted = false;
    _travelElapsed = 0.0;
}

bool AstroCover::isMoving() const
{
    return !isFPEqual(_position, _target) || _actuator.isActivated();
}

bool AstroCover::isOpen() const
{
    return _openSensor.isSet() ? _openLimitActive : _position >= 0.999f;
}

bool AstroCover::isClosed() const
{
    return _closedSensor.isSet() ? _closedLimitActive : _position <= 0.001f;
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
    _actuator.setupActivation(power);
    if (fabsf(power) > FLT_EPSILON) { _actuator.enableActivation(); }
    else { _actuator.disableActivation(); }
}

void AstroCover::update()
{
    const millis_t now = nzMillis();
    const double elapsedSeconds = _lastUpdate ? (double)(now - _lastUpdate) / 1000.0 : 0.0;
    _lastUpdate = now;
    if (_openSensor.isSet()) { pollLimitSensor(_openSensor, &_openLimitActive); }
    if (_closedSensor.isSet()) { pollLimitSensor(_closedSensor, &_closedLimitActive); }

    if (_openLimitActive && _closedLimitActive) {
        _faulted = true;
        applyActuatorPower(0.0f);
        return;
    }

    if (_openLimitActive) { _position = 1.0f; }
    if (_closedLimitActive) { _position = 0.0f; }

    bool opening = _target > 0.5f;
    bool targetReached = opening ? isOpen() : isClosed();
    if (_faulted || targetReached) {
        _travelElapsed = 0.0;
        applyActuatorPower(0.0f);
        return;
    }

    _travelElapsed += elapsedSeconds > 0.0 ? elapsedSeconds : 0.0;
    if (_travelTimeout > 0.0 && _travelElapsed >= _travelTimeout) {
        _faulted = true;
        applyActuatorPower(0.0f);
        return;
    }

    applyActuatorPower(opening ? 1.0f : -1.0f);

    float step = _travelRate * (float)(elapsedSeconds > 0.0 ? elapsedSeconds : 0.0);
    if (opening) { _position = constrain(_position + step, 0.0f, 1.0f); }
    else { _position = constrain(_position - step, 0.0f, 1.0f); }

    if (!_openSensor.isSet() && !_closedSensor.isSet()) {
        targetReached = opening ? isOpen() : isClosed();
        if (targetReached) {
            _travelElapsed = 0.0;
            applyActuatorPower(0.0f);
        }
    }
}

void AstroCover::unresolveAny(AstroObject *object)
{
    _actuator.unresolveAny(object);
    _openSensor.unresolveAny(object);
    _closedSensor.unresolveAny(object);
    AstroObject::unresolveAny(object);
}
