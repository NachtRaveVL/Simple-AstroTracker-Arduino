/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Sensor Measurements
*/

#include "AstroMeasurements.h"
#include "AstroUtils.h"
#include <math.h>
#include <stdio.h>

AstroMeasurement *newMeasurementObjectFromSubData(const AstroMeasurementData *dataIn)
{
    if (!dataIn || !isValidType(dataIn->type)) return nullptr;
    Astro_SOFT_ASSERT(dataIn && isValidType(dataIn->type), SFP(AStr_Err_InvalidParameter));

    if (dataIn) {
        switch (dataIn->type) {
            case (aid_t)AstroMeasurement::Binary:
                return new AstroBinaryMeasurement(dataIn);
            case (aid_t)AstroMeasurement::Single:
                return new AstroSingleMeasurement(dataIn);
            case (aid_t)AstroMeasurement::Double:
                return new AstroDoubleMeasurement(dataIn);
            case (aid_t)AstroMeasurement::Triple:
                return new AstroTripleMeasurement(dataIn);
            default: break;
        }
    }

    return nullptr;
}

float getMeasurementValue(const AstroMeasurement *measurement, uint8_t measurementRow, float binScale)
{
    if (measurement) {
        switch (measurement->type) {
            case AstroMeasurement::Binary:
                return ((AstroBinaryMeasurement *)measurement)->state ? binScale : 0.0f;
            case AstroMeasurement::Single:
                return ((AstroSingleMeasurement *)measurement)->value;
            case AstroMeasurement::Double:
                return ((AstroDoubleMeasurement *)measurement)->value[measurementRow];
            case AstroMeasurement::Triple:
                return ((AstroTripleMeasurement *)measurement)->value[measurementRow];
            default: break;
        }
    }
    return 0.0f;
}

Astro_UnitsType getMeasurementUnits(const AstroMeasurement *measurement, uint8_t measurementRow, Astro_UnitsType binUnits)
{
    if (measurement) {
        switch (measurement->type) {
            case AstroMeasurement::Binary:
                return binUnits;
            case AstroMeasurement::Single:
                return ((AstroSingleMeasurement *)measurement)->units;
            case AstroMeasurement::Double:
                return ((AstroDoubleMeasurement *)measurement)->units[measurementRow];
            case AstroMeasurement::Triple:
                return ((AstroTripleMeasurement *)measurement)->units[measurementRow];
            default: break;
        }
    }
    return Astro_UnitsType_Undefined;
}

uint8_t getMeasurementRowCount(const AstroMeasurement *measurement)
{
    return measurement ? max(1, (int)(measurement->type)) : 0;
}

AstroSingleMeasurement getAsSingleMeasurement(const AstroMeasurement *measurement, uint8_t measurementRow, float binScale, Astro_UnitsType binUnits)
{
    if (measurement) {
        switch (measurement->type) {
            case AstroMeasurement::Binary:
                return ((AstroBinaryMeasurement *)measurement)->getAsSingleMeasurement(binScale, binUnits);
            case AstroMeasurement::Single:
                return *((const AstroSingleMeasurement *)measurement);
            case AstroMeasurement::Double:
                return ((AstroDoubleMeasurement *)measurement)->getAsSingleMeasurement(measurementRow);
            case AstroMeasurement::Triple:
                return ((AstroTripleMeasurement *)measurement)->getAsSingleMeasurement(measurementRow);
            default: break;
        }
    }
    AstroSingleMeasurement retVal;
    retVal.frame = aframe_none; // meant to fail frame checks
    return retVal;
}


AstroMeasurement::AstroMeasurement(int classType, time_t timestampIn)
    : type((typeof(type))classType), timestamp(timestampIn)
{
    updateFrame();
}

AstroMeasurement::AstroMeasurement(const AstroMeasurementData *dataIn)
    : type((typeof(type))(dataIn->type)), timestamp(dataIn->timestamp)
{
    updateFrame(1);
}

void AstroMeasurement::saveToData(AstroMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    dataOut->type = (int8_t)type;
    dataOut->measurementRow = measurementRow;
    dataOut->timestamp = timestamp;
}

void AstroMeasurement::updateFrame(aframe_t minFrame)
{
    frame = max(minFrame, getController() ? getController()->getPollingFrame() : 0);
}


AstroBinaryMeasurement::AstroBinaryMeasurement()
    : AstroMeasurement(), state(false)
{ ; }

AstroBinaryMeasurement::AstroBinaryMeasurement(bool stateIn, time_t timestamp)
    : AstroMeasurement((int)Binary, timestamp), state(stateIn)
{ ; }

AstroBinaryMeasurement::AstroBinaryMeasurement(bool stateIn, time_t timestamp, aframe_t frame)
    : AstroMeasurement((int)Binary, timestamp, frame), state(stateIn)
{ ; }

AstroBinaryMeasurement::AstroBinaryMeasurement(const AstroMeasurementData *dataIn)
    : AstroMeasurement(dataIn),
      state(dataIn->measurementRow == 0 && dataIn->value >= 0.5f - FLT_EPSILON)
{ ; }

void AstroBinaryMeasurement::saveToData(AstroMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    AstroMeasurement::saveToData(dataOut, measurementRow, additionalDecPlaces);

    dataOut->value = measurementRow == 0 && state ? 1.0f : 0.0f;
    dataOut->units = measurementRow == 0 ? Astro_UnitsType_Raw_1 : Astro_UnitsType_Undefined;
}


AstroSingleMeasurement::AstroSingleMeasurement()
    : AstroMeasurement((int)Single), value(0.0f), units(Astro_UnitsType_Undefined)
{ ; }

AstroSingleMeasurement::AstroSingleMeasurement(float valueIn, Astro_UnitsType unitsIn, time_t timestamp)
    : AstroMeasurement((int)Single, timestamp), value(valueIn), units(unitsIn)
{ ; }

AstroSingleMeasurement::AstroSingleMeasurement(float valueIn, Astro_UnitsType unitsIn, time_t timestamp, aframe_t frame)
    : AstroMeasurement((int)Single, timestamp, frame), value(valueIn), units(unitsIn)
{ ; }

AstroSingleMeasurement::AstroSingleMeasurement(const AstroMeasurementData *dataIn)
    : AstroMeasurement(dataIn),
      value(dataIn->measurementRow == 0 ? dataIn->value : 0.0f),
      units(dataIn->measurementRow == 0 ? dataIn->units : Astro_UnitsType_Undefined)
{ ; }

void AstroSingleMeasurement::saveToData(AstroMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    AstroMeasurement::saveToData(dataOut, measurementRow, additionalDecPlaces);

    dataOut->value = measurementRow == 0 ? roundForExport(value, additionalDecPlaces) : 0.0f;
    dataOut->units = measurementRow == 0 ? units : Astro_UnitsType_Undefined;
}


AstroDoubleMeasurement::AstroDoubleMeasurement()
    : AstroMeasurement((int)Double), value{0}, units{Astro_UnitsType_Undefined,Astro_UnitsType_Undefined}
{ ; }

AstroDoubleMeasurement::AstroDoubleMeasurement(float value1, Astro_UnitsType units1,
                                               float value2, Astro_UnitsType units2,
                                               time_t timestamp)
    : AstroMeasurement((int)Double, timestamp), value{value1,value2}, units{units1,units2}
{ ; }

AstroDoubleMeasurement::AstroDoubleMeasurement(float value1, Astro_UnitsType units1,
                                               float value2, Astro_UnitsType units2,
                                               time_t timestamp, aframe_t frame)
    : AstroMeasurement((int)Double, timestamp, frame), value{value1,value2}, units{units1,units2}
{ ; }

AstroDoubleMeasurement::AstroDoubleMeasurement(const AstroMeasurementData *dataIn)
    : AstroMeasurement(dataIn),
      value{dataIn->measurementRow == 0 ? dataIn->value : 0.0f,
            dataIn->measurementRow == 1 ? dataIn->value : 0.0f
      },
      units{dataIn->measurementRow == 0 ? dataIn->units : Astro_UnitsType_Undefined,
            dataIn->measurementRow == 1 ? dataIn->units : Astro_UnitsType_Undefined
      }
{ ; }

void AstroDoubleMeasurement::saveToData(AstroMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    AstroMeasurement::saveToData(dataOut, measurementRow, additionalDecPlaces);

    dataOut->value = measurementRow >= 0 && measurementRow < 2 ? roundForExport(value[measurementRow], additionalDecPlaces) : 0.0f;
    dataOut->units = measurementRow >= 0 && measurementRow < 2 ? units[measurementRow] : Astro_UnitsType_Undefined;
}


AstroTripleMeasurement::AstroTripleMeasurement()
    : AstroMeasurement((int)Triple), value{0}, units{Astro_UnitsType_Undefined,Astro_UnitsType_Undefined,Astro_UnitsType_Undefined}
{ ; }

AstroTripleMeasurement::AstroTripleMeasurement(float value1, Astro_UnitsType units1,
                                               float value2, Astro_UnitsType units2,
                                               float value3, Astro_UnitsType units3,
                                               time_t timestamp)
    : AstroMeasurement((int)Triple, timestamp), value{value1,value2,value3}, units{units1,units2,units3}
{ ; }

AstroTripleMeasurement::AstroTripleMeasurement(float value1, Astro_UnitsType units1,
                                               float value2, Astro_UnitsType units2,
                                               float value3, Astro_UnitsType units3,
                                               time_t timestamp, aframe_t frame)
    : AstroMeasurement((int)Triple, timestamp, frame), value{value1,value2,value3}, units{units1,units2,units3}
{ ; }

AstroTripleMeasurement::AstroTripleMeasurement(const AstroMeasurementData *dataIn)
    : AstroMeasurement(dataIn),
      value{dataIn->measurementRow == 0 ? dataIn->value : 0.0f,
            dataIn->measurementRow == 1 ? dataIn->value : 0.0f,
            dataIn->measurementRow == 2 ? dataIn->value : 0.0f,
      },
      units{dataIn->measurementRow == 0 ? dataIn->units : Astro_UnitsType_Undefined,
            dataIn->measurementRow == 1 ? dataIn->units : Astro_UnitsType_Undefined,
            dataIn->measurementRow == 2 ? dataIn->units : Astro_UnitsType_Undefined,
      }
{ ; }

void AstroTripleMeasurement::saveToData(AstroMeasurementData *dataOut, uint8_t measurementRow, unsigned int additionalDecPlaces) const
{
    AstroMeasurement::saveToData(dataOut, measurementRow, additionalDecPlaces);

    dataOut->value = measurementRow >= 0 && measurementRow < 3 ? roundForExport(value[measurementRow], additionalDecPlaces) : 0.0f;
    dataOut->units = measurementRow >= 0 && measurementRow < 3 ? units[measurementRow] : Astro_UnitsType_Undefined;
}


AstroMeasurementData::AstroMeasurementData()
    : AstroSubData(), measurementRow(0), value(0.0f), units(Astro_UnitsType_Undefined), timestamp(0)
{
    type = 0; // no type differentiation
}

void AstroMeasurementData::toJSONObject(JsonObject &objectOut) const
{
    //AstroSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    objectOut[SFP(AStr_Key_MeasurementRow)] = measurementRow;
    objectOut[SFP(AStr_Key_Value)] = value;
    objectOut[SFP(AStr_Key_Units)] = unitsTypeToSymbol(units);
    objectOut[SFP(AStr_Key_Timestamp)] = timestamp;
}

void AstroMeasurementData::fromJSONObject(JsonObjectConst &objectIn)
{
    //AstroSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    measurementRow = objectIn[SFP(AStr_Key_MeasurementRow)] | measurementRow;
    value = objectIn[SFP(AStr_Key_Value)] | value;
    units = unitsTypeFromSymbol(objectIn[SFP(AStr_Key_Units)]);
    timestamp = objectIn[SFP(AStr_Key_Timestamp)] | timestamp;
}

void AstroMeasurementData::fromJSONVariant(JsonVariantConst &variantIn)
{
    if (variantIn.is<JsonObjectConst>()) {
        JsonObjectConst variantObj = variantIn;
        fromJSONObject(variantObj);
    } else if (variantIn.is<float>() || variantIn.is<int>()) {
        value = variantIn.as<float>();
    } else {
        Astro_SOFT_ASSERT(false, SFP(AStr_Err_UnsupportedOperation));
    }
}
