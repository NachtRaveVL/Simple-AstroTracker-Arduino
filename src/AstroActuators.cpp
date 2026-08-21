/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Actuators
*/

#include "Astruino.h"

AstroActuator::AstroActuator(Astro_ActuatorType actuatorType, aposi_t positionIndex)
    : AstroObject(AstroIdentity(actuatorType, positionIndex)), _actuatorType(actuatorType),
      _enableMode(Astro_EnableMode_Highest), _power(0.0f), _needsUpdate(false), _handles{nullptr}
{ ; }

AstroActuator::AstroActuator(const AstroObjectData *dataIn)
    : AstroObject(dataIn), _actuatorType(dataIn ? (Astro_ActuatorType)dataIn->objType : Astro_ActuatorType_Undefined),
      _enableMode(Astro_EnableMode_Highest), _power(0.0f), _needsUpdate(false), _handles{nullptr}
{ ; }

void AstroActuator::setPower(float power)
{
    _power = constrain(power, -1.0f, 1.0f);
}

bool AstroActuator::addActivationHandle(AstroActivationHandle *handle)
{
    if (!handle) { return false; }
    for (size_t i = 0; i < ASTRO_ACTIVATION_HANDLE_SLOTS; ++i) {
        if (_handles[i] == handle) { return true; }
        if (!_handles[i]) {
            _handles[i] = handle;
            setNeedsUpdate();
            return true;
        }
    }
    return false;
}

bool AstroActuator::removeActivationHandle(AstroActivationHandle *handle)
{
    for (size_t i = 0; i < ASTRO_ACTIVATION_HANDLE_SLOTS; ++i) {
        if (_handles[i] == handle) {
            for (size_t j = i; j + 1 < ASTRO_ACTIVATION_HANDLE_SLOTS; ++j) { _handles[j] = _handles[j + 1]; }
            _handles[ASTRO_ACTIVATION_HANDLE_SLOTS - 1] = nullptr;
            setNeedsUpdate();
            return true;
        }
    }
    return false;
}

void AstroActuator::resolveActivations()
{
    millis_t time = nzMillis();
    float drivingIntensity = 0.0f;
    int handleCount = 0;

    switch (_enableMode) {
        case Astro_EnableMode_Highest: {
            drivingIntensity = -__FLT_MAX__;
            for (size_t index = 0; index < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[index]; ++index) {
                if (_handles[index]->isValid() && !_handles[index]->isDone()) {
                    float handleIntensity = _handles[index]->getDriveIntensity();
                    if (handleIntensity > drivingIntensity) { drivingIntensity = handleIntensity; }
                    ++handleCount;
                }
            }
        } break;

        case Astro_EnableMode_Lowest: {
            drivingIntensity = __FLT_MAX__;
            for (size_t index = 0; index < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[index]; ++index) {
                if (_handles[index]->isValid() && !_handles[index]->isDone()) {
                    float handleIntensity = _handles[index]->getDriveIntensity();
                    if (handleIntensity < drivingIntensity) { drivingIntensity = handleIntensity; }
                    ++handleCount;
                }
            }
        } break;

        case Astro_EnableMode_Average: {
            for (size_t index = 0; index < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[index]; ++index) {
                if (_handles[index]->isValid() && !_handles[index]->isDone()) {
                    drivingIntensity += _handles[index]->getDriveIntensity();
                    ++handleCount;
                }
            }
            if (handleCount) { drivingIntensity /= handleCount; }
        } break;

        case Astro_EnableMode_Multiply: {
            bool started = false;
            for (size_t index = 0; index < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[index]; ++index) {
                if (_handles[index]->isValid() && !_handles[index]->isDone()) {
                    if (!started) {
                        drivingIntensity = _handles[index]->getDriveIntensity();
                        started = true;
                    } else {
                        drivingIntensity *= _handles[index]->getDriveIntensity();
                    }
                    ++handleCount;
                }
            }
        } break;

        case Astro_EnableMode_InOrder: {
            for (size_t index = 0; index < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[index]; ++index) {
                if (_handles[index]->isValid() && !_handles[index]->isDone()) {
                    drivingIntensity = _handles[index]->getDriveIntensity();
                    handleCount = 1;
                    break;
                }
            }
        } break;

        case Astro_EnableMode_RevOrder: {
            size_t count = 0;
            while (count < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[count]) { ++count; }
            while (count) {
                AstroActivationHandle *handle = _handles[--count];
                if (handle->isValid() && !handle->isDone()) {
                    drivingIntensity = handle->getDriveIntensity();
                    handleCount = 1;
                    break;
                }
            }
        } break;

        default:
            break;
    }

    switch (_enableMode) {
        case Astro_EnableMode_InOrder: {
            bool selected = false;
            for (size_t index = 0; index < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[index]; ++index) {
                AstroActivationHandle *handle = _handles[index];
                if (!selected && handle->isValid() && !handle->isDone() &&
                    isFPEqual(handle->getDriveIntensity(), drivingIntensity)) {
                    selected = true;
                    if (!handle->checkTime) { handle->checkTime = time; }
                } else if (handle->checkTime) {
                    handle->checkTime = 0;
                }
            }
        } break;

        case Astro_EnableMode_RevOrder: {
            size_t count = 0;
            while (count < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[count]) { ++count; }
            bool selected = false;
            while (count) {
                AstroActivationHandle *handle = _handles[--count];
                if (!selected && handle->isValid() && !handle->isDone() &&
                    isFPEqual(handle->getDriveIntensity(), drivingIntensity)) {
                    selected = true;
                    if (!handle->checkTime) { handle->checkTime = time; }
                } else if (handle->checkTime) {
                    handle->checkTime = 0;
                }
            }
        } break;

        default: {
            for (size_t index = 0; index < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[index]; ++index) {
                AstroActivationHandle *handle = _handles[index];
                if (handle->isValid() && !handle->isDone() && !handle->checkTime) {
                    handle->checkTime = time;
                }
            }
        } break;
    }

    setPower(handleCount ? drivingIntensity : 0.0f);
    _needsUpdate = false;
}

void AstroActuator::update()
{
    millis_t time = nzMillis();

    for (size_t index = 0; index < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[index];) {
        AstroActivationHandle *handle = _handles[index];
        if (handle->isActive()) { handle->elapseTo(time); }
        if (handle->actuator.get() != this || !handle->isValid() || handle->isDone()) {
            if (handle->actuator.get() == this) { handle->actuator = nullptr; }
            removeActivationHandle(handle);
            continue;
        }
        ++index;
    }

    resolveActivations();
}

void AstroCallbackActuator::setPower(float power)
{
    AstroActuator::setPower(power);
    if (_callback) { _callback(_context, _power); }
}

AstroDigitalActuator::AstroDigitalActuator(AstroDigitalPin outputPin, Astro_ActuatorType actuatorType, aposi_t positionIndex)
    : AstroActuator(actuatorType, positionIndex), _outputPin(outputPin)
{
    _outputPin.init();
}

void AstroDigitalActuator::setPower(float power)
{
    AstroActuator::setPower(power);
    if (_outputPin.isValid()) {
        if (!isFPEqual(_power, 0.0f)) { _outputPin.activate(); }
        else { _outputPin.deactivate(); }
    }
}

AstroRelayMotorActuator::AstroRelayMotorActuator(AstroDigitalPin forwardPin, AstroDigitalPin reversePin,
                                                 Astro_ActuatorType actuatorType, aposi_t positionIndex)
    : AstroActuator(actuatorType, positionIndex), _forwardPin(forwardPin), _reversePin(reversePin)
{
    _forwardPin.init();
    _reversePin.init();
    _forwardPin.deactivate();
    _reversePin.deactivate();
}

void AstroRelayMotorActuator::setPower(float power)
{
    AstroActuator::setPower(power);
    if (_power > 0.0f && !isFPEqual(_power, 0.0f)) {
        _reversePin.deactivate();
        _forwardPin.activate();
    } else if (_power < 0.0f && !isFPEqual(_power, 0.0f)) {
        _forwardPin.deactivate();
        _reversePin.activate();
    } else {
        _forwardPin.deactivate();
        _reversePin.deactivate();
    }
}

AstroAnalogActuator::AstroAnalogActuator(AstroAnalogPin outputPin, Astro_ActuatorType actuatorType, aposi_t positionIndex)
    : AstroActuator(actuatorType, positionIndex), _outputPin(outputPin)
{
    _outputPin.init();
}

void AstroAnalogActuator::setPower(float power)
{
    AstroActuator::setPower(power);
    if (_outputPin.isValid()) { _outputPin.analogWrite(_power < 0.0f ? -_power : _power); }
}

AstroFocuser::AstroFocuser(int32_t maximumPosition, aposi_t positionIndex)
    : AstroActuator(Astro_ActuatorType_Focuser, positionIndex), _position(0), _targetPosition(0),
      _minimumPosition(0), _maximumPosition(maximumPosition > 0 ? maximumPosition : 0), _moving(false),
      _moveCallback(nullptr), _stopCallback(nullptr), _positionCallback(nullptr), _context(nullptr)
{ ; }

AstroFocuser::AstroFocuser(const AstroObjectData *dataIn, int32_t maximumPosition)
    : AstroActuator(dataIn), _position(0), _targetPosition(0),
      _minimumPosition(0), _maximumPosition(maximumPosition > 0 ? maximumPosition : 0), _moving(false),
      _moveCallback(nullptr), _stopCallback(nullptr), _positionCallback(nullptr), _context(nullptr)
{ ; }

void AstroFocuser::setMoveCallback(MoveCallback callback, void *context)
{
    _moveCallback = callback;
    _context = context;
}

void AstroFocuser::setStopCallback(StopCallback callback)
{
    _stopCallback = callback;
}

void AstroFocuser::setPositionCallback(PositionCallback callback)
{
    _positionCallback = callback;
}

void AstroFocuser::setPosition(int32_t position)
{
    _position = position < _minimumPosition ? _minimumPosition : position > _maximumPosition ? _maximumPosition : position;
    if (_position == _targetPosition) { _moving = false; }
}

void AstroFocuser::setLimits(int32_t minimumPosition, int32_t maximumPosition)
{
    if (maximumPosition < minimumPosition) { return; }
    _minimumPosition = minimumPosition;
    _maximumPosition = maximumPosition;
    setPosition(_position);
    _targetPosition = _targetPosition < _minimumPosition ? _minimumPosition :
                      _targetPosition > _maximumPosition ? _maximumPosition : _targetPosition;
}

void AstroFocuser::moveTo(int32_t position)
{
    _targetPosition = position < _minimumPosition ? _minimumPosition : position > _maximumPosition ? _maximumPosition : position;
    _moving = _targetPosition != _position;
    if (_moveCallback) { _moveCallback(_context, _targetPosition); }
}

void AstroFocuser::moveBy(int32_t steps)
{
    int64_t target = (int64_t)_targetPosition + steps;
    if (target < _minimumPosition) { target = _minimumPosition; }
    if (target > _maximumPosition) { target = _maximumPosition; }
    moveTo((int32_t)target);
}

void AstroFocuser::halt()
{
    _targetPosition = _position;
    _moving = false;
    setPower(0.0f);
    if (_stopCallback) { _stopCallback(_context); }
}

void AstroFocuser::update()
{
    if (_positionCallback) {
        int32_t position = _position;
        if (_positionCallback(_context, &position)) { setPosition(position); }
    }
}
