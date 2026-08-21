/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Sensors
*/

#include "AstroSensors.h"

AstroSensor::AstroSensor(Astro_SensorType sensorType, Astro_UnitsType units, aposi_t positionIndex)
    : AstroObject(AstroIdentity(sensorType, positionIndex)), _sensorType(sensorType),
      _measurement(0.0, units, 0, aframe_none), _rawMinimum(0.0), _rawMaximum(1.0),
      _valueMinimum(0.0), _valueMaximum(1.0), _calibrated(false)
{ ; }

AstroSensor::AstroSensor(const AstroObjectData *dataIn, Astro_UnitsType units)
    : AstroObject(dataIn), _sensorType(dataIn ? (Astro_SensorType)dataIn->objType : Astro_SensorType_Undefined),
      _measurement(0.0, units, 0, aframe_none), _rawMinimum(0.0), _rawMaximum(1.0),
      _valueMinimum(0.0), _valueMaximum(1.0), _calibrated(false)
{ ; }

bool AstroSensor::poll(int64_t timestamp, aframe_t frame)
{
    double value;
    if (!readValue(&value)) { return false; }
    if (_calibrated) {
        const double ratio = (value - _rawMinimum) / (_rawMaximum - _rawMinimum);
        value = _valueMinimum + ratio * (_valueMaximum - _valueMinimum);
    }
    _measurement.value = value;
    _measurement.timestamp = timestamp;
    _measurement.frame = frame ? frame : 1;
    return true;
}

bool AstroSensor::setCalibration(double rawMinimum, double rawMaximum,
                                 double valueMinimum, double valueMaximum,
                                 Astro_UnitsType units)
{
    if (isFPEqual(rawMinimum, rawMaximum) || units == Astro_UnitsType_Undefined) { return false; }
    _rawMinimum = rawMinimum;
    _rawMaximum = rawMaximum;
    _valueMinimum = valueMinimum;
    _valueMaximum = valueMaximum;
    _calibrated = true;
    setUnits(units);
    return true;
}

void AstroSensor::clearCalibration()
{
    _calibrated = false;
    _rawMinimum = 0.0;
    _rawMaximum = 1.0;
    _valueMinimum = 0.0;
    _valueMaximum = 1.0;
    setUnits(Astro_UnitsType_Raw_1);
}

bool AstroSensor::getCalibration(double &rawMinimum, double &rawMaximum,
                                 double &valueMinimum, double &valueMaximum) const
{
    if (!_calibrated) { return false; }
    rawMinimum = _rawMinimum;
    rawMaximum = _rawMaximum;
    valueMinimum = _valueMinimum;
    valueMaximum = _valueMaximum;
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
