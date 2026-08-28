/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Scheduler
*/

#include "Astruino.h"
#include "AstroUtils.h"

AstroSchedulerConfig::AstroSchedulerConfig()
    : deploySunAltitudeDegrees(ASTRO_SCH_DEPLOY_SUN_ALT_DEG),
      stowSunAltitudeDegrees(ASTRO_SCH_STOW_SUN_ALT_DEG),
      alignmentToleranceDegrees(ASTRO_SCH_ALIGN_TOL_DEG),
      settleSeconds(ASTRO_SCH_SETTLE_SECS),
      reportIntervalSeconds(ASTRO_SCH_REPORT_INTERVAL_SECS)
{ ; }

AstroScheduler::AstroScheduler()
    : _mount(nullptr), _cover(nullptr), _device(nullptr), _thermal(nullptr),
      _safetyTrigger(nullptr), _logger(nullptr), _targetType(Astro_TargetType_M42), _config(),
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

void AstroScheduler::setLogger(AstroLogger *logger)
{
    _logger = logger;
}

void AstroScheduler::setTarget(Astro_TargetType targetType)
{
    _targetType = targetType;
    if (_mount) { _mount->setTarget(targetType); }
}

void AstroScheduler::setConfig(const AstroSchedulerConfig &config)
{
    _config = config;
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
    if (_logger) {
        AstroString stageName = schedulerStageToString(stage, true);
        if (!stageName.empty()) { _logger->logMessage(unixTime, stageName.c_str()); }
    }
}

void AstroScheduler::reportEnvironment(int64_t unixTime, const AstroThermalReadings &readings,
                                       const AstroThermalOutputs &outputs)
{
    if (!_logger || !_config.reportIntervalSeconds) { return; }
    if (_lastEnvReport && unixTime < _lastEnvReport + _config.reportIntervalSeconds) { return; }
    _logger->logEnvironment(unixTime, readings.ambientTemperatureC, readings.humidityPercent, outputs.dewPointC,
                            readings.opticsTemperatureC, readings.cameraSensorTemperatureC, readings.cameraBodyTemperatureC,
                            outputs.dewHeaterPower, outputs.cameraCoolingPower, outputs.cameraFanPower);
    _lastEnvReport = unixTime;
}

void AstroScheduler::update()
{
    const int64_t unixTime = (int64_t)unixNow();
    if (unixTime <= 0) { return; }

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
            if (sunAltitudeDegrees <= _config.deploySunAltitudeDegrees &&
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
            if (!_mount || _mount->isAligned(_config.alignmentToleranceDegrees)) {
                enterStage(Astro_SchedulerStage_Settling, unixTime);
                _settleStart = unixTime;
            }
        } break;

        case Astro_SchedulerStage_Settling: {
            bool aligned = !_mount || _mount->isAligned(_config.alignmentToleranceDegrees);
            if (!aligned) { _settleStart = unixTime; }
            else if (unixTime >= _settleStart + _config.settleSeconds) {
                if (_device && _device->ready()) { _device->startObservation(); }
                enterStage(Astro_SchedulerStage_Observing, unixTime);
            }
        } break;

        case Astro_SchedulerStage_Observing: {
            if (sunAltitudeDegrees >= _config.stowSunAltitudeDegrees) {
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
