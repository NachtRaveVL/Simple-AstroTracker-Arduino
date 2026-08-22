/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Scheduler
*/

#ifndef AstroScheduler_H
#define AstroScheduler_H

#include "AstroCamera.h"
#include "AstroCover.h"
#include "AstroData.h"
#include "AstroLogger.h"
#include "AstroMounts.h"
#include "AstroThermal.h"
#include "AstroTriggers.h"

// Scheduler Configuration
// Runtime scheduling thresholds used to control deploy, observing, and stow sequences.
struct AstroSchedulerConfig {
    double deploySunAltitudeDegrees;                        // Sun altitude that allows nighttime deployment
    double stowSunAltitudeDegrees;                          // Sun altitude that begins morning stow
    double alignmentToleranceDegrees;                       // Mount alignment tolerance before settling
    uint16_t settleSeconds;                                 // Required stable alignment time before observing
    uint32_t reportIntervalSeconds;                         // Interval between environment reports

    AstroSchedulerConfig();
};

// Scheduler Serialization Sub Data
// A part of ASYS system data, retaining the same settings used by the runtime scheduler.
struct AstroSchedulerSubData : public AstroSchedulerConfig, public AstroSubData {
    AstroSchedulerSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

// Scheduler
// Coordinates the nighttime deploy, slew, settle, observe, warm-up, and safe-stow sequence.
class AstroScheduler {
public:
    AstroScheduler();

    void setMount(SharedPtr<AstroMount> mount);
    void setCover(SharedPtr<AstroCover> cover);
    void setObservationDevice(SharedPtr<AstroCameraTrigger> device);
    void setThermalBalancer(AstroThermalBalancer *thermal);
    void setSafetyTrigger(SharedPtr<AstroTrigger> trigger);
    void setLogger(AstroLogger *logger);
    void setTarget(Astro_TargetId targetId);
    void setConfig(const AstroSchedulerConfig &config);

    void update();
    void unresolveAny(AstroObject *object);

    inline Astro_SchedulerStage getStage() const { return _stage; }
    inline bool inNighttimeMode() const
        { return _stage >= Astro_SchedulerStage_Deploying && _stage <= Astro_SchedulerStage_Observing; }

protected:
    SharedPtr<AstroMount> _mount;                           // Managed mount
    SharedPtr<AstroCover> _cover;                           // Managed cover
    SharedPtr<AstroCameraTrigger> _device;                  // Observation device
    AstroThermalBalancer *_thermal;                         // Thermal balancer, not owned
    AstroTriggerAttachment _safetyTrigger;                  // Observing safety trigger attachment
    AstroLogger *_logger;                                   // System logger, not owned
    Astro_TargetId _targetId;                               // Active observation target
    AstroSchedulerConfig _config;                           // Active scheduler configuration
    Astro_SchedulerStage _stage;                            // Current scheduler stage
    int64_t _stageStart;                                    // Stage start timestamp
    int64_t _settleStart;                                   // Alignment settle start timestamp
    int64_t _lastEnvReport;                                 // Last environment report timestamp

    void enterStage(Astro_SchedulerStage stage, int64_t unixTime);
    void reportEnvironment(int64_t unixTime, const AstroThermalReadings &readings,
                           const AstroThermalOutputs &outputs);
};

#endif // /ifndef AstroScheduler_H
