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

struct AxisFeedbackState { double primary = 0.0; double secondary = 0.0; bool available = true; };
static bool axisRead(void *context, uint8_t axisIndex, double *positionDegreesOut)
{
    AxisFeedbackState *state = (AxisFeedbackState *)context;
    if (!state->available || !positionDegreesOut) { return false; }
    *positionDegreesOut = axisIndex == 0 ? state->primary : state->secondary;
    return true;
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

    AstroValueSensor openLimit(Astro_SensorType_LimitSwitch, Astro_UnitsType_Raw_1, ASTRO_POS_SEARCH_FROMBEG, 0.0);
    AstroValueSensor closedLimit(Astro_SensorType_LimitSwitch, Astro_UnitsType_Raw_1, ASTRO_POS_SEARCH_FROMBEG, 1.0);
    AstroCover feedbackCover;
    ActuatorState feedbackCoverActState; AstroCallbackActuator feedbackCoverAct(actuatorWrite, &feedbackCoverActState);
    feedbackCover.setActuator(&feedbackCoverAct); feedbackCover.setOpenSensor(&openLimit); feedbackCover.setClosedSensor(&closedLimit);
    feedbackCover.open(); feedbackCover.update(1.0);
    check(!feedbackCover.isOpen() && feedbackCoverActState.power > 0.0f, "cover waits for open limit");
    closedLimit.setValue(0.0); openLimit.setValue(1.0); feedbackCover.update(0.1);
    check(feedbackCover.isOpen() && isFPEqual(feedbackCoverActState.power, 0.0f), "cover stops on open limit");
    closedLimit.setValue(1.0); feedbackCover.update(0.1);
    check(feedbackCover.isFaulted(), "contradictory cover limits fault");

    AstroCover timeoutCover;
    AstroValueSensor timeoutOpen(Astro_SensorType_LimitSwitch, Astro_UnitsType_Raw_1, ASTRO_POS_SEARCH_FROMBEG, 0.0);
    timeoutCover.setOpenSensor(&timeoutOpen); timeoutCover.setTravelTimeout(0.25); timeoutCover.open(); timeoutCover.update(0.5);
    check(timeoutCover.isFaulted(), "cover travel timeout faults");

    AstroMount mount(Astro_MountType_Equatorial);
    mount.setObserver(AstroObserver(49.2827, -123.1207));
    mount.setAxisRates(360.0, 360.0);
    AxisState axisState; mount.setAxisTargetCallback(axisWrite, &axisState);
    mount.unpark(); mount.setTarget(Astro_Target_M42); mount.track(); mount.update(1787011200, 1.0);
    check(mount.isAligned(0.01), "mount slews to target");
    check(axisState.writes == 2, "mount axis targets exported");
    mount.park(); mount.update(1787011201, 1.0);
    check(mount.isParked(), "mount parks");

    AstroMount parkedFeedbackMount(Astro_MountType_Equatorial);
    AxisFeedbackState parkedFeedbackState; parkedFeedbackState.primary = 15.0; parkedFeedbackState.secondary = 5.0;
    parkedFeedbackMount.setAxisPositionCallback(axisRead, &parkedFeedbackState); parkedFeedbackMount.update(1787011200, 0.0);
    check(!parkedFeedbackMount.isParked(), "position feedback invalidates stale parked state");
    parkedFeedbackMount.park();
    check(parkedFeedbackMount.isParking(), "feedback mount begins park movement");

    AstroMount feedbackMount(Astro_MountType_Equatorial);
    feedbackMount.setObserver(AstroObserver(49.2827, -123.1207)); feedbackMount.setAxisRates(360.0, 360.0);
    AxisFeedbackState feedbackState; feedbackMount.setAxisPositionCallback(axisRead, &feedbackState);
    feedbackMount.unpark(); feedbackMount.setTarget(Astro_Target_M31); feedbackMount.track(); feedbackMount.update(1787011200, 1.0);
    check(!feedbackMount.isAligned(0.01), "mount honors external position feedback");
    feedbackState.primary = feedbackMount.getPrimaryAxis().targetDegrees; feedbackState.secondary = feedbackMount.getSecondaryAxis().targetDegrees;
    feedbackMount.update(1787011200, 0.0);
    check(feedbackMount.isAligned(0.01), "mount aligns from external position feedback");

    AstroMount wrapMount(Astro_MountType_AltAz);
    wrapMount.setAxisRates(1.0, 360.0); wrapMount.setAxisPosition(0, 359.0); wrapMount.setAxisPosition(1, 0.0);
    wrapMount.setParkPosition(1.0, 0.0); wrapMount.unpark(); wrapMount.park(); wrapMount.update(1787011200, 1.0);
    check(isFPEqual(wrapMount.getPrimaryAxis().positionDegrees, 0.0), "altaz axis crosses zero by shortest path");
    wrapMount.update(1787011201, 1.0); check(wrapMount.isParked(), "altaz park completes across wrap");

    AstroMount limitedMount(Astro_MountType_Equatorial);
    limitedMount.setAxisLimits(0, -10.0, 10.0); limitedMount.setParkPosition(20.0, 0.0); limitedMount.unpark(); limitedMount.park();
    check(limitedMount.isLimitHit() && isFPEqual(limitedMount.getPrimaryAxis().targetDegrees, 10.0), "mount software limit clamps unsafe target");

    AstroMount singleAxis(Astro_MountType_SingleAxis);
    AxisState singleAxisState; singleAxis.setAxisTargetCallback(axisWrite, &singleAxisState); singleAxis.unpark(); singleAxis.track();
    singleAxis.pulseGuide(0, Astro_DirectionMode_Forward, 1000, 1.0); singleAxis.update(1787011200, 0.0);
    check(isFPEqual(singleAxis.getPrimaryAxis().targetDegrees, ASTRO_MOUNT_SIDEREAL_RATE_DEGPS), "pulse guide adds sidereal offset");
    singleAxis.update(1787011201, 10.0);
    check(singleAxisState.writes == 2, "single-axis mount exports one axis per update");
    check(singleAxis.getPrimaryAxis().targetDegrees > ASTRO_MOUNT_SIDEREAL_RATE_DEGPS, "single-axis mount advances at sidereal rate");

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
    scheduler.update(t++, 1.0, -10.0, false, readings);
    scheduler.update(t++, 1.0, -10.0, false, readings);
    check(mount.isParked() && cover.isClosed(), "safe stow completes before enclosure closure");

    AstroScheduler faultScheduler; faultScheduler.setCover(&timeoutCover); faultScheduler.update(t++, 0.1, -10.0, true, readings);
    check(faultScheduler.getStage() == Astro_SchedulerStage_Fault, "cover fault enters scheduler fault state");

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
