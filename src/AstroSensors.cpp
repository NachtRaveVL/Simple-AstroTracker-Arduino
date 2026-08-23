/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Sensors
*/

#include "Astruino.h"

AstroSensor *newSensorObjectFromData(const AstroSensorData *dataIn)
{
    if (!dataIn) { return nullptr; }

    switch (dataIn->id.object.classType) {
        case (aid_t)AstroSensor::Value:
            return new AstroValueSensor(dataIn);
        case (aid_t)AstroSensor::Callback:
            return new AstroCallbackSensor(dataIn);
        case (aid_t)AstroSensor::Digital:
            return new AstroDigitalSensor(dataIn);
        case (aid_t)AstroSensor::Analog:
            return new AstroAnalogSensor(dataIn);
        default:
            return nullptr;
    }
}

AstroSensor::AstroSensor(Astro_SensorType sensorType, Astro_UnitsType units, aposi_t positionIndex, int classTypeIn)
    : AstroObject(AstroIdentity(sensorType, positionIndex)), classType((decltype(Value))classTypeIn),
      _sensorType(sensorType), _measurement(0.0, units, 0, aframe_none), _calibrationData(nullptr), _measurementSignal()
{ ; }

AstroSensor::AstroSensor(const AstroSensorData *dataIn)
    : AstroObject(dataIn), classType(dataIn ? (decltype(Value))dataIn->id.object.classType : Unknown),
      _sensorType(dataIn ? (Astro_SensorType)dataIn->id.object.objType : Astro_SensorType_Undefined),
      _measurement(0.0, dataIn ? dataIn->measurementUnits : Astro_UnitsType_Undefined, 0, aframe_none),
      _calibrationData(nullptr), _measurementSignal()
{ ; }

bool AstroSensor::poll(int64_t timestamp, aframe_t frame)
{
    double value;
    if (!readValue(&value)) { return false; }
    _measurement.value = value;
    _measurement.timestamp = timestamp;
    _measurement.frame = frame ? frame : 1;
    calibrationTransform(&_measurement);
    _measurementSignal.fire(&_measurement);
    return true;
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
Signal<const AstroMeasurement *, ASTRO_DEFAULT_MAXSIZE> &AstroSensor::getMeasurementSignal()
{ return _measurementSignal; }
AstroData *AstroSensor::allocateData() const
{
    return _allocateDataForObjType((aid_t)AstroIdentity::Sensor, (aid_t)classType);
}

void AstroSensor::saveToData(AstroData *dataOut) const
{
    AstroObject::saveToData(dataOut);
    if (!dataOut) { return; }
    AstroSensorData *sensorData = static_cast<AstroSensorData *>(dataOut);
    sensorData->id.object.classType = (aid_t)classType;
    sensorData->measurementUnits = getUnits();
}

AstroDigitalSensor::AstroDigitalSensor(AstroDigitalPin inputPin, Astro_SensorType sensorType, aposi_t positionIndex)
    : AstroSensor(sensorType, Astro_UnitsType_Raw_1, positionIndex, Digital), _inputPin(inputPin) { _inputPin.init(); }
AstroDigitalSensor::AstroDigitalSensor(const AstroSensorData *dataIn)
    : AstroSensor(dataIn), _inputPin(dataIn ? &dataIn->inputPin : nullptr) { _inputPin.init(); }
bool AstroDigitalSensor::readValue(double *valueOut)
{ if (!valueOut || !_inputPin.isValid()) { return false; } *valueOut = _inputPin.isActive() ? 1.0 : 0.0; return true; }
AstroAnalogSensor::AstroAnalogSensor(AstroAnalogPin inputPin, Astro_SensorType sensorType, Astro_UnitsType units, aposi_t positionIndex)
    : AstroSensor(sensorType, units, positionIndex, Analog), _inputPin(inputPin) { _inputPin.init(); }
AstroAnalogSensor::AstroAnalogSensor(const AstroSensorData *dataIn)
    : AstroSensor(dataIn), _inputPin(dataIn ? &dataIn->inputPin : nullptr) { _inputPin.init(); }
bool AstroAnalogSensor::readValue(double *valueOut)
{ if (!valueOut || !_inputPin.isValid()) { return false; } *valueOut = _inputPin.analogRead(); return true; }

void AstroDigitalSensor::saveToData(AstroData *dataOut) const
{
    AstroSensor::saveToData(dataOut);
    if (dataOut) { _inputPin.saveToData(&static_cast<AstroSensorData *>(dataOut)->inputPin); }
}

void AstroAnalogSensor::saveToData(AstroData *dataOut) const
{
    AstroSensor::saveToData(dataOut);
    if (dataOut) { _inputPin.saveToData(&static_cast<AstroSensorData *>(dataOut)->inputPin); }
}


AstroSensorData::AstroSensorData()
    : AstroObjectData(), measurementUnits(Astro_UnitsType_Undefined), inputPin()
{
    _size = sizeof(*this);
}

void AstroSensorData::toJSONObject(JsonObject &objectOut) const
{
    AstroObjectData::toJSONObject(objectOut);
    objectOut["measurementUnits"] = (int)measurementUnits;
    if (inputPin.isSet()) {
        JsonObject pinObj = objectOut.createNestedObject("inputPin");
        inputPin.toJSONObject(pinObj);
    }
}

void AstroSensorData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroObjectData::fromJSONObject(objectIn);
    measurementUnits = (Astro_UnitsType)(objectIn["measurementUnits"] | (int)measurementUnits);
    JsonObjectConst pinObj = objectIn["inputPin"].as<JsonObjectConst>();
    if (!pinObj.isNull()) { inputPin.fromJSONObject(pinObj); }
}
