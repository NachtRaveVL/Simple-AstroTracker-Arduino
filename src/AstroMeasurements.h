/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Sensor Measurements
*/

#ifndef AstroMeasurements_H
#define AstroMeasurements_H

struct AstroMeasurement;
struct AstroSingleMeasurement;
struct AstroBinaryMeasurement;
struct AstroDoubleMeasurement;
struct AstroTripleMeasurement;

struct AstroMeasurementData;

#include "Astroduino.h"
#include "AstroData.h"

// Creates measurement object from passed trigger sub data (return ownership transfer - user code *must* delete returned object)
extern AstroMeasurement *newMeasurementObjectFromSubData(const AstroMeasurementData *dataIn);

// Gets the value of a measurement at a specified row (with optional binary true scaling value).
extern float getMeasurementValue(const AstroMeasurement *measurement, uint8_t measurementRow = 0, float binScale = 1.0f);
// Gets the units of a measurement at a specified row (with optional binary units).
extern Astro_UnitsType getMeasurementUnits(const AstroMeasurement *measurement, uint8_t measurementRow = 0, Astro_UnitsType binUnits = Astro_UnitsType_Raw_1);
// Gets the number of rows of data that a measurement holds.
extern uint8_t getMeasurementRowCount(const AstroMeasurement *measurement);
// Gets the single measurement of a measurement (with optional binary true scaling value / units).
extern AstroSingleMeasurement getAsSingleMeasurement(const AstroMeasurement *measurement, uint8_t measurementRow = 0, float binScale = 1.0f, Astro_UnitsType binUnits = Astro_UnitsType_Raw_1);

// Sensor Data Measurement Base
struct AstroMeasurement {
    enum : signed char { Binary, Single, Double, Triple, Unknown = -1 } type; // Measurement type (custom RTTI)
    inline bool isBinaryType() const { return type == Binary; }
    inline bool isSingleType() const { return type == Single; }
    inline bool isDoubleType() const { return type == Double; }
    inline bool isTripleType() const { return type == Triple; }
    inline bool isUnknownType() const { return type <= Unknown; }

    time_t timestamp;                                       // Time event recorded (UTC)
    aframe_t frame;                                         // Polling frame # measurement taken on, or 0 if not-set else 1 if user-set

    inline AstroMeasurement() : type(Unknown), timestamp(unixNow()), frame(0) { ; }
    inline AstroMeasurement(int classType, time_t timestampIn, aframe_t frameIn) : type((typeof(type))classType), timestamp(timestampIn), frame(frameIn) { ; }
    AstroMeasurement(int classType, time_t timestamp = 0);
    AstroMeasurement(const AstroMeasurementData *dataIn);

    void saveToData(AstroMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;

    inline void updateTimestamp() { timestamp = unixNow(); }
    void updateFrame(aframe_t minFrame = 0);
    inline void setMinFrame(aframe_t minFrame = 0) { frame = max(minFrame, frame); }
    inline bool isSet() const { return frame != aframe_none; }
};

// Single Value Sensor Data Measurement
struct AstroSingleMeasurement : public AstroMeasurement {
    float value;                                            // Polled value
    Astro_UnitsType units;                                  // Units of value

    AstroSingleMeasurement();
    AstroSingleMeasurement(float value, Astro_UnitsType units, time_t timestamp = unixNow());
    AstroSingleMeasurement(float value, Astro_UnitsType units, time_t timestamp, aframe_t frame);
    AstroSingleMeasurement(const AstroMeasurementData *dataIn);

    void saveToData(AstroMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;

    // Modifiers (in utils)

    inline AstroSingleMeasurement &toUnits(Astro_UnitsType outUnits, float convertParam = FLT_UNDEF);

    // Copiers (in utils)

    inline AstroSingleMeasurement asUnits(Astro_UnitsType outUnits, float convertParam = FLT_UNDEF) const;
};

// Binary Value Sensor Data Measurement
struct AstroBinaryMeasurement : public AstroMeasurement {
    bool state;                                             // Polled state

    AstroBinaryMeasurement();
    AstroBinaryMeasurement(bool state, time_t timestamp = unixNow());
    AstroBinaryMeasurement(bool state, time_t timestamp, aframe_t frame);
    AstroBinaryMeasurement(const AstroMeasurementData *dataIn);

    void saveToData(AstroMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;

    inline AstroSingleMeasurement getAsSingleMeasurement(float binScale = 1.0f, Astro_UnitsType binUnits = Astro_UnitsType_Raw_1) { return AstroSingleMeasurement(state ? binScale : 0.0f, binUnits, timestamp, frame); }
};

// Double Value Sensor Data Measurement
struct AstroDoubleMeasurement : public AstroMeasurement {
    float value[2];                                         // Polled values
    Astro_UnitsType units[2];                               // Units of values

    AstroDoubleMeasurement();
    AstroDoubleMeasurement(float value1, Astro_UnitsType units1, 
                           float value2, Astro_UnitsType units2, 
                           time_t timestamp = unixNow());
    AstroDoubleMeasurement(float value1, Astro_UnitsType units1, 
                           float value2, Astro_UnitsType units2, 
                           time_t timestamp, aframe_t frame);
    AstroDoubleMeasurement(const AstroMeasurementData *dataIn);

    void saveToData(AstroMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;

    inline AstroSingleMeasurement getAsSingleMeasurement(uint8_t measurementRow) { return AstroSingleMeasurement(value[measurementRow], units[measurementRow], timestamp, frame); }
};

// Triple Value Sensor Data Measurement
struct AstroTripleMeasurement : public AstroMeasurement {
    float value[3];                                         // Polled values
    Astro_UnitsType units[3];                               // Units of values

    AstroTripleMeasurement();
    AstroTripleMeasurement(float value1, Astro_UnitsType units1, 
                           float value2, Astro_UnitsType units2, 
                           float value3, Astro_UnitsType units3,
                           time_t timestamp = unixNow());
    AstroTripleMeasurement(float value1, Astro_UnitsType units1, 
                           float value2, Astro_UnitsType units2, 
                           float value3, Astro_UnitsType units3,
                           time_t timestamp, aframe_t frame);
    AstroTripleMeasurement(const AstroMeasurementData *dataIn);

    void saveToData(AstroMeasurementData *dataOut, uint8_t measurementRow = 0, unsigned int additionalDecPlaces = 0) const;

    inline AstroSingleMeasurement getAsSingleMeasurement(uint8_t measurementRow) { return AstroSingleMeasurement(value[measurementRow], units[measurementRow], timestamp, frame); }
    inline AstroDoubleMeasurement getAsDoubleMeasurement(uint8_t measurementRow1, uint8_t measurementRow2) { return AstroDoubleMeasurement(value[measurementRow1], units[measurementRow1], value[measurementRow2], units[measurementRow2], timestamp, frame); }
};


// Combined Measurement Serialization Sub Data
struct AstroMeasurementData : public AstroSubData {
    uint8_t measurementRow;                                 // Source measurement row index that data is from
    float value;                                            // Value
    Astro_UnitsType units;                                  // Units of value
    time_t timestamp;                                       // Timestamp

    AstroMeasurementData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
    void fromJSONVariant(JsonVariantConst &variantIn);
};

#endif // /ifndef AstroMeasurements_H
