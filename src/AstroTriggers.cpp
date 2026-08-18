/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Triggers
*/

#include "AstroTriggers.h"

AstroTrigger::AstroTrigger(uint32_t stableTimeMs)
    : _state(false), _pendingState(false), _pendingSince(0), _stableTimeMs(stableTimeMs)
{ ; }

bool AstroTrigger::update(bool requestedState, millis_t now)
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

AstroThresholdTrigger::AstroThresholdTrigger(double threshold, bool triggerBelow,
                                             double tolerance, uint32_t stableTimeMs)
    : AstroTrigger(stableTimeMs), _threshold(threshold),
      _tolerance(tolerance < 0.0 ? -tolerance : tolerance), _triggerBelow(triggerBelow)
{ ; }

bool AstroThresholdTrigger::update(double value, millis_t now)
{
    bool requested;
    if (_triggerBelow) {
        requested = _state ? value < (_threshold + _tolerance) : value < (_threshold - _tolerance);
    } else {
        requested = _state ? value > (_threshold - _tolerance) : value > (_threshold + _tolerance);
    }
    return AstroTrigger::update(requested, now);
}

AstroRangeTrigger::AstroRangeTrigger(double low, double high, bool triggerOutside,
                                     double tolerance, uint32_t stableTimeMs)
    : AstroTrigger(stableTimeMs), _low(low), _high(high),
      _tolerance(tolerance < 0.0 ? -tolerance : tolerance), _triggerOutside(triggerOutside)
{ ; }

bool AstroRangeTrigger::update(double value, millis_t now)
{
    bool inside;
    if (_state == _triggerOutside) {
        inside = value >= (_low + _tolerance) && value <= (_high - _tolerance);
    } else {
        inside = value >= (_low - _tolerance) && value <= (_high + _tolerance);
    }
    return AstroTrigger::update(_triggerOutside ? !inside : inside, now);
}
