/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Triggers
*/

#ifndef AstroTriggers_H
#define AstroTriggers_H

#include "AstroAttachments.h"
#include "AstroInterfaces.h"

// Trigger Base
// This is the base class for all triggers, which are used to alert the system
// to some change in a tracked property.
class AstroTrigger : public AstroSubObject,
                     public AstroTriggerObjectInterface,
                     public AstroSensorAttachmentInterface {
public:
    AstroTrigger(AstroIdentity sensorId = AstroIdentity(),
                 uint8_t measurementRow = 0,
                 uint32_t stableTimeMs = 0);
    AstroTrigger(SharedPtr<AstroSensor> sensor,
                 uint8_t measurementRow = 0,
                 uint32_t stableTimeMs = 0);

    virtual void update();
    virtual void unresolveAny(AstroObject *object) override { _sensor.unresolveAny(object); }
    void reset(bool state = false, millis_t now = astroNZMillis());

    virtual bool isTriggered() const override { return _state; }
    inline bool getPendingState() const { return _pendingState; }
    inline uint32_t getStableTimeMs() const { return _stableTimeMs; }
    inline void setStableTimeMs(uint32_t stableTimeMs) { _stableTimeMs = stableTimeMs; bumpRevisionIfNeeded(); }

    virtual AstroSensorAttachment &getSensorAttachment() override { return _sensor; }

protected:
    AstroSensorAttachment _sensor;                          // Sensor attachment
    bool _state;                                             // Current state flag
    bool _pendingState;                                      // Pending state awaiting stability time
    millis_t _pendingSince;                                  // Pending state start time, in milliseconds
    uint32_t _stableTimeMs;                                  // Minimum stable time, in milliseconds

    bool updateState(bool requestedState, millis_t now = astroNZMillis());
    virtual bool evaluateMeasurement(const AstroSingleMeasurement &measurement) const = 0;
};

// Sensor Data Measurement Value Trigger
// This trigger checks a measured value against a set tolerance value and is useful for
// simple comparisons that control triggering.
class AstroThresholdTrigger : public AstroTrigger {
public:
    AstroThresholdTrigger(AstroIdentity sensorId,
                          double threshold = 0.0,
                          bool triggerBelow = false,
                          uint8_t measurementRow = 0,
                          double tolerance = 0.0,
                          uint32_t stableTimeMs = 0);
    AstroThresholdTrigger(SharedPtr<AstroSensor> sensor,
                          double threshold = 0.0,
                          bool triggerBelow = false,
                          uint8_t measurementRow = 0,
                          double tolerance = 0.0,
                          uint32_t stableTimeMs = 0);

    inline void setThreshold(double threshold) { _threshold = threshold; _sensor.setNeedsMeasurement(); bumpRevisionIfNeeded(); }
    inline void setTolerance(double tolerance) { _tolerance = fabs(tolerance); _sensor.setNeedsMeasurement(); bumpRevisionIfNeeded(); }
    inline void setTriggerBelow(bool triggerBelow) { _triggerBelow = triggerBelow; _sensor.setNeedsMeasurement(); bumpRevisionIfNeeded(); }

    inline double getThreshold() const { return _threshold; }
    inline double getTolerance() const { return _tolerance; }
    inline bool getTriggerBelow() const { return _triggerBelow; }

protected:
    double _threshold;                                      // Threshold
    double _tolerance;                                      // De-trigger tolerance
    bool _triggerBelow;                                     // Trigger below flag

    virtual bool evaluateMeasurement(const AstroSingleMeasurement &measurement) const override;
};

// Sensor Data Measurement Range Trigger
// This trigger checks a measured value against a set tolerance range and is useful for
// ranged measurements that need to stay inside of or outside of a known range.
class AstroRangeTrigger : public AstroTrigger {
public:
    AstroRangeTrigger(AstroIdentity sensorId,
                      double low = 0.0,
                      double high = 0.0,
                      bool triggerOutside = true,
                      uint8_t measurementRow = 0,
                      double tolerance = 0.0,
                      uint32_t stableTimeMs = 0);
    AstroRangeTrigger(SharedPtr<AstroSensor> sensor,
                      double low = 0.0,
                      double high = 0.0,
                      bool triggerOutside = true,
                      uint8_t measurementRow = 0,
                      double tolerance = 0.0,
                      uint32_t stableTimeMs = 0);

    inline void setRange(double low, double high) { _low = low; _high = high; _sensor.setNeedsMeasurement(); bumpRevisionIfNeeded(); }
    inline void setTolerance(double tolerance) { _tolerance = fabs(tolerance); _sensor.setNeedsMeasurement(); bumpRevisionIfNeeded(); }
    inline void setTriggerOutside(bool triggerOutside) { _triggerOutside = triggerOutside; _sensor.setNeedsMeasurement(); bumpRevisionIfNeeded(); }

    inline double getLow() const { return _low; }
    inline double getHigh() const { return _high; }
    inline double getTolerance() const { return _tolerance; }
    inline bool getTriggerOutside() const { return _triggerOutside; }

protected:
    double _low;                                            // Low tolerance
    double _high;                                           // High tolerance
    double _tolerance;                                      // De-trigger tolerance
    bool _triggerOutside;                                   // Trigger outside flag

    virtual bool evaluateMeasurement(const AstroSingleMeasurement &measurement) const override;
};

#endif // /ifndef AstroTriggers_H
