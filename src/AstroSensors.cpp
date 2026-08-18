/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Sensors
*/

#include "AstroSensors.h"

AstroSensor::AstroSensor(Astro_SensorType sensorType, Astro_UnitsType units, aposi_t positionIndex)
    : AstroObject(AstroIdentity(sensorType, positionIndex)), _sensorType(sensorType),
      _measurement(0.0, units, 0, aframe_none)
{ ; }

AstroSensor::AstroSensor(const AstroObjectData *dataIn, Astro_UnitsType units)
    : AstroObject(dataIn), _sensorType(dataIn ? (Astro_SensorType)dataIn->objType : Astro_SensorType_Undefined),
      _measurement(0.0, units, 0, aframe_none)
{ ; }

bool AstroSensor::poll(int64_t timestamp, aframe_t frame)
{
    double value;
    if (!readValue(&value)) { return false; }
    _measurement.value = value;
    _measurement.timestamp = timestamp;
    _measurement.frame = frame ? frame : 1;
    return true;
}

AstroDigitalSensor::AstroDigitalSensor(AstroDigitalPin inputPin, Astro_SensorType sensorType, aposi_t positionIndex)
    : AstroSensor(sensorType, Astro_UnitsType_Raw_1, positionIndex), _inputPin(inputPin)
{
    _inputPin.init();
}

bool AstroDigitalSensor::readValue(double *valueOut)
{
    if (!valueOut || !_inputPin.isValid()) { return false; }
    *valueOut = _inputPin.isActive() ? 1.0 : 0.0;
    return true;
}

AstroAnalogSensor::AstroAnalogSensor(AstroAnalogPin inputPin, Astro_SensorType sensorType, Astro_UnitsType units, aposi_t positionIndex)
    : AstroSensor(sensorType, units, positionIndex), _inputPin(inputPin)
{
    _inputPin.init();
}

bool AstroAnalogSensor::readValue(double *valueOut)
{
    if (!valueOut || !_inputPin.isValid()) { return false; }
    *valueOut = _inputPin.analogRead();
    return true;
}
