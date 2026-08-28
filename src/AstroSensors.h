/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Sensors
*/

#ifndef AstroSensors_H
#define AstroSensors_H

struct AstroSensorData;

#include "AstroDatas.h"
#include "AstroMeasurements.h"
#include "AstroObject.h"
#include "AstroPins.h"

// Creates sensor object from passed sensor data (return ownership transfer - user code *must* delete returned object)
extern AstroSensor *newSensorObjectFromData(const AstroSensorData *dataIn);

// Sensor Base
// Base class for measurements supplied by pins, callbacks, or external sensor adapters.
class AstroSensor : public AstroObject, public AstroSensorObjectInterface, public AstroMeasurementUnitsInterface {
public:
    const enum : signed char { Value, Callback, Digital, Analog, Unknown = -1 } classType;
    AstroSensor(Astro_SensorType sensorType = Astro_SensorType_Undefined,
                Astro_UnitsType units = Astro_UnitsType_Undefined,
                aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG,
                int classTypeIn = Unknown);
    AstroSensor(const AstroSensorData *dataIn);
    virtual ~AstroSensor() { ; }

    virtual bool readValue(double *valueOut) = 0;
    virtual bool poll(int64_t timestamp = 0, aframe_t frame = 1) override;

    void setUserCalibrationData(AstroCalibrationData *userCalibrationData);
    inline const AstroCalibrationData *getUserCalibrationData() const { return _calibrationData; }

    // Transformation methods that convert from normalized reading intensity/driver value to calibration units.
    inline double calibrationTransform(double value) const { return _calibrationData ? _calibrationData->transform(value) : value; }
    inline void calibrationTransform(double *valueInOut, Astro_UnitsType *unitsOut = nullptr) const { if (valueInOut && _calibrationData) { _calibrationData->transform(valueInOut, unitsOut); } }
    inline AstroSingleMeasurement calibrationTransform(AstroSingleMeasurement measurement) const { return _calibrationData ? _calibrationData->transform(measurement) : measurement; }
    inline void calibrationTransform(AstroSingleMeasurement *measurementInOut) const { if (measurementInOut && _calibrationData) { _calibrationData->transform(measurementInOut); } }

    // Transformation methods that convert from calibration units to normalized reading intensity/driver value.
    inline double calibrationInvTransform(double value) const { return _calibrationData ? _calibrationData->inverseTransform(value) : value; }
    inline void calibrationInvTransform(double *valueInOut, Astro_UnitsType *unitsOut = nullptr) const { if (valueInOut && _calibrationData) { _calibrationData->inverseTransform(valueInOut, unitsOut); } }
    inline AstroSingleMeasurement calibrationInvTransform(AstroSingleMeasurement measurement) const { return _calibrationData ? _calibrationData->inverseTransform(measurement) : measurement; }
    inline void calibrationInvTransform(AstroSingleMeasurement *measurementInOut) const { if (measurementInOut && _calibrationData) { _calibrationData->inverseTransform(measurementInOut); } }

    inline Astro_SensorType getSensorType() const { return _sensorType; }
    inline Astro_UnitsType getUnits() const { return _measurement.units; }
    inline void setUnits(Astro_UnitsType units) { _measurement.units = units; }
    virtual void setMeasurementUnits(Astro_UnitsType units, uint8_t measurementRow = 0) override { if (measurementRow == 0) { setUnits(units); } }
    virtual Astro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const override { return measurementRow == 0 ? getUnits() : Astro_UnitsType_Undefined; }
    virtual const AstroSingleMeasurement &getMeasurement() const override { return _measurement; }
    Signal<const AstroMeasurement *, ASTRO_SENSOR_SIGNAL_SLOTS> &getMeasurementSignal();

protected:
    Astro_SensorType _sensorType;                           // Sensor type
    AstroSingleMeasurement _measurement;                   // Latest sensor measurement
    const AstroCalibrationData *_calibrationData;           // Calibration data
    Signal<const AstroMeasurement *, ASTRO_SENSOR_SIGNAL_SLOTS> _measurementSignal; // Measurement signal

    virtual AstroData *allocateData() const override;
    virtual void saveToData(AstroData *dataOut) const override;
};

// Value Sensor
// Stores a user supplied scalar measurement.
class AstroValueSensor : public AstroSensor {
public:
    AstroValueSensor(Astro_SensorType sensorType = Astro_SensorType_Undefined,
                     Astro_UnitsType units = Astro_UnitsType_Undefined,
                     aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG,
                     double value = 0.0)
        : AstroSensor(sensorType, units, positionIndex, Value), _value(value) { ; }
    AstroValueSensor(const AstroSensorData *dataIn, double value = 0.0)
        : AstroSensor(dataIn), _value(value) { ; }
    virtual bool readValue(double *valueOut) override { if (!valueOut) { return false; } *valueOut = _value; return true; }
    inline void setValue(double value) { _value = value; }
    inline double getValue() const { return _value; }
protected:
    double _value;                                         // Stored sensor value
};

// Callback Sensor
// Obtains sensor measurements through a user supplied callback.
class AstroCallbackSensor : public AstroSensor {
public:
    typedef bool (*ReadCallback)(void *context, double *valueOut);

    AstroCallbackSensor(ReadCallback callback = nullptr, void *context = nullptr,
                        Astro_SensorType sensorType = Astro_SensorType_Undefined,
                        Astro_UnitsType units = Astro_UnitsType_Undefined,
                        aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG)
        : AstroSensor(sensorType, units, positionIndex, Callback), _callback(callback), _context(context) { ; }
    AstroCallbackSensor(const AstroSensorData *dataIn)
        : AstroSensor(dataIn), _callback(nullptr), _context(nullptr) { ; }
    virtual bool readValue(double *valueOut) override { return _callback ? _callback(_context, valueOut) : false; }
protected:
    ReadCallback _callback;                                // Callback function
    void *_context;                                        // Callback user context
};

// Digital Sensor
// Reads a binary measurement from a digital input pin.
class AstroDigitalSensor : public AstroSensor {
public:
    AstroDigitalSensor(AstroDigitalPin inputPin = AstroDigitalPin(),
                       Astro_SensorType sensorType = Astro_SensorType_Undefined,
                       aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    AstroDigitalSensor(const AstroSensorData *dataIn);
    virtual bool readValue(double *valueOut) override;
    inline const AstroDigitalPin &getInputPin() const { return _inputPin; }
protected:
    AstroDigitalPin _inputPin;                             // Input pin
    virtual void saveToData(AstroData *dataOut) const override;
};

class AstroAnalogSensor : public AstroSensor {
public:
    AstroAnalogSensor(AstroAnalogPin inputPin = AstroAnalogPin(),
                      Astro_SensorType sensorType = Astro_SensorType_Undefined,
                      Astro_UnitsType units = Astro_UnitsType_Raw_1,
                      aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index

    AstroAnalogSensor(const AstroSensorData *dataIn);
    virtual bool readValue(double *valueOut) override;
    inline const AstroAnalogPin &getInputPin() const { return _inputPin; }
protected:
    AstroAnalogPin _inputPin;                              // Input pin
    virtual void saveToData(AstroData *dataOut) const override;
};


// Sensor Serialization Data
struct AstroSensorData : public AstroObjectData
{
    Astro_UnitsType measurementUnits;                       // Measurement units
    AstroPinData inputPin;                                  // Input pin

    AstroSensorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef AstroSensors_H
