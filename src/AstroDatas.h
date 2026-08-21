/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Datas
*/

#ifndef AstroDatas_H
#define AstroDatas_H

#include "AstroData.h"
#include "AstroMeasurements.h"
#include "AstroObject.h"

// Calibration Data
// Stores a simple linear Ax+B calibration transform for sensor or actuator values.
struct AstroCalibrationData {
    char ownerName[ASTRO_NAME_MAXSIZE];                      // Owning object name
    Astro_UnitsType calibrationUnits;                        // Calibration output units
    double multiplier;                                      // Linear calibration multiplier
    double offset;                                          // Linear calibration offset

    AstroCalibrationData();
    AstroCalibrationData(AstroIdentity ownerId, Astro_UnitsType calibrationUnitsIn = Astro_UnitsType_Undefined);

    // Transforms value from raw (or initial) value into calibrated (or transformed) value.
    inline double transform(double value) const { return (value * multiplier) + offset; }
    // Transforms value in-place from raw (or initial) value into calibrated (or transformed) value, with optional units write out.
    inline void transform(double *valueInOut, Astro_UnitsType *unitsOut = nullptr) const { *valueInOut = transform(*valueInOut);
                                                                                          if (unitsOut) { *unitsOut = calibrationUnits; } }
    // Transforms measurement from raw (or initial) measurement into calibrated (or transformed) measurement.
    inline AstroSingleMeasurement transform(AstroSingleMeasurement measurement) const { return AstroSingleMeasurement(transform(measurement.value), calibrationUnits, measurement.timestamp, measurement.frame); }
    // Transforms measurement in-place from raw (or initial) measurement into calibrated (or transformed) measurement.
    inline void transform(AstroSingleMeasurement *measurementInOut) const { transform(&measurementInOut->value, &measurementInOut->units); }

    // Inverse transforms value from calibrated (or transformed) value back into raw (or initial) value.
    inline double inverseTransform(double value) const { return (value - offset) / multiplier; }
    // Inverse transforms value in-place from calibrated (or transformed) value back into raw (or initial) value, with optional units write out.
    inline void inverseTransform(double *valueInOut, Astro_UnitsType *unitsOut = nullptr) const { *valueInOut = inverseTransform(*valueInOut);
                                                                                                 if (unitsOut) { *unitsOut = Astro_UnitsType_Raw_1; } }
    // Inverse transforms measurement from calibrated (or transformed) measurement back into raw (or initial) measurement.
    inline AstroSingleMeasurement inverseTransform(AstroSingleMeasurement measurement) const { return AstroSingleMeasurement(inverseTransform(measurement.value), Astro_UnitsType_Raw_1, measurement.timestamp, measurement.frame); }
    // Inverse transforms measurement in-place from calibrated (or transformed) measurement back into raw (or initial) measurement.
    inline void inverseTransform(AstroSingleMeasurement *measurementInOut) const { inverseTransform(&measurementInOut->value, &measurementInOut->units); }

    // Sets linear calibration curvature from two points.
    void setFromTwoPoints(double point1RawMeasuredAt,
                          double point1CalibratedTo,
                          double point2RawMeasuredAt,
                          double point2CalibratedTo);

    // Sets linear calibration curvature from two voltages.
    inline void setFromTwoVoltages(double point1VoltsAt,
                                   double point1CalibTo,
                                   double point2VoltsAt,
                                   double point2CalibTo,
                                   double analogRefVolts) {
        setFromTwoPoints(point1VoltsAt / analogRefVolts, point1CalibTo,
                         point2VoltsAt / analogRefVolts, point2CalibTo);
    }

    // Sets linear calibration curvature from known output range.
    inline void setFromRange(double minValue, double maxValue) { setFromTwoPoints(0.0, minValue, 1.0, maxValue); }
    // Sets linear calibration curvature from known output scale.
    inline void setFromScale(double scale) { setFromRange(0.0, scale); }
    // Sets linear calibration curvature from typical servo ranges.
    inline void setFromServo(double minDegrees, double maxDegrees) { setFromTwoPoints(0.025, minDegrees, 0.125, maxDegrees); }

    bool toJSON(char *bufferOut, size_t bufferSize) const;
    bool fromJSON(const char *jsonIn);
};

#endif // /ifndef AstroDatas_H
