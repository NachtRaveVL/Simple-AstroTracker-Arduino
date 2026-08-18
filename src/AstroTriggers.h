/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Triggers
*/

#ifndef AstroTriggers_H
#define AstroTriggers_H

#include "AstroMeasurements.h"
#include "AstroInterfaces.h"

// Trigger Base
// Base class for sensor/value conditions used by scheduler and equipment safety logic.
class AstroTrigger : public AstroTriggerObjectInterface {
public:
    AstroTrigger(uint32_t stableTimeMs = 0);

    bool update(bool requestedState, millis_t now = astroNZMillis());
    void reset(bool state = false, millis_t now = astroNZMillis());

    virtual bool isTriggered() const override { return _state; }
    inline bool getPendingState() const { return _pendingState; }
    inline uint32_t getStableTimeMs() const { return _stableTimeMs; }
    inline void setStableTimeMs(uint32_t stableTimeMs) { _stableTimeMs = stableTimeMs; }

protected:
    bool _state;                                             // Current state flag
    bool _pendingState;                                      // Pending state awaiting stability time
    millis_t _pendingSince;                                  // Pending state start time, in milliseconds
    uint32_t _stableTimeMs;                                  // Minimum stable time, in milliseconds
};

// Threshold Trigger
// Triggers when a value crosses a configured upper or lower threshold.
class AstroThresholdTrigger : public AstroTrigger {
public:
    AstroThresholdTrigger(double threshold = 0.0, bool triggerBelow = false,
                          double tolerance = 0.0, uint32_t stableTimeMs = 0); // Tolerance

    bool update(double value, millis_t now = astroNZMillis());

    inline void setThreshold(double threshold) { _threshold = threshold; }
    inline void setTolerance(double tolerance) { _tolerance = tolerance < 0.0 ? -tolerance : tolerance; }
    inline void setTriggerBelow(bool triggerBelow) { _triggerBelow = triggerBelow; }

protected:
    double _threshold;                                       // Threshold
    double _tolerance;                                       // Tolerance
    bool _triggerBelow;                                      // Trigger below
};

// Range Trigger
// Triggers when a value moves inside or outside a configured range.
class AstroRangeTrigger : public AstroTrigger {
public:
    AstroRangeTrigger(double low = 0.0, double high = 0.0, bool triggerOutside = true,
                      double tolerance = 0.0, uint32_t stableTimeMs = 0); // Tolerance

    bool update(double value, millis_t now = astroNZMillis());

    inline void setRange(double low, double high) { _low = low; _high = high; }
    inline void setTolerance(double tolerance) { _tolerance = tolerance < 0.0 ? -tolerance : tolerance; }
    inline void setTriggerOutside(bool triggerOutside) { _triggerOutside = triggerOutside; }

protected:
    double _low;                                             // Low
    double _high;                                            // High
    double _tolerance;                                       // Tolerance
    bool _triggerOutside;                                    // Trigger outside
};

#endif // /ifndef AstroTriggers_H
