/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Scheduler
*/

#include "Astruino.h"
#include "AstroUtils.h"


static String astroSchedulerStageToString(Astro_SchedulerStage stage)
{
    switch (stage) {
        case Astro_SchedulerStage_DayStowed: return SFP(AStr_DayStowed);
        case Astro_SchedulerStage_Deploying: return SFP(AStr_Deploying);
        case Astro_SchedulerStage_Cooling: return SFP(AStr_Cooling);
        case Astro_SchedulerStage_Slewing: return SFP(AStr_Slewing);
        case Astro_SchedulerStage_Settling: return SFP(AStr_Settling);
        case Astro_SchedulerStage_Observing: return SFP(AStr_Observing);
        case Astro_SchedulerStage_Warming: return SFP(AStr_Warming);
        case Astro_SchedulerStage_Stowing: return SFP(AStr_Stowing);
        case Astro_SchedulerStage_SafeStowed: return SFP(AStr_SafeStowed);
        case Astro_SchedulerStage_Fault: return SFP(AStr_Fault);
        default: return String();
    }
}

AstroSchedulerConfig::AstroSchedulerConfig()
    : deploySunAltitudeDegrees(ASTRO_SCH_DEPLOY_SUN_ALT_DEG),
      stowSunAltitudeDegrees(ASTRO_SCH_STOW_SUN_ALT_DEG),
      alignmentToleranceDegrees(ASTRO_SCH_ALIGN_TOL_DEG),
      settleSeconds(ASTRO_SCH_SETTLE_SECS),
      reportIntervalSeconds(ASTRO_SCH_REPORT_INTERVAL_SECS)
{ ; }

AstroScheduler::AstroScheduler()
    : _mount(nullptr), _cover(nullptr), _device(nullptr), _thermal(nullptr),
      _safetyTrigger(nullptr), _targetType(Astro_TargetType_M42),
      _needsScheduling(false), _lastDay{0},
      _stage(Astro_SchedulerStage_DayStowed), _stageStart(0), _settleStart(0), _lastEnvReport(0)
{ ; }

void AstroScheduler::setMount(SharedPtr<AstroMount> mount)
{
    _mount = mount;
}

void AstroScheduler::setCover(AstroCover *cover)
{
    _cover = cover;
}

void AstroScheduler::setObservationDevice(SharedPtr<AstroCameraTrigger> device)
{
    _device = device;
}

void AstroScheduler::setThermalBalancer(AstroThermalBalancer *thermal)
{
    _thermal = thermal;
}

void AstroScheduler::setSafetyTrigger(SharedPtr<AstroTrigger> trigger)
{
    _safetyTrigger.setObject(trigger);
}


void AstroScheduler::setTarget(Astro_TargetType targetType)
{
    _targetType = targetType;
    if (_mount) { _mount->setTarget(targetType); }
}

void AstroScheduler::setConfig(const AstroSchedulerConfig &config)
{
    ASTRO_SOFT_ASSERT(hasSchedulerData(), SFP(AStr_Err_NotYetInitialized));
    if (hasSchedulerData() &&
        (!isFPEqual(schedulerData()->deploySunAltitudeDegrees, config.deploySunAltitudeDegrees) ||
         !isFPEqual(schedulerData()->stowSunAltitudeDegrees, config.stowSunAltitudeDegrees) ||
         !isFPEqual(schedulerData()->alignmentToleranceDegrees, config.alignmentToleranceDegrees) ||
         schedulerData()->settleSeconds != config.settleSeconds ||
         schedulerData()->reportIntervalSeconds != config.reportIntervalSeconds)) {
        schedulerData()->deploySunAltitudeDegrees = config.deploySunAltitudeDegrees;
        schedulerData()->stowSunAltitudeDegrees = config.stowSunAltitudeDegrees;
        schedulerData()->alignmentToleranceDegrees = config.alignmentToleranceDegrees;
        schedulerData()->settleSeconds = config.settleSeconds;
        schedulerData()->reportIntervalSeconds = config.reportIntervalSeconds;

        setNeedsScheduling();
        Astruino::_activeInstance->_systemData->bumpRevisionIfNeeded();
    }
}

void AstroScheduler::unresolveAny(AstroObject *object)
{
    if (!object) { return; }
    if (_mount && _mount.get() == object) { _mount = nullptr; }
    if (_device && _device.get() == object) { _device = nullptr; }
    if (_safetyTrigger.get()) { _safetyTrigger.get()->unresolveAny(object); }
}

void AstroScheduler::enterStage(Astro_SchedulerStage stage, int64_t unixTime)
{
    if (_stage == stage) { return; }
    _stage = stage;
    _stageStart = unixTime;
    _settleStart = 0;
    if (getLogger()) {
        String stageName = astroSchedulerStageToString(stage);
        if (stageName.length()) { getLogger()->logMessage(stageName); }
    }
}

void AstroScheduler::reportEnvironment(int64_t unixTime, const AstroThermalReadings &readings,
                                       const AstroThermalOutputs &outputs)
{
    if (!getLogger() || !schedulerData()->reportIntervalSeconds) { return; }
    if (_lastEnvReport && unixTime < _lastEnvReport + schedulerData()->reportIntervalSeconds) { return; }
    String readingsReport;
    readingsReport.reserve(160);
    readingsReport.concat(F(" ambientC=")); readingsReport.concat(String(readings.ambientTemperatureC, 2));
    readingsReport.concat(F(" humidity=")); readingsReport.concat(String(readings.humidityPercent, 2));
    readingsReport.concat(F(" dewPointC=")); readingsReport.concat(String(outputs.dewPointC, 2));
    if (readings.opticsTemperatureC < 900.0) {
        readingsReport.concat(F(" opticsC=")); readingsReport.concat(String(readings.opticsTemperatureC, 2));
    }
    if (readings.cameraSensorTemperatureC < 900.0) {
        readingsReport.concat(F(" cameraSensorC=")); readingsReport.concat(String(readings.cameraSensorTemperatureC, 2));
    }
    if (readings.cameraBodyTemperatureC < 900.0) {
        readingsReport.concat(F(" cameraBodyC=")); readingsReport.concat(String(readings.cameraBodyTemperatureC, 2));
    }

    String outputsReport;
    outputsReport.reserve(96);
    if (outputs.dewHeaterPower >= 0.0f) {
        outputsReport.concat(F(" dewHeater=")); outputsReport.concat(String(outputs.dewHeaterPower * 100.0f, 1)); outputsReport.concat('%');
    }
    if (outputs.cameraCoolingPower >= 0.0f) {
        outputsReport.concat(F(" cameraCooling=")); outputsReport.concat(String(outputs.cameraCoolingPower * 100.0f, 1)); outputsReport.concat('%');
    }
    if (outputs.cameraFanPower >= 0.0f) {
        outputsReport.concat(F(" cameraFan=")); outputsReport.concat(String(outputs.cameraFanPower * 100.0f, 1)); outputsReport.concat('%');
    }

    getLogger()->logMessage(SFP(AStr_Log_EnvReport), readingsReport, outputsReport);
    _lastEnvReport = unixTime;
}

void AstroScheduler::update()
{
    if (!hasSchedulerData()) { return; }

    const int64_t unixTime = (int64_t)unixNow();
    if (unixTime <= 0) { return; }

    DateTime currTime = localTime(unixTime);
    if (!(_lastDay[0] == currTime.year()-2000 &&
          _lastDay[1] == currTime.month() &&
          _lastDay[2] == currTime.day())) {
        // only log uptime upon actual day change and if uptime has been at least 1d
        if (getLogger()->getSystemUptime() >= SECS_PER_DAY) {
            getLogger()->logSystemUptime();
        }
        broadcastDateChange();
    }

    if (needsScheduling()) { performScheduling(); }

    AstroEquatorialCoordinates sunCoordinates;
    double sunAltitudeDegrees = 90.0;
    if (getController() && astroResolveSolarSystemTarget(Astro_TargetType_Sun, unixTime, &sunCoordinates)) {
        sunAltitudeDegrees = astroEquatorialToHorizontal(sunCoordinates, getController()->getObserver(), unixTime).altitudeDegrees;
    }

    const bool safeToObserve = !_safetyTrigger.isSet() || !_safetyTrigger.isTriggered(true);
    if (_thermal) { _thermal->update(); }
    const AstroThermalReadings thermalReadings = _thermal ? _thermal->getReadings() : AstroThermalReadings();
    const AstroThermalOutputs thermalOutputs = _thermal ? _thermal->getOutputs() : AstroThermalOutputs();
    reportEnvironment(unixTime, thermalReadings, thermalOutputs);

    if ((_cover && _cover->isFaulted()) || (_mount && _mount->isLimitHit())) {
        if (_device) { _device->stopObservation(); }
        if (_mount && !_mount->isLimitHit()) { _mount->park(); }
        if (_thermal) { _thermal->setMode(Astro_ThermalMode_SafeStowed); }
        enterStage(Astro_SchedulerStage_Fault, unixTime);
        return;
    }

    if (!safeToObserve && _stage != Astro_SchedulerStage_Fault) {
        enterStage(Astro_SchedulerStage_SafeStowed, unixTime);
    }

    switch (_stage) {
        case Astro_SchedulerStage_DayStowed: {
            if (_thermal) { _thermal->setMode(Astro_ThermalMode_DayStorage); }
            if (_mount) { _mount->park(); }
            if (_cover && (!_mount || _mount->isParked())) { _cover->close(); }
            if (sunAltitudeDegrees <= schedulerData()->deploySunAltitudeDegrees &&
                (!_cover || _cover->isClosed()) && (!_mount || _mount->isParked())) {
                if (_cover) { _cover->open(); }
                enterStage(Astro_SchedulerStage_Deploying, unixTime);
            }
        } break;

        case Astro_SchedulerStage_Deploying: {
            if (_cover) { _cover->open(); }
            if (!_cover || _cover->isOpen()) {
                if (_mount) { _mount->unpark(); }
                if (_thermal) { _thermal->setMode(Astro_ThermalMode_NightObserving); }
                enterStage(Astro_SchedulerStage_Cooling, unixTime);
            }
        } break;

        case Astro_SchedulerStage_Cooling: {
            if (_thermal) { _thermal->setMode(Astro_ThermalMode_NightObserving); }
            if (!_thermal || _thermal->cameraStable(ASTRO_SCH_CAMERA_STABLE_DEG)) {
                if (_mount) {
                    _mount->unpark();
                    _mount->setTarget(_targetType);
                    _mount->track();
                }
                enterStage(Astro_SchedulerStage_Slewing, unixTime);
            }
        } break;

        case Astro_SchedulerStage_Slewing: {
            if (_mount) { _mount->track(); }
            if (!_mount || _mount->isAligned(schedulerData()->alignmentToleranceDegrees)) {
                enterStage(Astro_SchedulerStage_Settling, unixTime);
                _settleStart = unixTime;
            }
        } break;

        case Astro_SchedulerStage_Settling: {
            bool aligned = !_mount || _mount->isAligned(schedulerData()->alignmentToleranceDegrees);
            if (!aligned) { _settleStart = unixTime; }
            else if (unixTime >= _settleStart + schedulerData()->settleSeconds) {
                if (_device && _device->ready()) { _device->startObservation(); }
                enterStage(Astro_SchedulerStage_Observing, unixTime);
            }
        } break;

        case Astro_SchedulerStage_Observing: {
            if (sunAltitudeDegrees >= schedulerData()->stowSunAltitudeDegrees) {
                if (_device) { _device->stopObservation(); }
                if (_mount) { _mount->park(); }
                if (_thermal) { _thermal->setMode(Astro_ThermalMode_DayStorage); }
                enterStage(Astro_SchedulerStage_Warming, unixTime);
            }
        } break;

        case Astro_SchedulerStage_Warming: {
            if (_mount) { _mount->park(); }
            if (!_thermal || _thermal->cameraSafeToStow()) {
                enterStage(Astro_SchedulerStage_Stowing, unixTime);
            }
        } break;

        case Astro_SchedulerStage_Stowing: {
            if (_mount) { _mount->park(); }
            if (!_mount || _mount->isParked()) {
                if (_cover) { _cover->close(); }
                if (!_cover || _cover->isClosed()) { enterStage(Astro_SchedulerStage_DayStowed, unixTime); }
            }
        } break;

        case Astro_SchedulerStage_SafeStowed: {
            if (_device) { _device->stopObservation(); }
            if (_mount) { _mount->park(); }
            if (_thermal) { _thermal->setMode(Astro_ThermalMode_SafeStowed); }
            if (_cover && (!_mount || _mount->isParked())) { _cover->close(); }
            if (safeToObserve && (!_mount || _mount->isParked()) && (!_cover || _cover->isClosed())) {
                enterStage(Astro_SchedulerStage_DayStowed, unixTime);
            }
        } break;

        case Astro_SchedulerStage_Fault: {
            if (_device) { _device->stopObservation(); }
            if (_thermal) { _thermal->setMode(Astro_ThermalMode_SafeStowed); }
        } break;

        default: break;
    }
}

void AstroScheduler::updateDayTracking()
{
    DateTime currTime = localTime(unixNow());
    _lastDay[0] = currTime.year()-2000;
    _lastDay[1] = currTime.month();
    _lastDay[2] = currTime.day();

    setNeedsScheduling();
    Astruino::_activeInstance->setNeedsRedraw();
}

void AstroScheduler::performScheduling()
{
    ASTRO_HARD_ASSERT(hasSchedulerData(), SFP(AStr_Err_NotYetInitialized));

    _needsScheduling = false;
}

void AstroScheduler::broadcastDateChange()
{
    updateDayTracking();

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


AstroSchedulerSubData::AstroSchedulerSubData()
    : AstroSchedulerConfig(), AstroSubData(0)
{ ; }

void AstroSchedulerSubData::toJSONObject(JsonObject &objectOut) const
{
    AstroSubData::toJSONObject(objectOut);
    objectOut[SFP(AStr_Key_DeploySunAlt)] = deploySunAltitudeDegrees;
    objectOut[SFP(AStr_Key_StowSunAlt)] = stowSunAltitudeDegrees;
    objectOut[SFP(AStr_Key_AlignTol)] = alignmentToleranceDegrees;
    objectOut[SFP(AStr_Key_SettleSecs)] = settleSeconds;
    objectOut[SFP(AStr_Key_ReportSecs)] = reportIntervalSeconds;
}

void AstroSchedulerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroSubData::fromJSONObject(objectIn);
    deploySunAltitudeDegrees = objectIn[SFP(AStr_Key_DeploySunAlt)] | deploySunAltitudeDegrees;
    stowSunAltitudeDegrees = objectIn[SFP(AStr_Key_StowSunAlt)] | stowSunAltitudeDegrees;
    alignmentToleranceDegrees = objectIn[SFP(AStr_Key_AlignTol)] | alignmentToleranceDegrees;
    settleSeconds = objectIn[SFP(AStr_Key_SettleSecs)] | settleSeconds;
    reportIntervalSeconds = objectIn[SFP(AStr_Key_ReportSecs)] | reportIntervalSeconds;
}
