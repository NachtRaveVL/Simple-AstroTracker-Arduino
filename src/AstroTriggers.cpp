/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Triggers
*/

#include "Astruino.h"

AstroTrigger::AstroTrigger(AstroIdentity sensorId, uint8_t measurementRow, uint32_t stableTimeMs)
    : AstroSubObject(nullptr), _sensor(this, measurementRow), _state(false), _pendingState(false),
      _pendingSince(0), _stableTimeMs(stableTimeMs)
{
    _sensor.initObject(sensorId);
}

AstroTrigger::AstroTrigger(SharedPtr<AstroSensor> sensor, uint8_t measurementRow, uint32_t stableTimeMs)
    : AstroSubObject(nullptr), _sensor(this, measurementRow), _state(false), _pendingState(false),
      _pendingSince(0), _stableTimeMs(stableTimeMs)
{
    _sensor.initObject(sensor);
}

void AstroTrigger::update()
{
    _sensor.updateIfNeeded(true);
    const AstroSingleMeasurement &measurement = _sensor.getMeasurement();
    if (measurement.isSet()) { updateState(evaluateMeasurement(measurement)); }
}

bool AstroTrigger::updateState(bool requestedState, millis_t now)
{
    if (requestedState == _state) {
        _pendingState = _state;
        _pendingSince = now;
        return _state;
    }

    if (_pendingState != requestedState) {
        _pendingState = requestedState;
        _pendingSince = now;
        if (!_stableTimeMs) { _state = requestedState; }
    } else if (!_stableTimeMs || (millis_t)(now - _pendingSince) >= _stableTimeMs) {
        _state = requestedState;
    }
    return _state;
}

void AstroTrigger::reset(bool state, millis_t now)
{
    _state = state;
    _pendingState = state;
    _pendingSince = now;
}

AstroThresholdTrigger::AstroThresholdTrigger(AstroIdentity sensorId, double threshold, bool triggerBelow,
                                             uint8_t measurementRow, double tolerance, uint32_t stableTimeMs)
    : AstroTrigger(sensorId, measurementRow, stableTimeMs), _threshold(threshold),
      _tolerance(fabs(tolerance)), _triggerBelow(triggerBelow)
{ ; }

AstroThresholdTrigger::AstroThresholdTrigger(SharedPtr<AstroSensor> sensor, double threshold, bool triggerBelow,
                                             uint8_t measurementRow, double tolerance, uint32_t stableTimeMs)
    : AstroTrigger(sensor, measurementRow, stableTimeMs), _threshold(threshold),
      _tolerance(fabs(tolerance)), _triggerBelow(triggerBelow)
{ ; }

bool AstroThresholdTrigger::evaluateMeasurement(const AstroSingleMeasurement &measurement) const
{
    if (_triggerBelow) {
        return _state ? measurement.value < (_threshold + _tolerance)
                      : measurement.value < (_threshold - _tolerance);
    }
    return _state ? measurement.value > (_threshold - _tolerance)
                  : measurement.value > (_threshold + _tolerance);
}

AstroRangeTrigger::AstroRangeTrigger(AstroIdentity sensorId, double low, double high, bool triggerOutside,
                                     uint8_t measurementRow, double tolerance, uint32_t stableTimeMs)
    : AstroTrigger(sensorId, measurementRow, stableTimeMs), _low(low), _high(high),
      _tolerance(fabs(tolerance)), _triggerOutside(triggerOutside)
{ ; }

AstroRangeTrigger::AstroRangeTrigger(SharedPtr<AstroSensor> sensor, double low, double high, bool triggerOutside,
                                     uint8_t measurementRow, double tolerance, uint32_t stableTimeMs)
    : AstroTrigger(sensor, measurementRow, stableTimeMs), _low(low), _high(high),
      _tolerance(fabs(tolerance)), _triggerOutside(triggerOutside)
{ ; }

bool AstroRangeTrigger::evaluateMeasurement(const AstroSingleMeasurement &measurement) const
{
    bool inside;
    if (_state == _triggerOutside) {
        inside = measurement.value >= (_low + _tolerance) && measurement.value <= (_high - _tolerance);
    } else {
        inside = measurement.value >= (_low - _tolerance) && measurement.value <= (_high + _tolerance);
    }
    return _triggerOutside ? !inside : inside;
}
