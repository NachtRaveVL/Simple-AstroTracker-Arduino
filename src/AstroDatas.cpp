/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Datas
*/

#include "Astruino.h"
#include <stdio.h>

AstroCalibrationData::AstroCalibrationData()
    : ownerName{0}, calibrationUnits(Astro_UnitsType_Undefined), multiplier(1.0), offset(0.0)
{ ; }

AstroCalibrationData::AstroCalibrationData(AstroIdentity ownerId, Astro_UnitsType calibrationUnitsIn)
    : ownerName{0}, calibrationUnits(calibrationUnitsIn), multiplier(1.0), offset(0.0)
{
    snprintf(ownerName, sizeof(ownerName), "%s", ownerId.keyString.c_str());
}

double AstroCalibrationData::transform(double value) const
{
    return value * multiplier + offset;
}

double AstroCalibrationData::inverseTransform(double value) const
{
    return !isFPEqual(multiplier, 0.0) ? (value - offset) / multiplier : value;
}

void AstroCalibrationData::transform(AstroSingleMeasurement *measurementInOut) const
{
    if (!measurementInOut) { return; }
    measurementInOut->value = transform(measurementInOut->value);
    measurementInOut->units = calibrationUnits;
}

void AstroCalibrationData::inverseTransform(AstroSingleMeasurement *measurementInOut) const
{
    if (!measurementInOut) { return; }
    measurementInOut->value = inverseTransform(measurementInOut->value);
    measurementInOut->units = Astro_UnitsType_Raw_1;
}

void AstroCalibrationData::setFromTwoPoints(double point1RawMeasuredAt, double point1CalibratedTo,
                                            double point2RawMeasuredAt, double point2CalibratedTo)
{
    double rawDelta = point2RawMeasuredAt - point1RawMeasuredAt;
    if (isFPEqual(rawDelta, 0.0)) {
        multiplier = 1.0;
        offset = 0.0;
        return;
    }

    multiplier = (point2CalibratedTo - point1CalibratedTo) / rawDelta;
    offset = point1CalibratedTo - multiplier * point1RawMeasuredAt;
}

bool AstroCalibrationData::toJSON(char *bufferOut, size_t bufferSize) const
{
    if (!bufferOut || !bufferSize) { return false; }
    int written = snprintf(bufferOut, bufferSize,
        "{\"type\":\"ACAL\",\"owner\":\"%s\",\"units\":%d,\"multiplier\":%.12g,\"offset\":%.12g}",
        ownerName, (int)calibrationUnits, multiplier, offset);
    return written >= 0 && (size_t)written < bufferSize;
}

bool AstroCalibrationData::fromJSON(const char *jsonIn)
{
    if (!jsonIn) { return false; }

    char typeIn[8] = {0};
    char ownerIn[ASTRO_NAME_MAXSIZE] = {0};
    long unitsIn;
    double multiplierIn, offsetIn;
    if (!astroJSONGetString(jsonIn, "type", typeIn, sizeof(typeIn)) || strcmp(typeIn, "ACAL") != 0 ||
        !astroJSONGetString(jsonIn, "owner", ownerIn, sizeof(ownerIn)) ||
        !astroJSONGetLong(jsonIn, "units", &unitsIn) ||
        !astroJSONGetDouble(jsonIn, "multiplier", &multiplierIn) ||
        !astroJSONGetDouble(jsonIn, "offset", &offsetIn)) {
        return false;
    }

    snprintf(ownerName, sizeof(ownerName), "%s", ownerIn);
    calibrationUnits = (Astro_UnitsType)unitsIn;
    multiplier = multiplierIn;
    offset = offsetIn;
    return true;
}
