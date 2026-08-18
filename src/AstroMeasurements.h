/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Sensor Measurements
*/

#ifndef AstroMeasurements_H
#define AstroMeasurements_H

#include "AstroDefines.h"

// Sensor Measurement Base
// Common timestamp and polling-frame metadata for sensor measurements.
struct AstroMeasurement {
    enum : signed char { Binary, Single, Double, Triple, Unknown = -1 } type;

    int64_t timestamp;                                       // Measurement/event timestamp
    aframe_t frame;                                          // Polling frame number

    AstroMeasurement(int classType = Unknown, int64_t timestampIn = 0, aframe_t frameIn = aframe_none)
        : type((decltype(type))classType), timestamp(timestampIn), frame(frameIn)
    { ; }

    inline bool isBinaryType() const { return type == Binary; }
    inline bool isSingleType() const { return type == Single; }
    inline bool isDoubleType() const { return type == Double; }
    inline bool isTripleType() const { return type == Triple; }
    inline bool isUnknownType() const { return type <= Unknown; }
    inline bool isSet() const { return frame != aframe_none; }

    void updateFrame(aframe_t minFrame = 1);
};

// Single Value Measurement
// One measured value with associated units.
struct AstroSingleMeasurement : public AstroMeasurement {
    double value;                                            // Measured value
    Astro_UnitsType units;                                   // Measurement units

    AstroSingleMeasurement(double valueIn = 0.0,
                           Astro_UnitsType unitsIn = Astro_UnitsType_Undefined,
                           int64_t timestampIn = 0,
                           aframe_t frameIn = aframe_none)
        : AstroMeasurement(Single, timestampIn, frameIn), value(valueIn), units(unitsIn)
    { ; }

    AstroSingleMeasurement &toUnits(Astro_UnitsType outUnits, double convertParam = 0.0);
    AstroSingleMeasurement asUnits(Astro_UnitsType outUnits, double convertParam = 0.0) const;

    AstroSingleMeasurement &wrapBy(double range);
    AstroSingleMeasurement &wrapBySplit(double range);
    AstroSingleMeasurement wrappedBy(double range) const;
    AstroSingleMeasurement wrappedBySplit(double range) const;
};

// Binary Measurement
// Boolean sensor state with common measurement metadata.
struct AstroBinaryMeasurement : public AstroMeasurement {
    bool state;                                              // Binary state

    AstroBinaryMeasurement(bool stateIn = false, int64_t timestampIn = 0, aframe_t frameIn = aframe_none)
        : AstroMeasurement(Binary, timestampIn, frameIn), state(stateIn)
    { ; }

    AstroSingleMeasurement getAsSingleMeasurement(double trueScale = 1.0,
                                                  Astro_UnitsType unitsIn = Astro_UnitsType_Raw_1) const; // Units in
};

// Double Value Measurement
// Two related measured values with independent units.
struct AstroDoubleMeasurement : public AstroMeasurement {
    double value[2];                                         // Measured value
    Astro_UnitsType units[2];                                // Measurement units

    AstroDoubleMeasurement(double value1 = 0.0, Astro_UnitsType units1 = Astro_UnitsType_Undefined,
                           double value2 = 0.0, Astro_UnitsType units2 = Astro_UnitsType_Undefined,
                           int64_t timestampIn = 0, aframe_t frameIn = aframe_none); // Timestamp in

    AstroSingleMeasurement getAsSingleMeasurement(uint8_t row) const;
};

// Triple Value Measurement
// Three related measured values with independent units.
struct AstroTripleMeasurement : public AstroMeasurement {
    double value[3];                                         // Measured value
    Astro_UnitsType units[3];                                // Measurement units

    AstroTripleMeasurement(double value1 = 0.0, Astro_UnitsType units1 = Astro_UnitsType_Undefined,
                           double value2 = 0.0, Astro_UnitsType units2 = Astro_UnitsType_Undefined,
                           double value3 = 0.0, Astro_UnitsType units3 = Astro_UnitsType_Undefined,
                           int64_t timestampIn = 0, aframe_t frameIn = aframe_none); // Timestamp in

    AstroSingleMeasurement getAsSingleMeasurement(uint8_t row) const;
    AstroDoubleMeasurement getAsDoubleMeasurement(uint8_t row1, uint8_t row2) const;
};

extern double getMeasurementValue(const AstroMeasurement *measurement, uint8_t row = 0, double trueScale = 1.0);
extern Astro_UnitsType getMeasurementUnits(const AstroMeasurement *measurement, uint8_t row = 0,
                                           Astro_UnitsType binaryUnits = Astro_UnitsType_Raw_1);
extern uint8_t getMeasurementRowCount(const AstroMeasurement *measurement);
extern AstroSingleMeasurement getAsSingleMeasurement(const AstroMeasurement *measurement, uint8_t row = 0,
                                                      double trueScale = 1.0,
                                                      Astro_UnitsType binaryUnits = Astro_UnitsType_Raw_1);

// Measurement Serialization Data
// Serializable representation of one sensor measurement row.
struct AstroMeasurementData {
    uint8_t measurementRow;                                  // Measurement row
    double value;                                            // Measured value
    Astro_UnitsType units;                                   // Measurement units
    int64_t timestamp;                                       // Measurement/event timestamp
    aframe_t frame;                                          // Polling frame number

    AstroMeasurementData();
    bool toJSON(char *bufferOut, size_t bufferSize) const;
    bool fromJSON(const char *jsonIn);
};

#endif // /ifndef AstroMeasurements_H
