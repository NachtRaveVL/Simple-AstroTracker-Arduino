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
    double multiplier;                                       // Linear calibration multiplier
    double offset;                                           // Linear calibration offset

    AstroCalibrationData();
    AstroCalibrationData(AstroIdentity ownerId, Astro_UnitsType calibrationUnitsIn = Astro_UnitsType_Undefined);

    double transform(double value) const;
    double inverseTransform(double value) const;
    void transform(AstroSingleMeasurement *measurementInOut) const;
    void inverseTransform(AstroSingleMeasurement *measurementInOut) const;

    void setFromTwoPoints(double point1RawMeasuredAt, double point1CalibratedTo,
                          double point2RawMeasuredAt, double point2CalibratedTo);
    inline void setFromRange(double minValue, double maxValue) { setFromTwoPoints(0.0, minValue, 1.0, maxValue); }
    inline void setFromScale(double scale) { setFromRange(0.0, scale); }
    inline void setFromServo(double minDegrees, double maxDegrees) { setFromTwoPoints(0.025, minDegrees, 0.125, maxDegrees); }

    bool toJSON(char *bufferOut, size_t bufferSize) const;
    bool fromJSON(const char *jsonIn);
};

#endif // /ifndef AstroDatas_H
