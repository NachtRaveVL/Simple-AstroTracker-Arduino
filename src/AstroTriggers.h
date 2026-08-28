/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Triggers
*/

#ifndef AstroTriggers_H
#define AstroTriggers_H

class AstroTrigger;
class AstroMeasurementValueTrigger;
class AstroMeasurementRangeTrigger;
struct AstroTriggerSubData;

#include "AstroAttachments.h"
#include "AstroData.h"
#include "AstroInterfaces.h"

enum Astro_TriggerState : int8_t {
    Astro_TriggerState_Disabled = 0,
    Astro_TriggerState_NotTriggered,
    Astro_TriggerState_Triggered,
    Astro_TriggerState_Count,
    Astro_TriggerState_Undefined = -1
};

extern AstroTrigger *newTriggerObjectFromSubData(const AstroTriggerSubData *dataIn);

class AstroTrigger : public AstroSubObject,
                     public AstroTriggerObjectInterface,
                     public AstroMeasurementUnitsInterfaceStorageSingle,
                     public AstroSensorAttachmentInterface {
public:
    const enum : signed char { MeasureValue, MeasureRange, Unknown = -1 } type;

    AstroTrigger(AstroIdentity sensorId,
                 uint8_t measurementRow,
                 double detriggerTol,
                 millis_t detriggerDelay,
                 int typeIn = Unknown);
    AstroTrigger(SharedPtr<AstroSensor> sensor,
                 uint8_t measurementRow,
                 double detriggerTol,
                 millis_t detriggerDelay,
                 int typeIn = Unknown);
    AstroTrigger(const AstroTriggerSubData *dataIn);

    virtual void saveToData(AstroTriggerSubData *dataOut) const;
    virtual void update();
    Astro_TriggerState getTriggerState(bool poll = false);
    virtual bool isTriggered() const override { return _triggerState == Astro_TriggerState_Triggered; }

    virtual void setMeasurementUnits(Astro_UnitsType measurementUnits, uint8_t measurementRow = 0) override;
    virtual Astro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const override;
    inline uint8_t getMeasurementRow() const { return _sensor.getMeasurementRow(); }
    inline double getMeasurementConvertParam() const { return _sensor.getMeasurementConvertParam(); }
    inline double getDetriggerTolerance() const { return _detriggerTol; }
    inline millis_t getDetriggerDelay() const { return _detriggerDelay; }
    inline bool isDetriggerDelayActive() const { return _lastTrigger != millis_none; }

    virtual AstroSensorAttachment &getSensorAttachment() override { return _sensor; }
    Signal<Astro_TriggerState, ASTRO_TRIGGER_SIGNAL_SLOTS> &getTriggerSignal();

protected:
    AstroSensorAttachment _sensor;                          // Sensor attachment
    double _detriggerTol;                                   // De-trigger tolerance additive
    millis_t _detriggerDelay;                               // De-trigger timing delay
    millis_t _lastTrigger;                                  // Last trigger millis
    Astro_TriggerState _triggerState;                       // Trigger state
    Signal<Astro_TriggerState, ASTRO_TRIGGER_SIGNAL_SLOTS> _triggerSignal; // Trigger signal

    virtual void handleMeasurement(const AstroMeasurement *measurement) = 0;
};

class AstroMeasurementValueTrigger : public AstroTrigger {
public:
    AstroMeasurementValueTrigger(AstroIdentity sensorId,
                                 double triggerTol,
                                 bool triggerBelow = true,
                                 uint8_t measurementRow = 0,
                                 double detriggerTol = 0.0,
                                 millis_t detriggerDelay = 0);
    AstroMeasurementValueTrigger(SharedPtr<AstroSensor> sensor,
                                 double triggerTol,
                                 bool triggerBelow = true,
                                 uint8_t measurementRow = 0,
                                 double detriggerTol = 0.0,
                                 millis_t detriggerDelay = 0);
    AstroMeasurementValueTrigger(const AstroTriggerSubData *dataIn);

    virtual void saveToData(AstroTriggerSubData *dataOut) const override;
    void setTriggerTolerance(double tolerance);
    inline double getTriggerTolerance() const { return _triggerTol; }
    inline bool getTriggerBelow() const { return _triggerBelow; }

protected:
    double _triggerTol;                                     // Trigger tolerance limit
    bool _triggerBelow;                                     // Trigger below flag
    virtual void handleMeasurement(const AstroMeasurement *measurement) override;
};

class AstroMeasurementRangeTrigger : public AstroTrigger {
public:
    AstroMeasurementRangeTrigger(AstroIdentity sensorId,
                                 double toleranceLow,
                                 double toleranceHigh,
                                 bool triggerOutside = true,
                                 uint8_t measurementRow = 0,
                                 double detriggerTol = 0.0,
                                 millis_t detriggerDelay = 0);
    AstroMeasurementRangeTrigger(SharedPtr<AstroSensor> sensor,
                                 double toleranceLow,
                                 double toleranceHigh,
                                 bool triggerOutside = true,
                                 uint8_t measurementRow = 0,
                                 double detriggerTol = 0.0,
                                 millis_t detriggerDelay = 0);
    AstroMeasurementRangeTrigger(const AstroTriggerSubData *dataIn);

    virtual void saveToData(AstroTriggerSubData *dataOut) const override;
    void setTriggerMidpoint(double toleranceMid);
    inline double getTriggerToleranceLow() const { return _triggerTolLow; }
    inline double getTriggerToleranceHigh() const { return _triggerTolHigh; }
    inline bool getTriggerOutside() const { return _triggerOutside; }

protected:
    double _triggerTolLow;                                  // Low value tolerance
    double _triggerTolHigh;                                 // High value tolerance
    bool _triggerOutside;                                   // Trigger on outside flag
    virtual void handleMeasurement(const AstroMeasurement *measurement) override;
};

struct AstroTriggerSubData : public AstroSubData {
    char sensorName[ASTRO_NAME_MAXSIZE];                    // Sensor identity string
    int8_t measurementRow;                                  // Measurement row
    union {
        struct { double tolerance; bool triggerBelow; } measureValue;
        struct { double toleranceLow; double toleranceHigh; bool triggerOutside; } measureRange;
    } dataAs;
    double detriggerTol;                                    // De-trigger tolerance
    millis_t detriggerDelay;                                // De-trigger delay millis
    Astro_UnitsType measurementUnits;                       // Measurement units

    AstroTriggerSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef AstroTriggers_H
