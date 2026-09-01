/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Scheduler
*/

#include "Astruino.h"

AstroScheduler::AstroScheduler()
    : _needsScheduling(false), _inNighttimeMode(false), _lastDay{0}
{ ; }

AstroScheduler::~AstroScheduler()
{
    while (_trackings.size()) {
        auto trackingIter = _trackings.begin();
        delete trackingIter->second;
        _trackings.erase(trackingIter);
    }
}

void AstroScheduler::update()
{
    if (hasSchedulerData()) {
        #ifdef ASTRO_USE_VERBOSE_OUTPUT
            Serial.println(F("Scheduler::update")); flushYield();
        #endif

        {   time_t time = unixNow();
            DateTime currTime = localTime(time);
            bool nighttimeMode = _dailyTwilight.isNighttime(time);

            if (_inNighttimeMode != nighttimeMode) {
                _inNighttimeMode = nighttimeMode;
                setNeedsScheduling();
                Astruino::_activeInstance->setNeedsRedraw();
            }

            if (!(_lastDay[0] == currTime.year()-2000 &&
                  _lastDay[1] == currTime.month() &&
                  _lastDay[2] == currTime.day())) {
                // only log uptime upon actual day change and if uptime has been at least 1d
                if (getLogger()->getSystemUptime() >= SECS_PER_DAY) {
                    getLogger()->logSystemUptime();
                }
                broadcastDateChange();
            }
        }

        if (needsScheduling()) { performScheduling(); }

        for (auto trackingIter = _trackings.begin(); trackingIter != _trackings.end(); ++trackingIter) {
            trackingIter->second->update();
        }

        #ifdef ASTRO_USE_VERBOSE_OUTPUT
            Serial.println(F("Scheduler::~update")); flushYield();
        #endif
    }
}

void AstroScheduler::setPreDuskHeatingMins(unsigned int heatingMins)
{
    ASTRO_SOFT_ASSERT(hasSchedulerData(), SFP(AStr_Err_NotYetInitialized));

    if (hasSchedulerData() && schedulerData()->preDuskHeatingMins != heatingMins) {
        schedulerData()->preDuskHeatingMins = heatingMins;

        setNeedsScheduling();
        Astruino::_activeInstance->_systemData->bumpRevisionIfNeeded();
    }
}

void AstroScheduler::setReportInterval(TimeSpan interval)
{
    ASTRO_SOFT_ASSERT(hasSchedulerData(), SFP(AStr_Err_NotYetInitialized));

    if (hasSchedulerData() && schedulerData()->reportInterval != interval.totalseconds()) {
        schedulerData()->reportInterval = interval.totalseconds();
        Astruino::_activeInstance->_systemData->bumpRevisionIfNeeded();
    }
}

unsigned int AstroScheduler::getPreDuskHeatingMins() const
{
    ASTRO_SOFT_ASSERT(hasSchedulerData(), SFP(AStr_Err_NotYetInitialized));
    return hasSchedulerData() ? schedulerData()->preDuskHeatingMins : 0;
}

TimeSpan AstroScheduler::getReportInterval() const
{
    ASTRO_SOFT_ASSERT(hasSchedulerData(), SFP(AStr_Err_NotYetInitialized));
    return TimeSpan(hasSchedulerData() ? schedulerData()->reportInterval : 0);
}

void AstroScheduler::updateNightTracking()
{
    time_t time = unixNow();
    DateTime currTime = localTime(time);
    _lastDay[0] = currTime.year()-2000;
    _lastDay[1] = currTime.month();
    _lastDay[2] = currTime.day();

    Location loc = getController()->getSystemLocation();
    if (loc.hasPosition()) {
        double transit; // high noon, hours +fractional
        calcSunriseSunset((unsigned long)time, loc.latitude, loc.longitude, transit, _dailyTwilight.sunrise, _dailyTwilight.sunset,
                          loc.resolveSunAlt(), ASTRO_SYS_SUNRISESET_CALCITERS);
        calcSunriseSunset((unsigned long)time + SECS_PER_DAY, loc.latitude, loc.longitude, transit, _tomorrowTwilight.sunrise, _tomorrowTwilight.sunset,
                          loc.resolveSunAlt(), ASTRO_SYS_SUNRISESET_CALCITERS);
        _dailyTwilight.isUTC = true;
        _tomorrowTwilight.isUTC = true;
    } else if (_dailyTwilight.isUTC) {
        _dailyTwilight = Twilight();
        _tomorrowTwilight = Twilight();
    }
    _inNighttimeMode = _dailyTwilight.isNighttime(time);

    setNeedsScheduling();
    Astruino::_activeInstance->setNeedsRedraw();
}

void AstroScheduler::performScheduling()
{
    ASTRO_HARD_ASSERT(hasSchedulerData(), SFP(AStr_Err_NotYetInitialized));

    for (auto iter = Astruino::_activeInstance->_objects.begin(); iter != Astruino::_activeInstance->_objects.end(); ++iter) {
        if (iter->second->isMountType()) {
            auto mount = static_pointer_cast<AstroMount>(iter->second);

            {   auto trackingIter = _trackings.find(mount->getKey());

                if (trackingIter != _trackings.end()) {
                    if (trackingIter->second) {
                        trackingIter->second->setupStaging();
                    }
                } else {
                    #ifdef ASTRO_USE_VERBOSE_OUTPUT
                        Serial.print(F("Scheduler::performScheduling Mount found for: ")); Serial.println(iter->second->getId().getDisplayString()); flushYield();
                    #endif

                    AstroTracking *tracking = new AstroTracking(mount);
                    ASTRO_SOFT_ASSERT(tracking, SFP(AStr_Err_AllocationFailure));
                    if (tracking) { _trackings[mount->getKey()] = tracking; }
                }
            }
        }
    }

    _needsScheduling = false;
}

void AstroScheduler::broadcastDateChange()
{
    updateNightTracking();

    #ifdef ASTRO_USE_MULTITASKING
        // these can take a while to complete
        taskManager.scheduleOnce(0, []{
            if (getController()) {
                getController()->broadcastDateChanged();
            }
            yield();
            if (getLogger()) {
                getLogger()->notifyDateChanged();
            }
            yield();
            if (getPublisher()) {
                getPublisher()->notifyDateChanged();
            }
            yield();
        });
    #else
        if (getController()) {
            getController()->broadcastDateChanged();
        }
        if (getLogger()) {
            getLogger()->notifyDateChanged();
        }
        if (getPublisher()) {
            getPublisher()->notifyDateChanged();
        }
    #endif
}


AstroProcess::AstroProcess(SharedPtr<AstroMount> mountIn)
    : mount(mountIn), stageStart(unixNow())
{ ; }

void AstroProcess::clearActuatorReqs()
{
    while (actuatorReqs.size()) {
        actuatorReqs.begin()->disableActivation();
        actuatorReqs.erase(actuatorReqs.begin());
    }
}

void AstroProcess::setActuatorReqs(const Vector<AstroActuatorAttachment, ASTRO_SCH_REQACTS_MAXSIZE> &actuatorReqsIn)
{
    for (auto attachIter = actuatorReqs.begin(); attachIter != actuatorReqs.end(); ++attachIter) {
        bool found = false;
        auto key = attachIter->getKey();

        for (auto attachInIter = actuatorReqsIn.begin(); attachInIter != actuatorReqsIn.end(); ++attachInIter) {
            if (key == attachInIter->getKey()) {
                found = true;
                break;
            }
        }

        if (!found) { // disables actuators not found in new list
            attachIter->disableActivation();
        }
    }

    {   actuatorReqs.clear();
        for (auto attachInIter = actuatorReqsIn.begin(); attachInIter != actuatorReqsIn.end(); ++attachInIter) {
            actuatorReqs.push_back(*attachInIter);
            actuatorReqs.back().setParent(nullptr);
        }
    }
}


AstroTracking::AstroTracking(SharedPtr<AstroMount> mount)
    : AstroProcess(mount), stage(Init), canProcessAfter(0), lastEnvReport(0),
      stormingReported(false), daytimeSeqReported(false), stowageSeqReported(false)
{
    setupStaging();
}

AstroTracking::~AstroTracking()
{
    clearActuatorReqs();
}

void AstroTracking::setupStaging()
{
    #ifdef ASTRO_USE_VERBOSE_OUTPUT
    {   static int8_t _stageFS1 = (int8_t)-1; if (_stageFS1 != (int8_t)stage) {
        Serial.print(F("Tracking::setupStaging stage: ")); Serial.println((_stageFS1 = (int8_t)stage)); flushYield(); } }
    #endif

    bool isStorming = mount->getStormingTriggerAttachment().isTriggered();
    auto coverDriver = mount->getMountCoverDriver();
    auto &camera = mount->getCamera();
    auto &thermalBalancer = mount->getThermalBalancer();

    Vector<AstroActuatorAttachment, ASTRO_SCH_REQACTS_MAXSIZE> newActuatorReqs;

    switch (stage) {
        case Init:
        case Stow: {
            camera.stopObservation();
            mount->park();
            thermalBalancer.setMode(isStorming ? Astro_ThermalMode_SafeStowed : Astro_ThermalMode_DayStorage);
            if (coverDriver && coverDriver->isMoving()) { coverDriver->stop(); }
        } break;

        case Warm: {
            camera.stopObservation();
            mount->park();
            thermalBalancer.setMode(Astro_ThermalMode_DayStorage);
            if (coverDriver && mount->isParked() && thermalBalancer.cameraSafeToStow()) { coverDriver->close(); }
        } break;

        case Deploy: {
            camera.stopObservation();
            mount->park();
            thermalBalancer.setMode(Astro_ThermalMode_NightObserving);
            if (coverDriver) { coverDriver->open(); }
        } break;

        case Acquire: {
            camera.stopObservation();
            thermalBalancer.setMode(Astro_ThermalMode_NightObserving);
            if (coverDriver) { coverDriver->open(); }
            mount->unpark();
            mount->track();
        } break;

        case Track: {
            thermalBalancer.setMode(Astro_ThermalMode_NightObserving);
            if (coverDriver) { coverDriver->open(); }
            mount->unpark();
            mount->track();
            if (camera.ready()) { camera.startObservation(); }
        } break;

        default:
            break;
    }

    setActuatorReqs(newActuatorReqs);
    canProcessAfter = unixNow();

    #ifdef ASTRO_USE_VERBOSE_OUTPUT
    {   static int8_t _stageFS2 = (int8_t)-1; if (_stageFS2 != (int8_t)stage) {
        Serial.print(F("Tracking::~setupStaging stage: ")); Serial.println((_stageFS2 = (int8_t)stage)); flushYield(); } }
    #endif
}

void AstroTracking::update()
{
    #ifdef ASTRO_USE_VERBOSE_OUTPUT
    {   static int8_t _stageFU1 = (int8_t)-1; if (_stageFU1 != (int8_t)stage) {
        Serial.print(F("Tracking::update stage: ")); Serial.println((_stageFU1 = (int8_t)stage)); flushYield(); } }
    #endif

    time_t time = unixNow();
    bool reportDue = (!lastEnvReport || time >= lastEnvReport + getScheduler()->schedulerData()->reportInterval) &&
                     getScheduler()->schedulerData()->reportInterval > 0; // 0 disables
    auto temperatureSensor = mount->getTemperatureSensor();
    auto windSpeedSensor = mount->getWindSpeedSensor();

    if (reportDue && (temperatureSensor || windSpeedSensor)) {
        getLogger()->logProcess(mount.get(), SFP(AStr_Log_EnvReport));
        if (mount->getTemperatureSensor(true)) {
            #ifdef ASTRO_USE_MULTITASKING
                mount->getTemperatureSensor()->yieldForMeasurement();
            #endif
            auto temp = mount->getTemperatureSensorAttachment().getMeasurement();
            convertUnits(&temp, defaultTemperatureUnits());
            getLogger()->logMessage(SFP(AStr_Log_Field_Temp_Measured), measurementToString(temp));
        }
        if (mount->getWindSpeedSensor(true)) {
            #ifdef ASTRO_USE_MULTITASKING
                mount->getWindSpeedSensor()->yieldForMeasurement();
            #endif
            auto windSpeed = mount->getWindSpeedSensorAttachment().getMeasurement();
            convertUnits(&windSpeed, defaultSpeedUnits());
            getLogger()->logMessage(SFP(AStr_Log_Field_WindSpeed_Measured), measurementToString(windSpeed));
        }
        lastEnvReport = time;
    }

    if (!canProcessAfter || time >= canProcessAfter) {
        auto stageWas = stage != Init ? stage : Stow;
        auto currTime = localTime(time);
        bool logStage = false;
        auto sunrise = getScheduler()->getDailyTwilight().getSunriseLocalTime();
        auto sunset = getScheduler()->getDailyTwilight().getSunsetLocalTime();
        bool beforeSunrise = currTime < sunrise;
        bool afterSunset = currTime >= sunset;
        bool nighttime = beforeSunrise || afterSunset;
        auto nextSunrise = beforeSunrise ? sunrise : getScheduler()->getTomorrowTwilight().getSunriseLocalTime();
        bool preHeatingDue = !nighttime && currTime >= sunset - TimeSpan(0,0,getScheduler()->schedulerData()->preDuskHeatingMins,0) &&
                             mount->getHeatingTriggerAttachment().isTriggered() &&
                             linksFilterActuatorsByMountAndType(mount->getLinkages(), mount.get(), Astro_ActuatorType_DewHeater).size();
        bool isStorming = mount->getStormingTriggerAttachment().isTriggered();
        auto coverDriver = mount->getMountCoverDriver();
        auto &thermalBalancer = mount->getThermalBalancer();

        switch (stage) {
            case Init: {
                if (isStorming) {
                    stage = Stow; stageStart = time;
                    setupStaging();
                } else if (nighttime) {
                    stage = Deploy; stageStart = time;
                    setupStaging();
                } else if (preHeatingDue) {
                    stage = Warm; stageStart = time;
                    setupStaging();
                } else { // before-sunset / fail-safe
                    stage = Stow; stageStart = time;
                    setupStaging();
                }
            } break;

            case Warm: {
                if (isStorming) {
                    stage = Stow; stageStart = time;
                    setupStaging(); logStage = true;
                } else if (nighttime) {
                    stage = Deploy; stageStart = time;
                    setupStaging(); logStage = true;
                } else if (!preHeatingDue) {
                    stage = Stow; stageStart = time;
                    setupStaging(); logStage = true;
                } // else running heating
            } break;

            case Deploy: {
                if (!nighttime || isStorming) {
                    stage = Stow; stageStart = time;
                    setupStaging(); logStage = true;
                } else if (!mount->getMountCoverDriver() || mount->getMountCoverDriver()->isAligned()) {
                    stage = Acquire; stageStart = time;
                    setupStaging(); logStage = true;
                } // else running uncover
            } break;

            case Acquire: {
                if (!nighttime || isStorming) {
                    stage = Stow; stageStart = time;
                    setupStaging(); logStage = true;
                } else if (mount->isAligned()) {
                    stage = Track; stageStart = time;
                    setupStaging(); logStage = true;
                } // else acquiring target
            } break;

            case Track: {
                if (!nighttime || isStorming) {
                    stage = Stow; stageStart = time;
                    setupStaging(); logStage = true;
                } // else running tracking
            } break;

            case Stow: {
                if (mount->isParked() && thermalBalancer.cameraSafeToStow() && coverDriver) {
                    coverDriver->close();
                }

                bool stowed = mount->isParked() && thermalBalancer.cameraSafeToStow() &&
                              (!coverDriver || coverDriver->isClosed());

                if (stowed && !isStorming) {
                    if (nighttime) {
                        stage = Deploy; stageStart = time;
                        setupStaging(); logStage = true;
                    } else if (preHeatingDue) {
                        stage = Warm; stageStart = time;
                        setupStaging(); logStage = true;
                    } else if (!daytimeSeqReported) {
                        stormingReported = false; logStage = true;
                    }
                }

                if (stowageSeqReported && (!coverDriver || coverDriver->isClosed())) {
                    getLogger()->logProcess(mount.get(), SFP(AStr_Log_StowSequence), SFP(AStr_Log_HasEnded));
                    getLogger()->logMessage(SFP(AStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan(time - stageStart)));
                    stowageSeqReported = false;
                }
            } break;

            default:
                break;
        }

        if (logStage) {
            if (stageWas != stage) {
                switch (stageWas) {
                    case Warm: {
                        getLogger()->logProcess(mount.get(), SFP(AStr_Log_PreDuskWarmup), SFP(AStr_Log_HasEnded));
                        getLogger()->logMessage(SFP(AStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan(time - stageStart)));
                    } break;

                    case Deploy: {
                        if (mount->getMountCoverDriver() && mount->getMountCoverDriver()->isAligned()) {
                            getLogger()->logProcess(mount.get(), SFP(AStr_Log_DeploySequence), SFP(AStr_Log_HasEnded));
                            getLogger()->logMessage(SFP(AStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan(time - stageStart)));
                        }
                    } break;

                    case Acquire: {
                        getLogger()->logProcess(mount.get(), SFP(AStr_Log_AcquireSequence), SFP(AStr_Log_HasEnded));
                        getLogger()->logMessage(SFP(AStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan(time - stageStart)));
                    } break;

                    case Track: {
                        getLogger()->logProcess(mount.get(), SFP(AStr_Log_TrackingSequence), SFP(AStr_Log_HasEnded));
                        getLogger()->logMessage(SFP(AStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan(time - stageStart)));
                    } break;

                    case Stow: {
                        if (stormingReported) {
                            getLogger()->logProcess(mount.get(), SFP(AStr_Log_StormingSequence), SFP(AStr_Log_HasEnded));
                            getLogger()->logMessage(SFP(AStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan(time - stageStart)));
                            stormingReported = false;
                        } else if (daytimeSeqReported) {
                            getLogger()->logProcess(mount.get(), SFP(AStr_Log_DaytimeSequence), SFP(AStr_Log_HasEnded));
                            getLogger()->logMessage(SFP(AStr_Log_Field_Time_Measured), timeSpanToString(TimeSpan(time - stageStart)));
                            daytimeSeqReported = false;
                        }
                        if (stowageSeqReported) {
                            stowageSeqReported = false;
                        }
                    } break;
                }
            }

            switch (stage) {
                case Warm: {
                    getLogger()->logProcess(mount.get(), SFP(AStr_Log_PreDuskWarmup), SFP(AStr_Log_HasBegan));
                    getLogger()->logMessage(SFP(AStr_Log_Field_Heating_Duration), roundToString(getScheduler()->schedulerData()->preDuskHeatingMins), String('m'));
                    getLogger()->logMessage(SFP(AStr_Log_Field_Time_Start), localTime(stageStart).timestamp(DateTime::TIMESTAMP_TIME));
                    getLogger()->logMessage(SFP(AStr_Log_Field_Time_Finish), sunset.timestamp(DateTime::TIMESTAMP_TIME));
                } break;

                case Deploy: {
                    if (mount->getMountCoverDriver() && !mount->getMountCoverDriver()->isAligned()) {
                        getLogger()->logProcess(mount.get(), SFP(AStr_Log_DeploySequence), SFP(AStr_Log_HasBegan));
                    }
                } break;

                case Acquire: {
                    getLogger()->logProcess(mount.get(), SFP(AStr_Log_AcquireSequence), SFP(AStr_Log_HasBegan));
                    getLogger()->logMessage(SFP(AStr_Log_Field_Time_Start), localTime(stageStart).timestamp(DateTime::TIMESTAMP_TIME));
                } break;

                case Track: {
                    TimeSpan trackingSpan = nextSunrise - localTime(stageStart);
                    getLogger()->logProcess(mount.get(), SFP(AStr_Log_TrackingSequence), SFP(AStr_Log_HasBegan));
                    getLogger()->logMessage(SFP(AStr_Log_Field_Light_Duration), roundToString(trackingSpan.totalseconds() / (float)SECS_PER_HOUR, 1), String('h'));
                    getLogger()->logMessage(SFP(AStr_Log_Field_Time_Start), localTime(stageStart).timestamp(DateTime::TIMESTAMP_TIME));
                    getLogger()->logMessage(SFP(AStr_Log_Field_Time_Finish), nextSunrise.timestamp(DateTime::TIMESTAMP_TIME));
                } break;

                case Stow: {
                    if (isStorming && (nighttime || preHeatingDue) && !stormingReported) {
                        getLogger()->logProcess(mount.get(), SFP(AStr_Log_StormingSequence), SFP(AStr_Log_HasBegan));
                        stormingReported = true;
                    } else if (!nighttime && !daytimeSeqReported) {
                        getLogger()->logProcess(mount.get(), SFP(AStr_Log_DaytimeSequence), SFP(AStr_Log_HasBegan));
                        daytimeSeqReported = true;
                    }
                    if (mount->getMountCoverDriver() && !mount->getMountCoverDriver()->isClosed() && !stowageSeqReported) {
                        getLogger()->logProcess(mount.get(), SFP(AStr_Log_StowSequence), SFP(AStr_Log_HasBegan));
                        stowageSeqReported = true;
                    }
                } break;

                default:
                    break;
            }
        }
    }

    if (actuatorReqs.size()) {
        for (auto attachIter = actuatorReqs.begin(); attachIter != actuatorReqs.end(); ++attachIter) {
            attachIter->setupActivation();
            attachIter->enableActivation();
        }
    }

    #ifdef ASTRO_USE_VERBOSE_OUTPUT
    {   static int8_t _stageFU2 = (int8_t)-1; if (_stageFU2 != (int8_t)stage) {
        Serial.print(F("Tracking::~update stage: ")); Serial.println((_stageFU2 = (int8_t)stage)); flushYield(); } }
    #endif
}


AstroSchedulerSubData::AstroSchedulerSubData()
    : AstroSubData(0), reportInterval(8 * SECS_PER_HOUR), preDuskHeatingMins(10)
{ ; }

void AstroSchedulerSubData::toJSONObject(JsonObject &objectOut) const
{
    //AstroSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    if (preDuskHeatingMins != 10) { objectOut[SFP(AStr_Key_PreDuskHeatingMins)] = preDuskHeatingMins; }
    if (reportInterval != (8 * SECS_PER_HOUR)) { objectOut[SFP(AStr_Key_ReportInterval)] = reportInterval; }
}

void AstroSchedulerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    //AstroSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    preDuskHeatingMins = objectIn[SFP(AStr_Key_PreDuskHeatingMins)] | preDuskHeatingMins;
    reportInterval = objectIn[SFP(AStr_Key_ReportInterval)] | reportInterval;
}
