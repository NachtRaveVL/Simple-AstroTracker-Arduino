/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Sensor Measurements
*/

#include "AstroMeasurements.h"
#include "AstroUtils.h"
#include <math.h>
#include <stdio.h>

void AstroMeasurement::updateFrame(aframe_t minFrame)
{
    if (frame < minFrame) { frame = minFrame; }
    else if (++frame == aframe_none) { frame = 1; }
}

AstroSingleMeasurement &AstroSingleMeasurement::toUnits(Astro_UnitsType outUnits, double convertParam)
{
    double converted;
    if (astroConvertUnits(value, units, outUnits, &converted, convertParam)) {
        value = converted;
        units = outUnits;
    }
    return *this;
}

AstroSingleMeasurement AstroSingleMeasurement::asUnits(Astro_UnitsType outUnits, double convertParam) const
{
    AstroSingleMeasurement copy(*this);
    copy.toUnits(outUnits, convertParam);
    return copy;
}

AstroSingleMeasurement &AstroSingleMeasurement::wrapBy(double range)
{
    if (range > 0.0) {
        value = fmod(value, range);
        if (value < 0.0) { value += range; }
    }
    return *this;
}

AstroSingleMeasurement &AstroSingleMeasurement::wrapBySplit(double range)
{
    wrapBy(range);
    if (range > 0.0 && value > range * 0.5) { value -= range; }
    return *this;
}

AstroSingleMeasurement AstroSingleMeasurement::wrappedBy(double range) const
{
    AstroSingleMeasurement copy(*this);
    copy.wrapBy(range);
    return copy;
}

AstroSingleMeasurement AstroSingleMeasurement::wrappedBySplit(double range) const
{
    AstroSingleMeasurement copy(*this);
    copy.wrapBySplit(range);
    return copy;
}

AstroSingleMeasurement AstroBinaryMeasurement::getAsSingleMeasurement(double trueScale, Astro_UnitsType unitsIn) const
{
    return AstroSingleMeasurement(state ? trueScale : 0.0, unitsIn, timestamp, frame);
}

AstroDoubleMeasurement::AstroDoubleMeasurement(double value1, Astro_UnitsType units1,
                                               double value2, Astro_UnitsType units2,
                                               int64_t timestampIn, aframe_t frameIn)
    : AstroMeasurement(Double, timestampIn, frameIn), value{value1, value2}, units{units1, units2}
{ ; }

AstroSingleMeasurement AstroDoubleMeasurement::getAsSingleMeasurement(uint8_t row) const
{
    row = row > 1 ? 1 : row;
    return AstroSingleMeasurement(value[row], units[row], timestamp, frame);
}

AstroTripleMeasurement::AstroTripleMeasurement(double value1, Astro_UnitsType units1,
                                               double value2, Astro_UnitsType units2,
                                               double value3, Astro_UnitsType units3,
                                               int64_t timestampIn, aframe_t frameIn)
    : AstroMeasurement(Triple, timestampIn, frameIn), value{value1, value2, value3},
      units{units1, units2, units3}
{ ; }

AstroSingleMeasurement AstroTripleMeasurement::getAsSingleMeasurement(uint8_t row) const
{
    row = row > 2 ? 2 : row;
    return AstroSingleMeasurement(value[row], units[row], timestamp, frame);
}

AstroDoubleMeasurement AstroTripleMeasurement::getAsDoubleMeasurement(uint8_t row1, uint8_t row2) const
{
    row1 = row1 > 2 ? 2 : row1;
    row2 = row2 > 2 ? 2 : row2;
    return AstroDoubleMeasurement(value[row1], units[row1], value[row2], units[row2], timestamp, frame);
}

double getMeasurementValue(const AstroMeasurement *measurement, uint8_t row, double trueScale)
{
    if (!measurement) { return 0.0; }
    switch (measurement->type) {
        case AstroMeasurement::Binary:
            return ((const AstroBinaryMeasurement *)measurement)->state ? trueScale : 0.0;
        case AstroMeasurement::Single:
            return ((const AstroSingleMeasurement *)measurement)->value;
        case AstroMeasurement::Double:
            return ((const AstroDoubleMeasurement *)measurement)->value[row > 1 ? 1 : row];
        case AstroMeasurement::Triple:
            return ((const AstroTripleMeasurement *)measurement)->value[row > 2 ? 2 : row];
        default:
            return 0.0;
    }
}

Astro_UnitsType getMeasurementUnits(const AstroMeasurement *measurement, uint8_t row, Astro_UnitsType binaryUnits)
{
    if (!measurement) { return Astro_UnitsType_Undefined; }
    switch (measurement->type) {
        case AstroMeasurement::Binary:
            return binaryUnits;
        case AstroMeasurement::Single:
            return ((const AstroSingleMeasurement *)measurement)->units;
        case AstroMeasurement::Double:
            return ((const AstroDoubleMeasurement *)measurement)->units[row > 1 ? 1 : row];
        case AstroMeasurement::Triple:
            return ((const AstroTripleMeasurement *)measurement)->units[row > 2 ? 2 : row];
        default:
            return Astro_UnitsType_Undefined;
    }
}

uint8_t getMeasurementRowCount(const AstroMeasurement *measurement)
{
    if (!measurement) { return 0; }
    switch (measurement->type) {
        case AstroMeasurement::Binary:
        case AstroMeasurement::Single: return 1;
        case AstroMeasurement::Double: return 2;
        case AstroMeasurement::Triple: return 3;
        default: return 0;
    }
}

AstroSingleMeasurement getAsSingleMeasurement(const AstroMeasurement *measurement, uint8_t row,
                                              double trueScale, Astro_UnitsType binaryUnits)
{
    if (!measurement) { return AstroSingleMeasurement(); }
    return AstroSingleMeasurement(getMeasurementValue(measurement, row, trueScale),
                                  getMeasurementUnits(measurement, row, binaryUnits),
                                  measurement->timestamp, measurement->frame);
}

AstroMeasurementData::AstroMeasurementData()
    : measurementRow(0), value(0.0), units(Astro_UnitsType_Undefined), timestamp(0), frame(aframe_none)
{ ; }

bool AstroMeasurementData::toJSON(char *bufferOut, size_t bufferSize) const
{
    if (!bufferOut || !bufferSize) { return false; }
    int written = snprintf(bufferOut, bufferSize,
        "{\"type\":\"AMSR\",\"row\":%u,\"value\":%.8f,\"units\":%d,\"timestamp\":%lld,\"frame\":%lu}",
        (unsigned int)measurementRow, value, (int)units, (long long)timestamp, (unsigned long)frame);
    return written >= 0 && (size_t)written < bufferSize;
}

bool AstroMeasurementData::fromJSON(const char *jsonIn)
{
    if (!jsonIn) { return false; }

    char typeIn[8] = {0};
    unsigned long rowIn, frameIn;
    long unitsIn, timestampIn;
    double valueIn;
    if (!astroJSONGetString(jsonIn, "type", typeIn, sizeof(typeIn)) || strcmp(typeIn, "AMSR") != 0 ||
        !astroJSONGetUnsignedLong(jsonIn, "row", &rowIn) ||
        !astroJSONGetDouble(jsonIn, "value", &valueIn) ||
        !astroJSONGetLong(jsonIn, "units", &unitsIn) ||
        !astroJSONGetLong(jsonIn, "timestamp", &timestampIn) ||
        !astroJSONGetUnsignedLong(jsonIn, "frame", &frameIn)) {
        return false;
    }

    measurementRow = (uint8_t)rowIn;
    value = valueIn;
    units = (Astro_UnitsType)unitsIn;
    timestamp = (int64_t)timestampIn;
    frame = (aframe_t)frameIn;
    return true;
}
