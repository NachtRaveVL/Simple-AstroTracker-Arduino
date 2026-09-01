/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Sensors
*/

#include "Astruino.h"
#include "AstroCoreLogic.h"

AstroSensor *newSensorObjectFromData(const AstroSensorData *dataIn)
{
    if (dataIn && !isValidType(dataIn->id.object.idType)) return nullptr;
    ASTRO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(AStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData() && dataIn->id.object.idType == (aid_t)AstroIdentity::Sensor) {
        switch (dataIn->id.object.classType) {
            case (aid_t)AstroSensor::Value:
                return new AstroValueSensor(dataIn);
            case (aid_t)AstroSensor::Callback:
                return new AstroCallbackSensor(dataIn);
            case (aid_t)AstroSensor::Digital:
                return new AstroDigitalSensor(dataIn);
            case (aid_t)AstroSensor::Analog:
                return new AstroAnalogSensor(dataIn);
            default: break;
        }
    }

    return nullptr;
}

Astro_UnitsType defaultUnitsForSensor(Astro_SensorType sensorType, uint8_t measurementRow,
                                      Astro_MeasurementMode measureMode)
{
    (void)measurementRow;
    measureMode = measureMode == Astro_MeasurementMode_Undefined && getController()
                    ? getController()->getMeasurementMode() : measureMode;

    switch (sensorType) {
        case Astro_SensorType_Temperature:
        case Astro_SensorType_CameraTemperature:
            return defaultTemperatureUnits(measureMode);
        case Astro_SensorType_Humidity:
            return Astro_UnitsType_Humidity_RH;
        case Astro_SensorType_Position:
            return defaultAngleUnits(measureMode);
        case Astro_SensorType_Light:
            return Astro_UnitsType_Percentile_100;
        case Astro_SensorType_WindSpeed:
            return defaultSpeedUnits(measureMode);
        case Astro_SensorType_Voltage:
            return Astro_UnitsType_Voltage_Volts;
        case Astro_SensorType_Current:
            return Astro_UnitsType_Current_Amperage;
        case Astro_SensorType_LimitSwitch:
        case Astro_SensorType_Rain:
            return Astro_UnitsType_Raw_1;
        default:
            return Astro_UnitsType_Undefined;
    }
}

Astro_UnitsCategory defaultCategoryForSensor(Astro_SensorType sensorType, uint8_t measurementRow)
{
    (void)measurementRow;
    switch (sensorType) {
        case Astro_SensorType_Temperature:
        case Astro_SensorType_CameraTemperature:
            return Astro_UnitsCategory_Temperature;
        case Astro_SensorType_Humidity:
            return Astro_UnitsCategory_Humidity;
        case Astro_SensorType_Position:
            return Astro_UnitsCategory_Angle;
        case Astro_SensorType_Light:
            return Astro_UnitsCategory_Percentile;
        case Astro_SensorType_WindSpeed:
            return Astro_UnitsCategory_Speed;
        case Astro_SensorType_Voltage:
            return Astro_UnitsCategory_Voltage;
        case Astro_SensorType_Current:
            return Astro_UnitsCategory_Current;
        case Astro_SensorType_LimitSwitch:
        case Astro_SensorType_Rain:
            return Astro_UnitsCategory_Raw;
        default:
            return Astro_UnitsCategory_Undefined;
    }
}

AstroSensor::AstroSensor(Astro_SensorType sensorType, Astro_UnitsType units,
                         aposi_t positionIndex, int classTypeIn)
    : AstroObject(AstroIdentity(sensorType, positionIndex)),
      AstroMeasurementUnitsInterfaceStorageSingle(definedUnitsElse(units, defaultUnitsForSensor(sensorType))),
      classType((decltype(Value))classTypeIn), _sensorType(sensorType),
      _lastMeasurement(0.0, definedUnitsElse(units, defaultUnitsForSensor(sensorType)), 0, aframe_none),
      _isTakingMeasure(false), _calibrationData(nullptr), _measurementSignal()
{
    _calibrationData = getController() ? getController()->getUserCalibrationData(getKey()) : nullptr;
}

AstroSensor::AstroSensor(const AstroSensorData *dataIn)
    : AstroObject(dataIn),
      AstroMeasurementUnitsInterfaceStorageSingle(dataIn
          ? definedUnitsElse(dataIn->measurementUnits, defaultUnitsForSensor((Astro_SensorType)dataIn->id.object.objType))
          : Astro_UnitsType_Undefined),
      classType(dataIn ? (decltype(Value))dataIn->id.object.classType : Unknown),
      _sensorType(dataIn ? (Astro_SensorType)dataIn->id.object.objType : Astro_SensorType_Undefined),
      _lastMeasurement(0.0, dataIn ? definedUnitsElse(dataIn->measurementUnits,
                       defaultUnitsForSensor((Astro_SensorType)dataIn->id.object.objType)) : Astro_UnitsType_Undefined,
                       0, aframe_none),
      _isTakingMeasure(false), _calibrationData(nullptr), _measurementSignal()
{
    _calibrationData = getController() ? getController()->getUserCalibrationData(getKey()) : nullptr;
}

bool AstroSensor::takeMeasurement(bool force)
{
    if (_isTakingMeasure || (!force && !needsPolling())) { return false; }

    _isTakingMeasure = true;
    double value = 0.0;
    if (!readValue(&value)) {
        _isTakingMeasure = false;
        return false;
    }

    Astro_UnitsType units = getMeasurementUnits();
    AstroSingleMeasurement measurement(value, units, unixNow(),
        getController() ? max((aframe_t)1, getController()->getPollingFrame()) : (aframe_t)1);
    calibrationTransform(&measurement);
    if (units != Astro_UnitsType_Undefined && measurement.units != units) {
        convertUnits(&measurement, units);
    }

    finishMeasurement(measurement);
    return true;
}

const AstroMeasurement *AstroSensor::getMeasurement(bool poll)
{
    if (poll || needsPolling()) { takeMeasurement(true); }
    return &_lastMeasurement;
}

bool AstroSensor::needsPolling(aframe_t allowance) const
{
    return getController() ? getController()->isPollingFrameOld(_lastMeasurement.frame, allowance)
                           : !_lastMeasurement.isSet();
}

void AstroSensor::update()
{
    AstroObject::update();
}

void AstroSensor::yieldForMeasurement(millis_t timeout)
{
    const millis_t stopTime = millis() + timeout;
    while (isTakingMeasurement() && stopTime - millis() > 0) { yield(); }
}

void AstroSensor::finishMeasurement(const AstroSingleMeasurement &measurement)
{
    _lastMeasurement = measurement;
    _isTakingMeasure = false;

#ifdef ASTRO_USE_MULTITASKING
    scheduleSignalFireOnce<const AstroMeasurement *>(getSharedPtr(), _measurementSignal, &_lastMeasurement);
#else
    _measurementSignal.fire(&_lastMeasurement);
#endif
    publishData(this);
}

void AstroSensor::setUserCalibrationData(AstroCalibrationData *userCalibrationData)
{
    if (_calibrationData && _calibrationData != userCalibrationData) { bumpRevisionIfNeeded(); }
    if (getController()) {
        if (userCalibrationData && getController()->setUserCalibrationData(userCalibrationData)) {
            _calibrationData = getController()->getUserCalibrationData(getKey());
        } else if (!userCalibrationData && _calibrationData && getController()->dropUserCalibrationData(_calibrationData)) {
            _calibrationData = nullptr;
        }
    } else { _calibrationData = userCalibrationData; }
}

void AstroSensor::setMeasurementUnits(Astro_UnitsType units, uint8_t measurementRow)
{
    if (!measurementRow && _measurementUnits[0] != units) {
        _measurementUnits[0] = units;
        if (_lastMeasurement.isSet()) { convertUnits(&_lastMeasurement, units); }
        bumpRevisionIfNeeded();
    }
}

Astro_UnitsType AstroSensor::getMeasurementUnits(uint8_t measurementRow) const
{
    return measurementRow ? Astro_UnitsType_Undefined : _measurementUnits[0];
}

Signal<const AstroMeasurement *, ASTRO_SENSOR_SIGNAL_SLOTS> &AstroSensor::getMeasurementSignal()
{
    return _measurementSignal;
}

AstroData *AstroSensor::allocateData() const
{
    return _allocateDataForObjType((aid_t)AstroIdentity::Sensor, (aid_t)classType);
}

void AstroSensor::saveToData(AstroData *dataOut)
{
    AstroObject::saveToData(dataOut);
    if (!dataOut) { return; }
    AstroSensorData *sensorData = static_cast<AstroSensorData *>(dataOut);
    sensorData->id.object.classType = (aid_t)classType;
    sensorData->measurementUnits = getMeasurementUnits();
}

AstroDigitalSensor::AstroDigitalSensor(AstroDigitalPin inputPin, Astro_SensorType sensorType,
                                       aposi_t positionIndex)
    : AstroSensor(sensorType, Astro_UnitsType_Raw_1, positionIndex, Digital), _inputPin(inputPin),
      _usingISR(false), _pendingState(false), _hasPendingState(false),
      _pendingStateStart(millis_none), _stateStableTimeMs(ASTRO_SENSOR_BINARY_STABLE_MILLIS), _stateSignal()
{
    _inputPin.init();
}

AstroDigitalSensor::AstroDigitalSensor(const AstroSensorData *dataIn)
    : AstroSensor(dataIn), _inputPin(dataIn ? &dataIn->inputPin : nullptr),
      _usingISR(false), _pendingState(false), _hasPendingState(false),
      _pendingStateStart(millis_none),
      _stateStableTimeMs(dataIn ? dataIn->stateStableTimeMs : ASTRO_SENSOR_BINARY_STABLE_MILLIS), _stateSignal()
{
    _inputPin.init();
    if (dataIn && dataIn->usingISR) { tryRegisterISR(); }
}

AstroDigitalSensor::~AstroDigitalSensor()
{
    if (_usingISR) {
        // TODO: detach ISR from taskManager (not currently possible, maybe in future?)
    }
}

bool AstroDigitalSensor::readValue(double *valueOut)
{
    if (!valueOut || !_inputPin.isValid()) { return false; }
    *valueOut = _inputPin.isActive() ? 1.0 : 0.0;
    return true;
}

bool AstroDigitalSensor::takeMeasurement(bool force)
{
    if (!_inputPin.isValid() || _isTakingMeasure || (!force && !needsPolling())) { return false; }

    _isTakingMeasure = true;
    const bool stateBefore = _lastMeasurement.isSet() && _lastMeasurement.value >= 0.5;
    const bool sampledState = _inputPin.isActive();
    uint32_t pendingStateStart = _pendingStateStart;
    const bool state = astroUpdateStableBinaryState(stateBefore, sampledState, millis(), _stateStableTimeMs,
                                                    _pendingState, _hasPendingState, pendingStateStart);
    _pendingStateStart = pendingStateStart;

    AstroSingleMeasurement measurement(state ? 1.0 : 0.0, getMeasurementUnits(), unixNow(),
        getController() ? max((aframe_t)1, getController()->getPollingFrame()) : (aframe_t)1);
    finishMeasurement(measurement);

    if (state != stateBefore) {
#ifdef ASTRO_USE_MULTITASKING
        scheduleSignalFireOnce<bool>(getSharedPtr(), _stateSignal, state);
#else
        _stateSignal.fire(state);
#endif
    }
    return true;
}

void AstroDigitalSensor::setMeasurementUnits(Astro_UnitsType measurementUnits, uint8_t)
{
    if (measurementUnits != Astro_UnitsType_Raw_1) {
        ASTRO_SOFT_ASSERT(false, SFP(AStr_Err_UnsupportedOperation));
    }
    if (_measurementUnits[0] != Astro_UnitsType_Raw_1) {
        _measurementUnits[0] = Astro_UnitsType_Raw_1;
        bumpRevisionIfNeeded();
    }
}

Astro_UnitsType AstroDigitalSensor::getMeasurementUnits(uint8_t) const
{
    return _calibrationData ? _calibrationData->calibrationUnits : Astro_UnitsType_Raw_1;
}

bool AstroDigitalSensor::tryRegisterISR(bool anyChange)
{
#ifdef ASTRO_USE_MULTITASKING
    if (!_usingISR && _inputPin.isValid() && checkPinCanInterrupt(_inputPin.pin)) {
        taskManager.addInterrupt(&interruptImpl, _inputPin.pin,
                                 !anyChange ? (_inputPin.activeLow ? FALLING : RISING) : CHANGE);
        _usingISR = true;
    }
#else
    (void)anyChange;
#endif
    return _usingISR;
}

void AstroDigitalSensor::setStateStableTime(uint16_t stableTimeMs)
{
    if (_stateStableTimeMs != stableTimeMs) {
        _stateStableTimeMs = stableTimeMs;
        _hasPendingState = false;
        bumpRevisionIfNeeded();
    }
}

Signal<bool, ASTRO_SENSOR_SIGNAL_SLOTS> &AstroDigitalSensor::getStateSignal()
{
    return _stateSignal;
}

void AstroDigitalSensor::saveToData(AstroData *dataOut)
{
    AstroSensor::saveToData(dataOut);
    if (dataOut) {
        AstroSensorData *sensorData = static_cast<AstroSensorData *>(dataOut);
        _inputPin.saveToData(&sensorData->inputPin);
        sensorData->usingISR = _usingISR;
        sensorData->stateStableTimeMs = _stateStableTimeMs;
    }
}

AstroAnalogSensor::AstroAnalogSensor(AstroAnalogPin inputPin, Astro_SensorType sensorType,
                                     Astro_UnitsType units, aposi_t positionIndex)
    : AstroSensor(sensorType, units, positionIndex, Analog), _inputPin(inputPin)
{
    _inputPin.init();
}

AstroAnalogSensor::AstroAnalogSensor(const AstroSensorData *dataIn)
    : AstroSensor(dataIn), _inputPin(dataIn ? &dataIn->inputPin : nullptr)
{
    _inputPin.init();
}

bool AstroAnalogSensor::readValue(double *valueOut)
{
    if (!valueOut || !_inputPin.isValid()) { return false; }

#if ASTRO_SENSOR_ANALOGREAD_SAMPLES > 1
    uint32_t rawRead = 0;
    for (int sampleIndex = 0; sampleIndex < ASTRO_SENSOR_ANALOGREAD_SAMPLES; ++sampleIndex) {
#if ASTRO_SENSOR_ANALOGREAD_DELAY > 0
        if (sampleIndex) { delay(ASTRO_SENSOR_ANALOGREAD_DELAY); }
#endif
        rawRead += _inputPin.analogRead_raw();
    }
    rawRead /= ASTRO_SENSOR_ANALOGREAD_SAMPLES;
    *valueOut = _inputPin.bitRes.transform(rawRead);
#else
    *valueOut = _inputPin.analogRead();
#endif
    return true;
}

void AstroAnalogSensor::saveToData(AstroData *dataOut)
{
    AstroSensor::saveToData(dataOut);
    if (dataOut) { _inputPin.saveToData(&static_cast<AstroSensorData *>(dataOut)->inputPin); }
}


AstroSensorData::AstroSensorData()
    : AstroObjectData(), measurementUnits(Astro_UnitsType_Undefined), inputPin(),
      usingISR(false), stateStableTimeMs(ASTRO_SENSOR_BINARY_STABLE_MILLIS)
{
    _size = sizeof(*this);
    _version = 2;
}

void AstroSensorData::toJSONObject(JsonObject &objectOut) const
{
    AstroObjectData::toJSONObject(objectOut);
    if (measurementUnits != Astro_UnitsType_Undefined) { objectOut[SFP(AStr_Key_MeasurementUnits)] = unitsTypeToSymbol(measurementUnits); }
    if (inputPin.isSet()) {
        JsonObject pinObj = objectOut.createNestedObject(SFP(AStr_Key_InputPin));
        inputPin.toJSONObject(pinObj);
    }
    if (id.object.classType == (aid_t)AstroSensor::Digital) {
        if (usingISR != false) { objectOut[SFP(AStr_Key_UsingISR)] = usingISR; }
        if (stateStableTimeMs != ASTRO_SENSOR_BINARY_STABLE_MILLIS) { objectOut[SFP(AStr_Key_StateStableTimeMs)] = stateStableTimeMs; }
    }
}

void AstroSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroObjectData::fromJSONObject(objectIn);
    measurementUnits = unitsTypeFromSymbol(objectIn[SFP(AStr_Key_MeasurementUnits)]);
    JsonObjectConst pinObj = objectIn[SFP(AStr_Key_InputPin)].as<JsonObjectConst>();
    if (!pinObj.isNull()) { inputPin.fromJSONObject(pinObj); }
    usingISR = objectIn[SFP(AStr_Key_UsingISR)] | usingISR;
    stateStableTimeMs = objectIn[SFP(AStr_Key_StateStableTimeMs)] | stateStableTimeMs;
}
