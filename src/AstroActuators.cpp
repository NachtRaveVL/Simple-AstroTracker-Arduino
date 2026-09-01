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
      _enabled(false), _actuatorType(actuatorType), _enableMode(Astro_EnableMode_Highest), _power(0.0f), _needsUpdate(false),
      _contPowerUsage(), _parentRail(this)
{ ; }

AstroActuator::AstroActuator(const AstroActuatorData *dataIn)
    : AstroObject(dataIn), classType(dataIn ? (typeof(classType))dataIn->id.object.classType : Unknown),
      _enabled(false), _actuatorType(dataIn ? (Astro_ActuatorType)dataIn->id.object.objType : Astro_ActuatorType_Undefined),
      _enableMode(dataIn ? dataIn->enableMode : Astro_EnableMode_Highest), _power(0.0f), _needsUpdate(false),
      _contPowerUsage(), _parentRail(this)
{
    if (dataIn) {
        if (dataIn->contPowerUsage.units != Astro_UnitsType_Undefined) { _contPowerUsage = AstroSingleMeasurement(&dataIn->contPowerUsage); }
        _parentRail.initObject(dataIn->railName);
    }
}

void AstroActuator::_enableActuator(float intensity)
{
    bool wasEnabled = _enabled;
    _power = constrain(intensity, -1.0f, 1.0f);
    _enabled = fabsf(_power) > FLT_EPSILON;
    if (wasEnabled != _enabled) { handleActivation(); }
}

void AstroActuator::_disableActuator()
{
    bool wasEnabled = _enabled;
    _enabled = false;
    _power = 0.0f;
    if (wasEnabled) { handleActivation(); }
}

bool AstroActuator::getCanEnable()
{
    return !getParentRail() || getParentRail()->canActivate(this);
}

void AstroActuator::setContinuousPowerUsage(AstroSingleMeasurement contPowerUsage)
{
    _contPowerUsage = contPowerUsage;
    _contPowerUsage.setMinFrame(1);
    bumpRevisionIfNeeded();
}

const AstroSingleMeasurement &AstroActuator::getContinuousPowerUsage()
{
    return _contPowerUsage;
}

AstroAttachment &AstroActuator::getParentRailAttachment()
{
    return _parentRail;
}

Signal<AstroActuator *, ASTRO_ACTUATOR_SIGNAL_SLOTS> &AstroActuator::getActivationSignal()
{
    return _activateSignal;
}

void AstroActuator::handleActivation()
{
    if (_enabled) {
        getLogger()->logActivation(this);
    } else {
        for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
            if ((*handleIter)->checkTime) { (*handleIter)->checkTime = 0; }
        }

        getLogger()->logDeactivation(this);
    }

    #ifdef ASTRO_USE_MULTITASKING
        scheduleSignalFireOnce<AstroActuator *>(getSharedPtr(), _activateSignal, this);
    #else
        _activateSignal.fire(this);
    #endif
}

void AstroActuator::update()
{
    AstroObject::update();

    _parentRail.resolve();

    millis_t time = nzMillis();

    // Update running handles and elapse them as needed, determine forced status, and remove invalid/finished handles
    bool forced = false;
    if (_handles.size()) {
        for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
            if (_enabled && (*handleIter)->isActive()) {
                (*handleIter)->elapseTo(time);
            }
            if ((*handleIter)->actuator.get() != this || !(*handleIter)->isValid() || (*handleIter)->isDone()) {
                if ((*handleIter)->actuator.get() == this) { (*handleIter)->actuator = nullptr; }
                handleIter = _handles.erase(handleIter) - 1;
                setNeedsUpdate();
                continue;
            }
            forced = forced || (*handleIter)->isForced();
        }
    }

    // Enablement checking
    bool canEnable = _handles.size() && (forced || getCanEnable());

    if (!canEnable && (_enabled || _needsUpdate)) { // If enabled and shouldn't be (unless force enabled)
        _disableActuator();
    } else if (canEnable && (!_enabled || _needsUpdate)) { // If can enable and isn't (maybe force enabled)
        float drivingIntensity = 0.0f;

        // Determine what driving intensity [-1,1] actuator should use
        switch (_enableMode) {
            case Astro_EnableMode_Highest:
            case Astro_EnableMode_DescOrder: {
                drivingIntensity = -__FLT_MAX__;
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        auto handleIntensity = (*handleIter)->getDriveIntensity();
                        if (handleIntensity > drivingIntensity) { drivingIntensity = handleIntensity; }
                    }
                }
            } break;

            case Astro_EnableMode_Lowest:
            case Astro_EnableMode_AscOrder: {
                drivingIntensity = __FLT_MAX__;
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        auto handleIntensity = (*handleIter)->getDriveIntensity();
                        if (handleIntensity < drivingIntensity) { drivingIntensity = handleIntensity; }
                    }
                }
            } break;

            case Astro_EnableMode_Average: {
                int handleCount = 0;
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        drivingIntensity += (*handleIter)->getDriveIntensity();
                        ++handleCount;
                    }
                }
                if (handleCount) { drivingIntensity /= handleCount; }
            } break;

            case Astro_EnableMode_Multiply: {
                drivingIntensity = (*_handles.begin())->getDriveIntensity();
                for (auto handleIter = _handles.begin() + 1; handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        drivingIntensity *= (*handleIter)->getDriveIntensity();
                    }
                }
            } break;

            case Astro_EnableMode_InOrder: {
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        drivingIntensity += (*handleIter)->getDriveIntensity();
                        break;
                    }
                }
            } break;

            case Astro_EnableMode_RevOrder: {
                for (auto handleIter = _handles.end() - 1; handleIter != _handles.begin() - 1; --handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone()) {
                        drivingIntensity += (*handleIter)->getDriveIntensity();
                        break;
                    }
                }
            } break;

            default:
                break;
        }

        // Enable/disable activation handles as needed (serial modes only select 1 at a time)
        switch (_enableMode) {
            case Astro_EnableMode_InOrder:
            case Astro_EnableMode_DescOrder: {
                bool selected = false;
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if (!selected && (*handleIter)->isValid() && !(*handleIter)->isDone() && isFPEqual((*handleIter)->getDriveIntensity(), drivingIntensity)) {
                        selected = true; (*handleIter)->checkTime = time;
                    } else if ((*handleIter)->checkTime != 0) {
                        (*handleIter)->checkTime = 0;
                    }
                }
            } break;

            case Astro_EnableMode_RevOrder:
            case Astro_EnableMode_AscOrder: {
                bool selected = false;
                for (auto handleIter = _handles.end() - 1; handleIter != _handles.begin() - 1; --handleIter) {
                    if (!selected && (*handleIter)->isValid() && !(*handleIter)->isDone() && isFPEqual((*handleIter)->getDriveIntensity(), drivingIntensity)) {
                        selected = true; (*handleIter)->checkTime = time;
                    } else if ((*handleIter)->checkTime != 0) {
                        (*handleIter)->checkTime = 0;
                    }
                }
            } break;

            default: {
                for (auto handleIter = _handles.begin(); handleIter != _handles.end(); ++handleIter) {
                    if ((*handleIter)->isValid() && !(*handleIter)->isDone() && (*handleIter)->checkTime == 0) {
                        (*handleIter)->checkTime = time;
                    }
                }
            } break;
        }

        _enableActuator(drivingIntensity);
    }
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
    if (_contPowerUsage.isSet()) { _contPowerUsage.saveToData(&actuatorData->contPowerUsage); }
    if (_parentRail.isSet()) {
        strncpy(actuatorData->railName, _parentRail.getKeyString().c_str(), ASTRO_NAME_MAXSIZE - 1);
        actuatorData->railName[ASTRO_NAME_MAXSIZE - 1] = '\0';
    }
}

void AstroCallbackActuator::_enableActuator(float intensity)
{
    bool wasEnabled = _enabled;
    _power = constrain(intensity, -1.0f, 1.0f);
    _enabled = fabsf(_power) > FLT_EPSILON;
    if (_callback) { _callback(_context, _power); }
    if (wasEnabled != _enabled) { handleActivation(); }
}

void AstroCallbackActuator::_disableActuator()
{
    bool wasEnabled = _enabled;
    _enabled = false;
    _power = 0.0f;
    if (_callback) { _callback(_context, 0.0f); }
    if (wasEnabled) { handleActivation(); }
}

AstroDigitalActuator::AstroDigitalActuator(AstroDigitalPin outputPin, Astro_ActuatorType actuatorType, aposi_t positionIndex)
    : AstroActuator(actuatorType, positionIndex, Digital), _outputPin(outputPin)
{
    _outputPin.init();
    _outputPin.deactivate();
}

AstroDigitalActuator::AstroDigitalActuator(const AstroActuatorData *dataIn)
    : AstroActuator(dataIn), _outputPin(dataIn ? &dataIn->outputPin : nullptr)
{
    _outputPin.init();
    _outputPin.deactivate();
}

AstroDigitalActuator::~AstroDigitalActuator()
{
    if (_enabled) {
        _enabled = false;
        _power = 0.0f;
        _outputPin.deactivate();
    }
}

bool AstroDigitalActuator::getCanEnable()
{
    return _outputPin.isValid() && AstroActuator::getCanEnable();
}

void AstroDigitalActuator::_enableActuator(float intensity)
{
    bool wasEnabled = _enabled;
    if (_outputPin.isValid() && intensity > FLT_EPSILON) {
        _enabled = true;
        _power = 1.0f;
        _outputPin.activate();
    } else {
        _enabled = false;
        _power = 0.0f;
        if (_outputPin.isValid()) { _outputPin.deactivate(); }
    }
    if (wasEnabled != _enabled) { handleActivation(); }
}

void AstroDigitalActuator::_disableActuator()
{
    bool wasEnabled = _enabled;
    _enabled = false;
    _power = 0.0f;
    if (_outputPin.isValid()) { _outputPin.deactivate(); }
    if (wasEnabled) { handleActivation(); }
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

AstroRelayMotorActuator::~AstroRelayMotorActuator()
{
    if (_enabled) {
        _enabled = false;
        _power = 0.0f;
        _forwardPin.deactivate();
        _reversePin.deactivate();
    }
}

bool AstroRelayMotorActuator::getCanEnable()
{
    return _forwardPin.isValid() && _reversePin.isValid() && AstroActuator::getCanEnable();
}

void AstroRelayMotorActuator::_enableActuator(float intensity)
{
    bool wasEnabled = _enabled;
    _power = constrain(intensity, -1.0f, 1.0f);
    if (_power > FLT_EPSILON) {
        _enabled = true;
        _reversePin.deactivate();
        _forwardPin.activate();
    } else if (_power < -FLT_EPSILON) {
        _enabled = true;
        _forwardPin.deactivate();
        _reversePin.activate();
    } else {
        _enabled = false;
        _power = 0.0f;
        _forwardPin.deactivate();
        _reversePin.deactivate();
    }
    if (wasEnabled != _enabled) { handleActivation(); }
}

void AstroRelayMotorActuator::_disableActuator()
{
    bool wasEnabled = _enabled;
    _enabled = false;
    _power = 0.0f;
    _forwardPin.deactivate();
    _reversePin.deactivate();
    if (wasEnabled) { handleActivation(); }
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
    _outputPin.analogWrite(0.0f);
}

AstroAnalogActuator::AstroAnalogActuator(const AstroActuatorData *dataIn)
    : AstroActuator(dataIn), _outputPin(dataIn ? &dataIn->outputPin : nullptr)
{
    _outputPin.init();
    _outputPin.analogWrite(0.0f);
}

AstroAnalogActuator::~AstroAnalogActuator()
{
    if (_enabled) {
        _enabled = false;
        _power = 0.0f;
        _outputPin.analogWrite(0.0f);
    }
}

bool AstroAnalogActuator::getCanEnable()
{
    return _outputPin.isValid() && AstroActuator::getCanEnable();
}

void AstroAnalogActuator::_enableActuator(float intensity)
{
    bool wasEnabled = _enabled;
    _power = constrain(intensity, 0.0f, 1.0f);
    _enabled = _power > FLT_EPSILON;
    if (_outputPin.isValid()) { _outputPin.analogWrite(_power); }
    if (wasEnabled != _enabled) { handleActivation(); }
}

void AstroAnalogActuator::_disableActuator()
{
    bool wasEnabled = _enabled;
    _enabled = false;
    _power = 0.0f;
    if (_outputPin.isValid()) { _outputPin.analogWrite(0.0f); }
    if (wasEnabled) { handleActivation(); }
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
    _disableActuator();
    if (_stopCallback) { _stopCallback(_context); }
}

void AstroFocuser::update()
{
    AstroActuator::update();

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
      minimumPosition(0), maximumPosition(10000), contPowerUsage(), railName{0}
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
    if (enableMode != Astro_EnableMode_Undefined) { objectOut[SFP(AStr_Key_EnableMode)] = enableModeToString(enableMode); }
    if (outputPin.isSet()) { JsonObject pinObj = objectOut.createNestedObject(SFP(AStr_Key_OutputPin)); outputPin.toJSONObject(pinObj); }
    if (outputPin2.isSet()) { JsonObject pinObj = objectOut.createNestedObject(SFP(AStr_Key_OutputPin2)); outputPin2.toJSONObject(pinObj); }
    if (contPowerUsage.value > FLT_EPSILON) {
        JsonObject powerObj = objectOut.createNestedObject(SFP(AStr_Key_ContinuousPowerUsage));
        contPowerUsage.toJSONObject(powerObj);
    }
    if (railName[0]) { objectOut[SFP(AStr_Key_RailName)] = railName; }
    if (id.object.classType == (aid_t)AstroActuator::Focuser) {
        objectOut[SFP(AStr_Key_MinimumPosition)] = minimumPosition;
        objectOut[SFP(AStr_Key_MaximumPosition)] = maximumPosition;
    }
}

void AstroActuatorData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroObjectData::fromJSONObject(objectIn);
    JsonVariantConst enableModeVar = objectIn[SFP(AStr_Key_EnableMode)];
    if (!enableModeVar.isNull()) { enableMode = enableModeFromString(enableModeVar); }
    JsonObjectConst pinObj = objectIn[SFP(AStr_Key_OutputPin)].as<JsonObjectConst>();
    if (!pinObj.isNull()) { outputPin.fromJSONObject(pinObj); }
    JsonObjectConst pin2Obj = objectIn[SFP(AStr_Key_OutputPin2)].as<JsonObjectConst>();
    if (!pin2Obj.isNull()) { outputPin2.fromJSONObject(pin2Obj); }
    JsonVariantConst powerVar = objectIn[SFP(AStr_Key_ContinuousPowerUsage)];
    if (!powerVar.isNull()) { contPowerUsage.fromJSONVariant(powerVar); }
    const char *railNameIn = objectIn[SFP(AStr_Key_RailName)] | nullptr;
    if (railNameIn) {
        strncpy(railName, railNameIn, ASTRO_NAME_MAXSIZE - 1);
        railName[ASTRO_NAME_MAXSIZE - 1] = '\0';
    }
    minimumPosition = objectIn[SFP(AStr_Key_MinimumPosition)] | minimumPosition;
    maximumPosition = objectIn[SFP(AStr_Key_MaximumPosition)] | maximumPosition;
}
