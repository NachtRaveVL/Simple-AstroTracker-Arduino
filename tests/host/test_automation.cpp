#include "Astruino.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

static void check(bool condition, const char *message)
{
    if (!condition) { std::cerr << "FAIL: " << message << std::endl; std::exit(1); }
}

struct TriggerState { int starts = 0; int stops = 0; };
static void cameraTrigger(void *context, bool active)
{
    TriggerState *state = (TriggerState *)context;
    if (active) { ++state->starts; } else { ++state->stops; }
}

struct LogState { int count = 0; };
static void logSink(void *context, const AstroLogEvent &) { ++((LogState *)context)->count; }

struct ActuatorState { float power = 0.0f; int writes = 0; };
static void actuatorWrite(void *context, float power)
{
    ActuatorState *state = (ActuatorState *)context;
    state->power = power; ++state->writes;
}

struct AxisState { int writes = 0; double primary = 0.0; double secondary = 0.0; };
static void axisWrite(void *context, uint8_t axisIndex, double targetDegrees)
{
    AxisState *state = (AxisState *)context;
    ++state->writes;
    if (axisIndex == 0) { state->primary = targetDegrees; } else { state->secondary = targetDegrees; }
}

struct PublishState { int count = 0; uint8_t columns = 0; };
static void publishSink(void *context, int64_t, const AstroDataColumn *, uint8_t columnCount)
{
    PublishState *state = (PublishState *)context;
    ++state->count; state->columns = columnCount;
}

int main()
{
    double dew = AstroThermalBalancer::calculateDewPoint(10.0, 90.0);
    check(dew > 8.0 && dew < 9.5, "dew point");

    AstroThermalBalancer thermal;
    thermal.setMode(Astro_ThermalMode_NightObserving);
    thermal.setCameraTarget(-10.0);
    thermal.setCameraCoolingRamp(120.0);
    AstroThermalReadings readings;
    readings.ambientTemperatureC = 10.0;
    readings.humidityPercent = 90.0;
    readings.opticsTemperatureC = 8.0;
    readings.cameraSensorTemperatureC = 5.0;
    readings.cameraBodyTemperatureC = 10.0;
    ActuatorState dewActState, coolActState, fanActState;
    AstroCallbackActuator dewAct(actuatorWrite, &dewActState);
    AstroCallbackActuator coolAct(actuatorWrite, &coolActState);
    AstroCallbackActuator fanAct(actuatorWrite, &fanActState);
    thermal.setDewHeater(&dewAct);
    thermal.setCameraCooler(&coolAct);
    thermal.setCameraFan(&fanAct);
    AstroThermalOutputs out = thermal.update(readings, 10.0);
    check(out.dewHeaterPower > 0.0f, "dew heater balances upward");
    check(out.cameraCoolingPower > 0.0f, "camera cooling demand");
    check(out.opticsTargetC <= readings.ambientTemperatureC + 4.01, "optics heat capped near ambient");
    check(dewActState.writes == 1 && isFPEqual(dewActState.power, out.dewHeaterPower), "dew heater actuator output");
    check(coolActState.writes == 1 && isFPEqual(coolActState.power, out.cameraCoolingPower), "camera cooler actuator output");

    AstroCover cover;
    ActuatorState coverActState; AstroCallbackActuator coverAct(actuatorWrite, &coverActState); cover.setActuator(&coverAct);
    cover.setTravelRate(1.0f); cover.open(); cover.update(1.0);
    check(cover.isOpen(), "cover opens");
    check(coverActState.writes >= 1, "cover actuator driven");
    cover.close(); cover.update(1.0);
    check(cover.isClosed(), "cover closes");

    AstroMount mount(Astro_MountType_Equatorial);
    mount.setObserver(AstroObserver(49.2827, -123.1207));
    mount.setAxisRates(360.0, 360.0);
    AxisState axisState; mount.setAxisTargetCallback(axisWrite, &axisState);
    mount.setTarget(Astro_Target_M42); mount.track(); mount.update(1787011200, 1.0);
    check(mount.isAligned(0.01), "mount slews to target");
    check(axisState.writes == 2, "mount axis targets exported");
    mount.stow(); mount.update(1787011201, 1.0);
    check(mount.isAligned(0.01), "mount stows");

    TriggerState triggerState;
    AstroCameraTrigger camera(cameraTrigger, &triggerState);
    AstroLogger logger; LogState logState; logger.setSink(logSink, &logState);
    AstroScheduler scheduler;
    scheduler.setMount(&mount); scheduler.setCover(&cover); scheduler.setObservationDevice(&camera); scheduler.setThermalBalancer(&thermal); scheduler.setLogger(&logger);
    scheduler.setTarget(Astro_Target_M31);
    AstroSchedulerConfig config; config.settleSeconds = 2; config.deploySunAltitudeDegrees = -6.0; config.stowSunAltitudeDegrees = -3.0; scheduler.setConfig(config);

    int64_t t = 1787011200;
    readings.cameraSensorTemperatureC = 10.0;
    scheduler.update(t++, 1.0, -10.0, true, readings);
    check(scheduler.getStage() == Astro_SchedulerStage_Deploying, "night deployment begins");
    scheduler.update(t++, 1.0, -10.0, true, readings);
    check(scheduler.getStage() == Astro_SchedulerStage_Cooling, "cover opens before cooling");
    readings.cameraSensorTemperatureC = -10.0;
    scheduler.update(t++, 10.0, -10.0, true, readings);
    check(scheduler.getStage() == Astro_SchedulerStage_Slewing, "cooling completes");
    scheduler.update(t++, 1.0, -10.0, true, readings);
    check(scheduler.getStage() == Astro_SchedulerStage_Settling, "slew completes");
    scheduler.update(t++, 1.0, -10.0, true, readings);
    scheduler.update(t++, 1.0, -10.0, true, readings);
    check(scheduler.getStage() == Astro_SchedulerStage_Observing, "settle enters observation");
    check(camera.isCapturing() && triggerState.starts == 1, "camera signaled");

    scheduler.update(t++, 1.0, -10.0, false, readings);
    check(scheduler.getStage() == Astro_SchedulerStage_SafeStowed, "unsafe condition forces safe state");
    check(!camera.isCapturing(), "unsafe condition stops camera");

    AstroLogger filterLogger; LogState filterState; filterLogger.setSink(logSink, &filterState); filterLogger.setLogLevel(Astro_LogLevel_Errors);
    filterLogger.logMessage(t, "hidden"); filterLogger.logWarning(t, "hidden"); filterLogger.logError(t, "shown");
    check(filterState.count == 1, "logger filtering");

    AstroPublisher publisher; PublishState publishState; publisher.setSink(publishSink, &publishState);
    check(publisher.addColumn(1) && publisher.addColumn(2), "publisher columns");
    publisher.publishData(1, 10.0, 5, t);
    check(publishState.count == 0, "publisher waits for frame");
    publisher.publishData(2, 20.0, 5, t);
    check(publishState.count == 1 && publishState.columns == 2, "publisher emits complete frame");

    check(logState.count > 0, "scheduler reporting/logging active");
    std::cout << "PASS automation" << std::endl;
    return 0;
}
