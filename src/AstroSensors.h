/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Sensors
*/

#ifndef AstroSensors_H
#define AstroSensors_H

class AstroSensor;
class AstroValueSensor;
class AstroCallbackSensor;
class AstroBinarySensor;
class AstroAnalogSensor;
class AstroDigitalSensor;

struct AstroSensorData;
struct AstroBinarySensorData;
struct AstroAnalogSensorData;
struct AstroDigitalSensorData;

#include "Astruino.h"
#include "AstroDatas.h"

// Creates sensor object from passed sensor data (return ownership transfer - user code *must* delete returned object)
extern AstroSensor *newSensorObjectFromData(const AstroSensorData *dataIn);

// Returns the default measurement units for a sensor type.
extern Astro_UnitsType defaultUnitsForSensor(Astro_SensorType sensorType, uint8_t measurementRow = 0,
                                              Astro_MeasurementMode measureMode = Astro_MeasurementMode_Undefined);
// Returns the default measurement category for a sensor type.
extern Astro_UnitsCategory defaultCategoryForSensor(Astro_SensorType sensorType, uint8_t measurementRow = 0);

// Sensor Base
// Base class for measurements supplied by pins, callbacks, or external sensor adapters.
class AstroSensor : public AstroObject, public AstroSensorObjectInterface,
                    public AstroMeasurementUnitsInterfaceStorageSingle {
public:
    const enum : signed char { Value, Callback, Binary, Analog, Digital, Unknown = -1 } classType;
    inline bool isValueClass() const { return classType == Value; }
    inline bool isCallbackClass() const { return classType == Callback; }
    inline bool isBinaryClass() const { return classType == Binary; }
    inline bool isAnalogClass() const { return classType == Analog; }
    inline bool isDigitalClass() const { return classType == Digital; }
    inline bool isUnknownClass() const { return classType <= Unknown; }

    AstroSensor(Astro_SensorType sensorType = Astro_SensorType_Undefined,
                Astro_UnitsType units = Astro_UnitsType_Undefined,
                aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG,
                int classTypeIn = Unknown);
    AstroSensor(const AstroSensorData *dataIn);
    virtual ~AstroSensor() { _isTakingMeasure = false; }

    virtual bool readValue(double *valueOut) = 0;
    virtual bool takeMeasurement(bool force = false) override;
    virtual const AstroMeasurement *getMeasurement(bool poll = false) override;
    virtual bool isTakingMeasurement() const override { return _isTakingMeasure; }
    virtual bool needsPolling(aframe_t allowance = 0) const override;
    virtual void update() override;

    void yieldForMeasurement(millis_t timeout = ASTRO_DATA_LOOP_INTERVAL);

    void setUserCalibrationData(AstroCalibrationData *userCalibrationData);
    inline const AstroCalibrationData *getUserCalibrationData() const { return _calibrationData; }

    // Transformation methods that convert from normalized reading intensity/driver value to calibration units.
    inline float calibrationTransform(float value) const { return _calibrationData ? _calibrationData->transform(value) : value; }
    inline void calibrationTransform(float *valueInOut, Astro_UnitsType *unitsOut = nullptr) const { if (valueInOut && _calibrationData) { _calibrationData->transform(valueInOut, unitsOut); } }
    inline AstroSingleMeasurement calibrationTransform(AstroSingleMeasurement measurement) { return _calibrationData ? AstroSingleMeasurement(_calibrationData->transform(measurement.value), _calibrationData->calibrationUnits, measurement.timestamp, measurement.frame) : measurement; }
    inline void calibrationTransform(AstroSingleMeasurement *measurementInOut) const { if (measurementInOut && _calibrationData) { _calibrationData->transform(&measurementInOut->value, &measurementInOut->units); } }

    // Transformation methods that convert from calibration units to normalized reading intensity/driver value.
    inline float calibrationInvTransform(float value) const { return _calibrationData ? _calibrationData->inverseTransform(value) : value; }
    inline void calibrationInvTransform(float *valueInOut, Astro_UnitsType *unitsOut = nullptr) const { if (valueInOut && _calibrationData) { _calibrationData->inverseTransform(valueInOut, unitsOut); } }
    inline AstroSingleMeasurement calibrationInvTransform(AstroSingleMeasurement measurement) { return _calibrationData ? AstroSingleMeasurement(_calibrationData->inverseTransform(measurement.value), Astro_UnitsType_Raw_1, measurement.timestamp, measurement.frame) : measurement; }
    inline void calibrationInvTransform(AstroSingleMeasurement *measurementInOut) const { if (measurementInOut && _calibrationData) { _calibrationData->inverseTransform(&measurementInOut->value, &measurementInOut->units); } }

    inline Astro_SensorType getSensorType() const { return _sensorType; }
    inline Astro_UnitsType getUnits() const { return getMeasurementUnits(); }
    inline void setUnits(Astro_UnitsType units) { setMeasurementUnits(units); }
    virtual void setMeasurementUnits(Astro_UnitsType units, uint8_t measurementRow = 0) override;
    virtual Astro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const override;
    Signal<const AstroMeasurement *, ASTRO_SENSOR_SIGNAL_SLOTS> &getMeasurementSignal();

protected:
    Astro_SensorType _sensorType;                           // Sensor type
    AstroSingleMeasurement _lastMeasurement;                // Latest sensor measurement
    bool _isTakingMeasure;                                  // Measurement in progress flag
    const AstroCalibrationData *_calibrationData;           // Calibration data
    Signal<const AstroMeasurement *, ASTRO_SENSOR_SIGNAL_SLOTS> _measurementSignal; // Measurement signal

    void finishMeasurement(const AstroSingleMeasurement &measurement);

    virtual AstroData *allocateData() const override;
    virtual void saveToData(AstroData *dataOut) override;
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

// Simple Binary Sensor
// Reads a binary measurement from a digital input pin. Binary inputs support the same
// interrupt-driven notification path and minimum-stable-time filtering as sibling libraries.
class AstroBinarySensor : public AstroSensor {
public:
    AstroBinarySensor(AstroDigitalPin inputPin = AstroDigitalPin(),
                      Astro_SensorType sensorType = Astro_SensorType_Undefined,
                      aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    AstroBinarySensor(const AstroBinarySensorData *dataIn);
    virtual ~AstroBinarySensor();

    virtual bool readValue(double *valueOut) override;
    virtual bool takeMeasurement(bool force = false) override;

    virtual void setMeasurementUnits(Astro_UnitsType measurementUnits, uint8_t measurementRow = 0) override;
    virtual Astro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const override;

    bool tryRegisterISR(bool anyChange = false);
    inline void notifyISRTriggered() { takeMeasurement(true); }

    void setStateStableTime(uint16_t stableTimeMs);
    inline uint16_t getStateStableTime() const { return _stateStableTimeMs; }
    inline bool isUsingISR() const { return _usingISR; }
    inline bool isActive(bool poll = false) { return getMeasurementValue(getMeasurement(poll)) >= 0.5; }
    inline const AstroDigitalPin &getInputPin() const { return _inputPin; }
    Signal<bool, ASTRO_SENSOR_SIGNAL_SLOTS> &getStateSignal();

protected:
    AstroDigitalPin _inputPin;                              // Input pin
    bool _usingISR;                                         // Using ISR flag
    bool _pendingState;                                     // Pending stable-state candidate
    bool _hasPendingState;                                  // Pending state exists flag
    millis_t _pendingStateStart;                            // Pending state start time
    uint16_t _stateStableTimeMs;                            // Required stable time before accepting a state change
    Signal<bool, ASTRO_SENSOR_SIGNAL_SLOTS> _stateSignal;   // Binary state-change signal

    virtual void saveToData(AstroData *dataOut) override;
};

// Analog Sensor
// Reads a normalized analog value and applies optional calibration/unit conversion.
class AstroAnalogSensor : public AstroSensor {
public:
    AstroAnalogSensor(AstroAnalogPin inputPin = AstroAnalogPin(),
                      Astro_SensorType sensorType = Astro_SensorType_Undefined,
                      Astro_UnitsType units = Astro_UnitsType_Raw_1,
                      aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index

    AstroAnalogSensor(const AstroAnalogSensorData *dataIn);
    virtual bool readValue(double *valueOut) override;
    inline const AstroAnalogPin &getInputPin() const { return _inputPin; }
protected:
    AstroAnalogPin _inputPin;                              // Input pin
    virtual void saveToData(AstroData *dataOut) override;
};

// Digital Sensor
// Intermediate class for digital protocol sensors. Binary switch-style inputs use AstroBinarySensor.
class AstroDigitalSensor : public AstroSensor {
public:
    AstroDigitalSensor(AstroDigitalPin inputPin = AstroDigitalPin(),
                       Astro_SensorType sensorType = Astro_SensorType_Undefined,
                       Astro_UnitsType units = Astro_UnitsType_Undefined,
                       aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG,
                       int classTypeIn = Digital);
    AstroDigitalSensor(const AstroDigitalSensorData *dataIn);

    inline const AstroDigitalPin &getInputPin() const { return _inputPin; }

protected:
    AstroDigitalPin _inputPin;                              // Digital input pin

    virtual void saveToData(AstroData *dataOut) override;
};

// Sensor Serialization Data
struct AstroSensorData : public AstroObjectData
{
    Astro_UnitsType measurementUnits;                       // Measurement units

    AstroSensorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Binary Sensor Serialization Data
struct AstroBinarySensorData : public AstroSensorData
{
    AstroPinData inputPin;                                  // Input pin
    bool usingISR;                                          // Using ISR flag
    uint16_t stateStableTimeMs;                             // Minimum stable time before state change is accepted

    AstroBinarySensorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Analog Sensor Serialization Data
struct AstroAnalogSensorData : public AstroSensorData
{
    AstroPinData inputPin;                                  // Input pin

    AstroAnalogSensorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

// Digital Sensor Serialization Data
struct AstroDigitalSensorData : public AstroSensorData
{
    AstroPinData inputPin;                                  // Input pin

    AstroDigitalSensorData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef AstroSensors_H
