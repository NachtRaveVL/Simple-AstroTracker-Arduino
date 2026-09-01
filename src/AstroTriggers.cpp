/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Triggers
*/

#include "Astruino.h"
#include <string.h>

AstroTrigger *newTriggerObjectFromSubData(const AstroTriggerSubData *dataIn)
{
    if (!dataIn || !isValidType(dataIn->type)) return nullptr;
    ASTRO_SOFT_ASSERT(dataIn && isValidType(dataIn->type), SFP(AStr_Err_InvalidParameter));

    if (dataIn) {
        switch (dataIn->type) {
            case AstroTrigger::MeasureValue: return new AstroMeasurementValueTrigger(dataIn);
            case AstroTrigger::MeasureRange: return new AstroMeasurementRangeTrigger(dataIn);
            default: break;
        }
    }

    return nullptr;
}

AstroTrigger::AstroTrigger(AstroIdentity sensorId, uint8_t measurementRow, double detriggerTol,
                           millis_t detriggerDelay, int typeIn)
    : AstroSubObject(nullptr), AstroMeasurementUnitsInterfaceStorageSingle(Astro_UnitsType_Undefined),
      type(static_cast<decltype(MeasureValue)>(typeIn)), _sensor(this, measurementRow),
      _detriggerTol(detriggerTol), _detriggerDelay(detriggerDelay), _lastTrigger(millis_none),
      _triggerState(Astro_TriggerState_Disabled), _triggerSignal()
{ _sensor.initObject(sensorId); }
AstroTrigger::AstroTrigger(SharedPtr<AstroSensor> sensor, uint8_t measurementRow, double detriggerTol,
                           millis_t detriggerDelay, int typeIn)
    : AstroSubObject(nullptr), AstroMeasurementUnitsInterfaceStorageSingle(Astro_UnitsType_Undefined),
      type(static_cast<decltype(MeasureValue)>(typeIn)), _sensor(this, measurementRow),
      _detriggerTol(detriggerTol), _detriggerDelay(detriggerDelay), _lastTrigger(millis_none),
      _triggerState(Astro_TriggerState_Disabled), _triggerSignal()
{ _sensor.initObject(sensor); }
AstroTrigger::AstroTrigger(const AstroTriggerSubData *dataIn)
    : AstroSubObject(nullptr), AstroMeasurementUnitsInterfaceStorageSingle(dataIn ? dataIn->measurementUnits : Astro_UnitsType_Undefined),
      type(static_cast<decltype(MeasureValue)>(dataIn ? (int)dataIn->type : (int)Unknown)),
      _sensor(this, dataIn ? dataIn->measurementRow : 0), _detriggerTol(dataIn ? dataIn->detriggerTol : 0.0),
      _detriggerDelay(dataIn ? dataIn->detriggerDelay : 0), _lastTrigger(millis_none),
      _triggerState(Astro_TriggerState_Disabled), _triggerSignal()
{ if (dataIn && dataIn->sensorName[0]) { _sensor.initObject(dataIn->sensorName); } }
void AstroTrigger::saveToData(AstroTriggerSubData *dataOut) const
{
    if (!dataOut) { return; }
    dataOut->type = (aid_t)type;
    dataOut->measurementRow = (int8_t)getMeasurementRow();
    dataOut->measurementUnits = getMeasurementUnits();
    dataOut->detriggerTol = _detriggerTol;
    dataOut->detriggerDelay = _detriggerDelay;
    if (_sensor.isSet()) {
        strncpy(dataOut->sensorName, _sensor.getKeyString().c_str(), ASTRO_NAME_MAXSIZE - 1);
        dataOut->sensorName[ASTRO_NAME_MAXSIZE - 1] = '\0';
    }
}
void AstroTrigger::update() { _sensor.updateIfNeeded(true); }
Astro_TriggerState AstroTrigger::getTriggerState(bool poll) { _sensor.updateIfNeeded(poll); return _triggerState; }
void AstroTrigger::setMeasurementUnits(Astro_UnitsType units, uint8_t row)
{ if (!row && _measurementUnits[0] != units) { _measurementUnits[0] = units; _sensor.setMeasurementUnits(units); bumpRevisionIfNeeded(); } }
Astro_UnitsType AstroTrigger::getMeasurementUnits(uint8_t row) const
{ return row ? Astro_UnitsType_Undefined : (_measurementUnits[0] != Astro_UnitsType_Undefined ? _measurementUnits[0] : _sensor.getMeasurementUnits()); }
Signal<Astro_TriggerState, ASTRO_TRIGGER_SIGNAL_SLOTS> &AstroTrigger::getTriggerSignal() { return _triggerSignal; }

AstroMeasurementValueTrigger::AstroMeasurementValueTrigger(AstroIdentity sensorId, double triggerTol, bool triggerBelow,
                                                           uint8_t row, double detriggerTol, millis_t detriggerDelay)
    : AstroTrigger(sensorId, row, detriggerTol, detriggerDelay, MeasureValue), _triggerTol(triggerTol), _triggerBelow(triggerBelow)
{ _sensor.setHandleMethod(&AstroMeasurementValueTrigger::handleMeasurement, this); }
AstroMeasurementValueTrigger::AstroMeasurementValueTrigger(SharedPtr<AstroSensor> sensor, double triggerTol, bool triggerBelow,
                                                           uint8_t row, double detriggerTol, millis_t detriggerDelay)
    : AstroTrigger(sensor, row, detriggerTol, detriggerDelay, MeasureValue), _triggerTol(triggerTol), _triggerBelow(triggerBelow)
{ _sensor.setHandleMethod(&AstroMeasurementValueTrigger::handleMeasurement, this); }
AstroMeasurementValueTrigger::AstroMeasurementValueTrigger(const AstroTriggerSubData *dataIn)
    : AstroTrigger(dataIn), _triggerTol(dataIn ? dataIn->dataAs.measureValue.tolerance : 0.0),
      _triggerBelow(dataIn ? dataIn->dataAs.measureValue.triggerBelow : true)
{ _sensor.setHandleMethod(&AstroMeasurementValueTrigger::handleMeasurement, this); }
void AstroMeasurementValueTrigger::saveToData(AstroTriggerSubData *dataOut) const
{ AstroTrigger::saveToData(dataOut); dataOut->dataAs.measureValue.tolerance = _triggerTol; dataOut->dataAs.measureValue.triggerBelow = _triggerBelow; }
void AstroMeasurementValueTrigger::setTriggerTolerance(double tolerance)
{ if (!isFPEqual(_triggerTol, tolerance)) { _triggerTol = tolerance; _sensor.setNeedsMeasurement(); bumpRevisionIfNeeded(); } }
void AstroMeasurementValueTrigger::handleMeasurement(const AstroMeasurement *measurement)
{
    if (!measurement || !measurement->isSet()) { return; }
    bool wasState = _triggerState == Astro_TriggerState_Triggered;
    AstroSingleMeasurement measure = getAsSingleMeasurement(measurement, getMeasurementRow());
    if (getMeasurementUnits() != Astro_UnitsType_Undefined && measure.units != getMeasurementUnits()) {
        measure.toUnits(getMeasurementUnits(), getMeasurementConvertParam());
    }
    _sensor.setMeasurement(measure);
    double tolAdditive = wasState ? _detriggerTol : 0.0;
    bool nextState = _triggerBelow ? measure.value <= _triggerTol + tolAdditive + DBL_EPSILON
                                   : measure.value >= _triggerTol - tolAdditive - DBL_EPSILON;
    millis_t now = nzMillis();
    if (_lastTrigger && (millis_t)(now - _lastTrigger) >= _detriggerDelay) { _lastTrigger = millis_none; }
    if (_triggerState == Astro_TriggerState_Disabled || (nextState != wasState && (nextState || !_lastTrigger))) {
        _triggerState = nextState ? Astro_TriggerState_Triggered : Astro_TriggerState_NotTriggered;
        _lastTrigger = nextState && _detriggerDelay ? now : millis_none;
        _triggerSignal.fire(_triggerState);
    }
}

AstroMeasurementRangeTrigger::AstroMeasurementRangeTrigger(AstroIdentity sensorId, double low, double high, bool outside,
                                                           uint8_t row, double detriggerTol, millis_t detriggerDelay)
    : AstroTrigger(sensorId, row, detriggerTol, detriggerDelay, MeasureRange), _triggerTolLow(low), _triggerTolHigh(high), _triggerOutside(outside)
{ _sensor.setHandleMethod(&AstroMeasurementRangeTrigger::handleMeasurement, this); }
AstroMeasurementRangeTrigger::AstroMeasurementRangeTrigger(SharedPtr<AstroSensor> sensor, double low, double high, bool outside,
                                                           uint8_t row, double detriggerTol, millis_t detriggerDelay)
    : AstroTrigger(sensor, row, detriggerTol, detriggerDelay, MeasureRange), _triggerTolLow(low), _triggerTolHigh(high), _triggerOutside(outside)
{ _sensor.setHandleMethod(&AstroMeasurementRangeTrigger::handleMeasurement, this); }
AstroMeasurementRangeTrigger::AstroMeasurementRangeTrigger(const AstroTriggerSubData *dataIn)
    : AstroTrigger(dataIn), _triggerTolLow(dataIn ? dataIn->dataAs.measureRange.toleranceLow : 0.0),
      _triggerTolHigh(dataIn ? dataIn->dataAs.measureRange.toleranceHigh : 0.0),
      _triggerOutside(dataIn ? dataIn->dataAs.measureRange.triggerOutside : true)
{ _sensor.setHandleMethod(&AstroMeasurementRangeTrigger::handleMeasurement, this); }
void AstroMeasurementRangeTrigger::saveToData(AstroTriggerSubData *dataOut) const
{
    AstroTrigger::saveToData(dataOut);
    dataOut->dataAs.measureRange.toleranceLow = _triggerTolLow;
    dataOut->dataAs.measureRange.toleranceHigh = _triggerTolHigh;
    dataOut->dataAs.measureRange.triggerOutside = _triggerOutside;
}
void AstroMeasurementRangeTrigger::setTriggerMidpoint(double midpoint)
{
    double half = (_triggerTolHigh - _triggerTolLow) * 0.5;
    if (!isFPEqual(_triggerTolLow, midpoint - half)) {
        _triggerTolLow = midpoint - half;
        _triggerTolHigh = midpoint + half;
        _sensor.setNeedsMeasurement();
        bumpRevisionIfNeeded();
    }
}
void AstroMeasurementRangeTrigger::handleMeasurement(const AstroMeasurement *measurement)
{
    if (!measurement || !measurement->isSet()) { return; }
    bool wasState = _triggerState == Astro_TriggerState_Triggered;
    AstroSingleMeasurement measure = getAsSingleMeasurement(measurement, getMeasurementRow());
    if (getMeasurementUnits() != Astro_UnitsType_Undefined && measure.units != getMeasurementUnits()) {
        measure.toUnits(getMeasurementUnits(), getMeasurementConvertParam());
    }
    _sensor.setMeasurement(measure);
    double tolAdditive = wasState ? _detriggerTol : 0.0;
    bool nextState = _triggerOutside
        ? (measure.value <= _triggerTolLow + tolAdditive + DBL_EPSILON || measure.value >= _triggerTolHigh - tolAdditive - DBL_EPSILON)
        : (measure.value >= _triggerTolLow - tolAdditive - DBL_EPSILON && measure.value <= _triggerTolHigh + tolAdditive + DBL_EPSILON);
    millis_t now = nzMillis();
    if (_lastTrigger && (millis_t)(now - _lastTrigger) >= _detriggerDelay) { _lastTrigger = millis_none; }
    if (_triggerState == Astro_TriggerState_Disabled || (nextState != wasState && (nextState || !_lastTrigger))) {
        _triggerState = nextState ? Astro_TriggerState_Triggered : Astro_TriggerState_NotTriggered;
        _lastTrigger = nextState && _detriggerDelay ? now : millis_none;
        _triggerSignal.fire(_triggerState);
    }
}

AstroTriggerSubData::AstroTriggerSubData()
    : AstroSubData(AstroTrigger::Unknown), sensorName{0}, measurementRow(0), dataAs(),
      detriggerTol(0.0), detriggerDelay(0), measurementUnits(Astro_UnitsType_Undefined)
{ ; }
void AstroTriggerSubData::toJSONObject(JsonObject &objectOut) const
{
    AstroSubData::toJSONObject(objectOut);
    if (sensorName[0]) { objectOut[SFP(AStr_Key_SensorName)] = sensorName; }
    if (measurementRow) { objectOut[SFP(AStr_Key_MeasurementRow)] = measurementRow; }
    if (detriggerTol > DBL_EPSILON) { objectOut[SFP(AStr_Key_DetriggerTol)] = detriggerTol; }
    if (detriggerDelay) { objectOut[SFP(AStr_Key_DetriggerDelay)] = detriggerDelay; }
    if (measurementUnits != Astro_UnitsType_Undefined) { objectOut[SFP(AStr_Key_MeasurementUnits)] = unitsTypeToSymbol(measurementUnits); }
    if (type == AstroTrigger::MeasureValue) {
        objectOut[SFP(AStr_Key_Tolerance)] = dataAs.measureValue.tolerance;
        objectOut[SFP(AStr_Key_TriggerBelow)] = dataAs.measureValue.triggerBelow;
    } else if (type == AstroTrigger::MeasureRange) {
        objectOut[SFP(AStr_Key_ToleranceLow)] = dataAs.measureRange.toleranceLow;
        objectOut[SFP(AStr_Key_ToleranceHigh)] = dataAs.measureRange.toleranceHigh;
        objectOut[SFP(AStr_Key_TriggerOutside)] = dataAs.measureRange.triggerOutside;
    }
}
void AstroTriggerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroSubData::fromJSONObject(objectIn);
    const char *sensor = objectIn[SFP(AStr_Key_SensorName)] | nullptr;
    if (sensor) { strncpy(sensorName, sensor, ASTRO_NAME_MAXSIZE - 1); sensorName[ASTRO_NAME_MAXSIZE - 1] = '\0'; }
    measurementRow = objectIn[SFP(AStr_Key_MeasurementRow)] | measurementRow;
    detriggerTol = objectIn[SFP(AStr_Key_DetriggerTol)] | detriggerTol;
    detriggerDelay = objectIn[SFP(AStr_Key_DetriggerDelay)] | detriggerDelay;
    measurementUnits = unitsTypeFromSymbol(objectIn[SFP(AStr_Key_MeasurementUnits)]);
    if (type == AstroTrigger::MeasureValue) {
        dataAs.measureValue.tolerance = objectIn[SFP(AStr_Key_Tolerance)] | dataAs.measureValue.tolerance;
        dataAs.measureValue.triggerBelow = objectIn[SFP(AStr_Key_TriggerBelow)] | dataAs.measureValue.triggerBelow;
    } else if (type == AstroTrigger::MeasureRange) {
        dataAs.measureRange.toleranceLow = objectIn[SFP(AStr_Key_ToleranceLow)] | dataAs.measureRange.toleranceLow;
        dataAs.measureRange.toleranceHigh = objectIn[SFP(AStr_Key_ToleranceHigh)] | dataAs.measureRange.toleranceHigh;
        dataAs.measureRange.triggerOutside = objectIn[SFP(AStr_Key_TriggerOutside)] | dataAs.measureRange.triggerOutside;
    }
}
