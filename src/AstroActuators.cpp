/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Actuators
*/

#include "Astruino.h"

AstroActuator *newActuatorObjectFromData(const AstroActuatorData *dataIn)
{
    if (dataIn && !isValidType(dataIn->id.object.idType)) return nullptr;
    ASTRO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(AStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData() && dataIn->id.object.idType == (aid_t)AstroIdentity::Actuator) {
        switch (dataIn->id.object.classType) {
            case (aid_t)AstroActuator::Base:
                return new AstroActuator(dataIn);
            case (aid_t)AstroActuator::Callback:
                return new AstroCallbackActuator(dataIn);
            case (aid_t)AstroActuator::Digital:
                return new AstroDigitalActuator(dataIn);
            case (aid_t)AstroActuator::RelayMotor:
                return new AstroRelayMotorActuator(dataIn);
            case (aid_t)AstroActuator::Analog:
                return new AstroAnalogActuator(dataIn);
            case (aid_t)AstroActuator::Focuser:
                return new AstroFocuser(dataIn);
            default: break;
        }
    }

    return nullptr;
}

AstroActuator::AstroActuator(Astro_ActuatorType actuatorType, aposi_t positionIndex, int classTypeIn)
    : AstroObject(AstroIdentity(actuatorType, positionIndex)), classType((typeof(classType))classTypeIn),
      _actuatorType(actuatorType), _enableMode(Astro_EnableMode_Highest), _power(0.0f), _needsUpdate(false), _handles{nullptr}
{ ; }

AstroActuator::AstroActuator(const AstroActuatorData *dataIn)
    : AstroObject(dataIn), classType(dataIn ? (typeof(classType))dataIn->id.object.classType : Unknown),
      _actuatorType(dataIn ? (Astro_ActuatorType)dataIn->id.object.objType : Astro_ActuatorType_Undefined),
      _enableMode(dataIn ? dataIn->enableMode : Astro_EnableMode_Highest), _power(0.0f), _needsUpdate(false), _handles{nullptr}
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

AstroData *AstroActuator::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void AstroActuator::saveToData(AstroData *dataOut)
{
    AstroObject::saveToData(dataOut);
    if (!dataOut) { return; }
    AstroActuatorData *actuatorData = static_cast<AstroActuatorData *>(dataOut);
    actuatorData->id.object.classType = (aid_t)classType;
    actuatorData->enableMode = _enableMode;
}

void AstroCallbackActuator::setPower(float power)
{
    AstroActuator::setPower(power);
    if (_callback) { _callback(_context, _power); }
}

AstroDigitalActuator::AstroDigitalActuator(AstroDigitalPin outputPin, Astro_ActuatorType actuatorType, aposi_t positionIndex)
    : AstroActuator(actuatorType, positionIndex, Digital), _outputPin(outputPin)
{
    _outputPin.init();
}

AstroDigitalActuator::AstroDigitalActuator(const AstroActuatorData *dataIn)
    : AstroActuator(dataIn), _outputPin(dataIn ? &dataIn->outputPin : nullptr)
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

void AstroDigitalActuator::saveToData(AstroData *dataOut)
{
    AstroActuator::saveToData(dataOut);
    if (dataOut) { _outputPin.saveToData(&static_cast<AstroActuatorData *>(dataOut)->outputPin); }
}

AstroRelayMotorActuator::AstroRelayMotorActuator(AstroDigitalPin forwardPin, AstroDigitalPin reversePin,
                                                 Astro_ActuatorType actuatorType, aposi_t positionIndex)
    : AstroActuator(actuatorType, positionIndex, RelayMotor), _forwardPin(forwardPin), _reversePin(reversePin)
{
    _forwardPin.init();
    _reversePin.init();
    _forwardPin.deactivate();
    _reversePin.deactivate();
}

AstroRelayMotorActuator::AstroRelayMotorActuator(const AstroActuatorData *dataIn)
    : AstroActuator(dataIn), _forwardPin(dataIn ? &dataIn->outputPin : nullptr),
      _reversePin(dataIn ? &dataIn->outputPin2 : nullptr)
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

void AstroRelayMotorActuator::saveToData(AstroData *dataOut)
{
    AstroActuator::saveToData(dataOut);
    if (dataOut) {
        AstroActuatorData *actuatorData = static_cast<AstroActuatorData *>(dataOut);
        _forwardPin.saveToData(&actuatorData->outputPin);
        _reversePin.saveToData(&actuatorData->outputPin2);
    }
}

AstroAnalogActuator::AstroAnalogActuator(AstroAnalogPin outputPin, Astro_ActuatorType actuatorType, aposi_t positionIndex)
    : AstroActuator(actuatorType, positionIndex, Analog), _outputPin(outputPin)
{
    _outputPin.init();
}

AstroAnalogActuator::AstroAnalogActuator(const AstroActuatorData *dataIn)
    : AstroActuator(dataIn), _outputPin(dataIn ? &dataIn->outputPin : nullptr)
{
    _outputPin.init();
}

void AstroAnalogActuator::setPower(float power)
{
    AstroActuator::setPower(power);
    if (_outputPin.isValid()) { _outputPin.analogWrite(_power < 0.0f ? -_power : _power); }
}

void AstroAnalogActuator::saveToData(AstroData *dataOut)
{
    AstroActuator::saveToData(dataOut);
    if (dataOut) { _outputPin.saveToData(&static_cast<AstroActuatorData *>(dataOut)->outputPin); }
}

AstroFocuser::AstroFocuser(int32_t maximumPosition, aposi_t positionIndex)
    : AstroActuator(Astro_ActuatorType_Focuser, positionIndex, Focuser), _position(0), _targetPosition(0),
      _minimumPosition(0), _maximumPosition(maximumPosition > 0 ? maximumPosition : 0), _moving(false),
      _moveCallback(nullptr), _stopCallback(nullptr), _positionCallback(nullptr), _context(nullptr)
{ ; }

AstroFocuser::AstroFocuser(const AstroActuatorData *dataIn, int32_t maximumPosition)
    : AstroActuator(dataIn), _position(0), _targetPosition(0),
      _minimumPosition(dataIn ? dataIn->minimumPosition : 0),
      _maximumPosition(dataIn ? dataIn->maximumPosition : (maximumPosition > 0 ? maximumPosition : 0)), _moving(false),
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

void AstroFocuser::saveToData(AstroData *dataOut)
{
    AstroActuator::saveToData(dataOut);
    if (dataOut) {
        AstroActuatorData *actuatorData = static_cast<AstroActuatorData *>(dataOut);
        actuatorData->minimumPosition = _minimumPosition;
        actuatorData->maximumPosition = _maximumPosition;
    }
}

AstroActuatorData::AstroActuatorData()
    : AstroObjectData(), enableMode(Astro_EnableMode_Highest), outputPin(), outputPin2(),
      minimumPosition(0), maximumPosition(10000)
{
    _size = sizeof(*this);
    id.object.idType = (aid_t)AstroIdentity::Actuator;
    id.object.objType = (aid_t)Astro_ActuatorType_Undefined;
    id.object.posIndex = aposi_none;
    id.object.classType = (aid_t)AstroActuator::Unknown;
}

void AstroActuatorData::toJSONObject(JsonObject &objectOut) const
{
    AstroObjectData::toJSONObject(objectOut);
    objectOut["enableMode"] = (int)enableMode;
    if (outputPin.isSet()) { JsonObject pinObj = objectOut.createNestedObject("outputPin"); outputPin.toJSONObject(pinObj); }
    if (outputPin2.isSet()) { JsonObject pinObj = objectOut.createNestedObject("outputPin2"); outputPin2.toJSONObject(pinObj); }
    if (id.object.classType == (aid_t)AstroActuator::Focuser) {
        objectOut["minimumPosition"] = minimumPosition;
        objectOut["maximumPosition"] = maximumPosition;
    }
}

void AstroActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroObjectData::fromJSONObject(objectIn);
    enableMode = (Astro_EnableMode)(objectIn["enableMode"] | (int)enableMode);
    JsonObjectConst pinObj = objectIn["outputPin"].as<JsonObjectConst>();
    if (!pinObj.isNull()) { outputPin.fromJSONObject(pinObj); }
    JsonObjectConst pin2Obj = objectIn["outputPin2"].as<JsonObjectConst>();
    if (!pin2Obj.isNull()) { outputPin2.fromJSONObject(pin2Obj); }
    minimumPosition = objectIn["minimumPosition"] | minimumPosition;
    maximumPosition = objectIn["maximumPosition"] | maximumPosition;
}
