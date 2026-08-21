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
    _power = astroConstrain(power, -1.0f, 1.0f);
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

void AstroActuator::resolveActivations(millis_t time)
{
    float resolved = 0.0f;
    int count = 0;

    switch (_enableMode) {
        case Astro_EnableMode_Highest:
            resolved = -__FLT_MAX__;
            for (size_t i = 0; i < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[i]; ++i) {
                if (_handles[i]->isValid() && !_handles[i]->isDone()) {
                    float drive = _handles[i]->getDriveIntensity();
                    if (drive > resolved) { resolved = drive; }
                    ++count;
                }
            }
            break;

        case Astro_EnableMode_Lowest:
            resolved = __FLT_MAX__;
            for (size_t i = 0; i < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[i]; ++i) {
                if (_handles[i]->isValid() && !_handles[i]->isDone()) {
                    float drive = _handles[i]->getDriveIntensity();
                    if (drive < resolved) { resolved = drive; }
                    ++count;
                }
            }
            break;

        case Astro_EnableMode_Average:
            for (size_t i = 0; i < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[i]; ++i) {
                if (_handles[i]->isValid() && !_handles[i]->isDone()) {
                    resolved += _handles[i]->getDriveIntensity();
                    ++count;
                }
            }
            if (count) { resolved /= count; }
            break;

        case Astro_EnableMode_Multiply:
            for (size_t i = 0; i < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[i]; ++i) {
                if (_handles[i]->isValid() && !_handles[i]->isDone()) {
                    resolved = count ? resolved * _handles[i]->getDriveIntensity()
                                     : _handles[i]->getDriveIntensity();
                    ++count;
                }
            }
            break;

        case Astro_EnableMode_InOrder:
            for (size_t i = 0; i < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[i]; ++i) {
                if (_handles[i]->isValid() && !_handles[i]->isDone()) {
                    resolved = _handles[i]->getDriveIntensity();
                    ++count;
                    break;
                }
            }
            break;

        case Astro_EnableMode_RevOrder:
            for (int i = ASTRO_ACTIVATION_HANDLE_SLOTS - 1; i >= 0; --i) {
                if (_handles[i] && _handles[i]->isValid() && !_handles[i]->isDone()) {
                    resolved = _handles[i]->getDriveIntensity();
                    ++count;
                    break;
                }
            }
            break;

        default:
            break;
    }

    if (!count) { resolved = 0.0f; }

    switch (_enableMode) {
        case Astro_EnableMode_InOrder: {
            bool selected = false;
            for (size_t i = 0; i < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[i]; ++i) {
                AstroActivationHandle *handle = _handles[i];
                if (!selected && handle->isValid() && !handle->isDone()) {
                    selected = true;
                    if (!handle->checkTime) { handle->checkTime = time; }
                } else if (handle->checkTime) {
                    handle->checkTime = 0;
                }
            }
        } break;

        case Astro_EnableMode_RevOrder: {
            bool selected = false;
            for (int i = ASTRO_ACTIVATION_HANDLE_SLOTS - 1; i >= 0; --i) {
                AstroActivationHandle *handle = _handles[i];
                if (!handle) { continue; }
                if (!selected && handle->isValid() && !handle->isDone()) {
                    selected = true;
                    if (!handle->checkTime) { handle->checkTime = time; }
                } else if (handle->checkTime) {
                    handle->checkTime = 0;
                }
            }
        } break;

        default:
            for (size_t i = 0; i < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[i]; ++i) {
                if (_handles[i]->isValid() && !_handles[i]->isDone() && !_handles[i]->checkTime) {
                    _handles[i]->checkTime = time;
                }
            }
            break;
    }

    setPower(resolved);
}

void AstroActuator::update()
{
    AstroObject::update();

    millis_t time = astroNZMillis();
    bool wasActive = !isFPEqual(_power, 0.0f);

    for (size_t i = 0; i < ASTRO_ACTIVATION_HANDLE_SLOTS && _handles[i]; ) {
        AstroActivationHandle *handle = _handles[i];
        if (wasActive && handle->isActive()) { handle->elapseTo(time); }

        if (handle->actuator.get() != this || !handle->isValid() || handle->isDone()) {
            if (handle->actuator.get() == this) { handle->actuator = nullptr; }
            removeActivationHandle(handle);
            continue;
        }
        ++i;
    }

    bool canEnable = _handles[0] != nullptr;
    if (!canEnable && (wasActive || _needsUpdate)) {
        setPower(0.0f);
    } else if (canEnable && (!wasActive || _needsUpdate)) {
        resolveActivations(time);
    }
    _needsUpdate = false;
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
