/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Sensors
*/

#ifndef AstroSensors_H
#define AstroSensors_H

#include "AstroMeasurements.h"
#include "AstroObject.h"
#include "AstroPins.h"

// Sensor Base
// Base class for measurements supplied by pins, callbacks, or external sensor adapters.
class AstroSensor : public AstroObject, public AstroSensorObjectInterface, public AstroMeasurementUnitsInterface {
public:
    AstroSensor(Astro_SensorType sensorType = Astro_SensorType_Generic,
                Astro_UnitsType units = Astro_UnitsType_Undefined,
                aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index
    AstroSensor(const AstroObjectData *dataIn, Astro_UnitsType units = Astro_UnitsType_Undefined);
    virtual ~AstroSensor() { ; }

    virtual bool readValue(double *valueOut) = 0;
    virtual bool poll(int64_t timestamp = 0, aframe_t frame = 1) override;

    inline Astro_SensorType getSensorType() const { return _sensorType; }
    inline Astro_UnitsType getUnits() const { return _measurement.units; }
    inline void setUnits(Astro_UnitsType units) { _measurement.units = units; }
    virtual void setMeasurementUnits(Astro_UnitsType units, uint8_t measurementRow = 0) override { if (measurementRow == 0) { setUnits(units); } }
    virtual Astro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const override { return measurementRow == 0 ? getUnits() : Astro_UnitsType_Undefined; }
    virtual const AstroSingleMeasurement &getMeasurement() const override { return _measurement; }

protected:
    Astro_SensorType _sensorType;                            // Sensor type
    AstroSingleMeasurement _measurement;                     // Latest sensor measurement
};

// Value Sensor
// Stores a user supplied scalar measurement.
class AstroValueSensor : public AstroSensor {
public:
    AstroValueSensor(Astro_SensorType sensorType = Astro_SensorType_Generic,
                     Astro_UnitsType units = Astro_UnitsType_Undefined,
                     aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG,
                     double value = 0.0)
        : AstroSensor(sensorType, units, positionIndex), _value(value)
    { ; }

    AstroValueSensor(const AstroObjectData *dataIn, Astro_UnitsType units = Astro_UnitsType_Undefined, double value = 0.0)
        : AstroSensor(dataIn, units), _value(value)
    { ; }

    virtual bool readValue(double *valueOut) override { if (!valueOut) { return false; } *valueOut = _value; return true; }
    inline void setValue(double value) { _value = value; }
    inline double getValue() const { return _value; }

protected:
    double _value;                                           // Stored sensor value
};

// Callback Sensor
// Obtains sensor measurements through a user supplied callback.
class AstroCallbackSensor : public AstroSensor {
public:
    typedef bool (*ReadCallback)(void *context, double *valueOut);

    AstroCallbackSensor(ReadCallback callback = nullptr, void *context = nullptr,
                        Astro_SensorType sensorType = Astro_SensorType_Generic,
                        Astro_UnitsType units = Astro_UnitsType_Undefined,
                        aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG)
        : AstroSensor(sensorType, units, positionIndex), _callback(callback), _context(context)
    { ; }

    virtual bool readValue(double *valueOut) override {
        return _callback ? _callback(_context, valueOut) : false;
    }

protected:
    ReadCallback _callback;                                  // Callback function
    void *_context;                                          // Callback user context
};

// Digital Sensor
// Reads a binary measurement from a digital input pin.
class AstroDigitalSensor : public AstroSensor {
public:
    AstroDigitalSensor(AstroDigitalPin inputPin = AstroDigitalPin(),
                       Astro_SensorType sensorType = Astro_SensorType_Generic,
                       aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index

    virtual bool readValue(double *valueOut) override;
    inline const AstroDigitalPin &getInputPin() const { return _inputPin; }

protected:
    AstroDigitalPin _inputPin;                               // Input pin
};

// Analog Sensor
// Reads a normalized scalar measurement from an analog input pin.
class AstroAnalogSensor : public AstroSensor {
public:
    AstroAnalogSensor(AstroAnalogPin inputPin = AstroAnalogPin(),
                      Astro_SensorType sensorType = Astro_SensorType_Generic,
                      Astro_UnitsType units = Astro_UnitsType_Raw_1,
                      aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index

    virtual bool readValue(double *valueOut) override;
    inline const AstroAnalogPin &getInputPin() const { return _inputPin; }

protected:
    AstroAnalogPin _inputPin;                                // Input pin
};

#endif // /ifndef AstroSensors_H
