/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Scheduler
*/

#ifndef AstroScheduler_H
#define AstroScheduler_H

class AstroScheduler;
struct AstroSchedulerSubData;
struct AstroProcess;
struct AstroTracking;

#include "Astruino.h"

// Scheduler
// The Scheduler acts as the system's main scheduling attendant, who looks through all
// the various equipment and mounts you have programmed in, and figures out the best
// case tracking processes that should occur to support them. It is also responsible
// for setting up and maintaining the drivers that get assigned to mounts (such as the
// various mechanical orientation devices in use), as well as determining when
// significant time or event changes have occurred and broadcasting such out.
class AstroScheduler {
public:
    AstroScheduler();
    ~AstroScheduler();

    void update();

    inline void setNeedsScheduling() { _needsScheduling = hasSchedulerData(); }
    inline bool needsScheduling() { return _needsScheduling; }
    inline bool inNighttimeMode() const { return _inNighttimeMode; }

    void setPreDuskHeatingMins(unsigned int heatingMins);
    void setReportInterval(TimeSpan reportInterval);

    unsigned int getPreDuskHeatingMins() const;
    TimeSpan getReportInterval() const;

    inline const Twilight &getDailyTwilight() const { return _dailyTwilight; }
    inline const Twilight &getTomorrowTwilight() const { return _tomorrowTwilight; }

protected:
    Twilight _dailyTwilight;                                // Daily twilight settings
    Twilight _tomorrowTwilight;                             // Tomorrow's twilight settings
    bool _needsScheduling;                                  // Needs rescheduling tracking flag
    bool _inNighttimeMode;                                  // Nighttime mode flag
    aposi_t _lastDay[3];                                    // Last day tracking for rescheduling (Y-2k,M,D)
    Map<akey_t, AstroTracking *, ASTRO_SCH_PROCS_MAXSIZE> _trackings; // Mount tracking processes

    friend class Astruino;
    friend struct AstroProcess;
    friend struct AstroTracking;

    inline AstroSchedulerSubData *schedulerData() const;
    inline bool hasSchedulerData() const;

    void updateNightTracking();
    void performScheduling();
    void broadcastDateChange();
};

// Scheduler Process Base
// Processes are created and managed by Scheduler to manage the nightly control
// sequences necessary for astronomical mount alignment.
struct AstroProcess {
    SharedPtr<AstroMount> mount;                            // Mount

    Vector<AstroActuatorAttachment, ASTRO_SCH_REQACTS_MAXSIZE> actuatorReqs; // Actuators required for this stage (keep-enabled list)

    time_t stageStart;                                      // Stage start time

    AstroProcess(SharedPtr<AstroMount> mount);

    void clearActuatorReqs();
    void setActuatorReqs(const Vector<AstroActuatorAttachment, ASTRO_SCH_REQACTS_MAXSIZE> &actuatorReqsIn);
};

// Scheduler Tracking Process
struct AstroTracking : public AstroProcess {
    enum : signed char {Init,Warm,Deploy,Acquire,Track,Stow} stage; // Current tracking stage

    time_t canProcessAfter;                                 // Time next processing can occur (unix/UTC), else 0/disabled
    time_t lastEnvReport;                                   // Last time an environment report was generated (unix/UTC)
    bool stormingReported;                                  // Flag for storming reported
    bool daytimeSeqReported;                                // Flag for daytime sequence reported
    bool stowageSeqReported;                                // Flag for stowage sequence reported

    AstroTracking(SharedPtr<AstroMount> mount);
    ~AstroTracking();

    void setupStaging();
    void update();

private:
    void reset();
};


// Scheduler Serialization Sub Data
// A part of ASYS system data.
struct AstroSchedulerSubData : public AstroSubData {
    // TODO: additional stored params.
    uint8_t preDuskHeatingMins;                             // Duration to run dew heaters/de-icers (if present/needed) before nightly tracking starts, in minutes (default: 10)
    time_t reportInterval;                                  // Interval between environmental sensor reports, in seconds (default: 8hrs)

    AstroSchedulerSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef AstroScheduler_H
