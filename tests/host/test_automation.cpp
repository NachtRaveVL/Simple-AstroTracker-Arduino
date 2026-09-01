#include "Astruino.h"
#include <cmath>
#include <cstdlib>
#include <iostream>

static void check(bool condition, const char *message)
{
    if (!condition) { std::cerr << "FAIL: " << message << std::endl; std::exit(1); }
}

struct LogState {
    int count = 0;
    Astro_LogLevel lastLevel = Astro_LogLevel_None;
    void handle(const AstroLogEvent event) { ++count; lastLevel = event.level; }
};

struct PublishState {
    int count = 0;
    uint8_t columns = 0;
    void handle(Pair<uint8_t, const AstroDataColumn *> event)
    {
        ++count;
        columns = event.first;
    }
};

struct TriggerState {
    int count = 0;
    Astro_TriggerState state = Astro_TriggerState_Undefined;
    void handle(Astro_TriggerState nextState) { ++count; state = nextState; }
};

int main()
{
    SharedPtr<AstroValueSensor> temperature(new AstroValueSensor(Astro_SensorType_Temperature,
                                                                 Astro_UnitsType_Temperature_Celsius,
                                                                 0, 11.0));
    AstroMeasurementValueTrigger trigger(temperature, 10.0, false, 0, 1.0, 0);
    TriggerState triggerState;
    MethodSlot<TriggerState, Astro_TriggerState> triggerSlot(&triggerState, &TriggerState::handle);
    trigger.getTriggerSignal().attach(triggerSlot);

    trigger.update();
    check(trigger.getTriggerState() == Astro_TriggerState_Triggered, "value trigger enters triggered state");
    check(triggerState.count == 1 && triggerState.state == Astro_TriggerState_Triggered,
          "trigger signal reports state transition");

    temperature->setValue(9.5);
    trigger.update();
    check(trigger.getTriggerState() == Astro_TriggerState_Triggered,
          "detrigger tolerance holds trigger inside hysteresis band");
    temperature->setValue(8.5);
    trigger.update();
    check(trigger.getTriggerState() == Astro_TriggerState_NotTriggered,
          "value trigger exits after hysteresis threshold");

    Astruino controller;
    controller.init();

    SharedPtr<AstroActuator> actuator(new AstroActuator(Astro_ActuatorType_Cover, 0));
    AstroActivationHandle forward(actuator, Astro_DirectionMode_Forward, 0.4f);
    AstroActivationHandle reverse(actuator, Astro_DirectionMode_Reverse, 0.8f);
    actuator->setEnableMode(Astro_EnableMode_Highest);
    actuator->update();
    check(isFPEqual(actuator->getDriveIntensity(), 0.4f), "highest mode uses signed highest request");
    actuator->setEnableMode(Astro_EnableMode_Lowest);
    actuator->update();
    check(isFPEqual(actuator->getDriveIntensity(), -0.8f), "lowest mode uses signed lowest request");

    LogState logState;
    MethodSlot<LogState, const AstroLogEvent> logSlot(&logState, &LogState::handle);
    controller.logger.getLogSignal().attach(logSlot);
    controller.logger.setLogLevel(Astro_LogLevel_Errors);
    controller.logger.logMessage("hidden");
    controller.logger.logWarning("hidden");
    controller.logger.logError("shown");
    check(logState.count == 1 && logState.lastLevel == Astro_LogLevel_Errors,
          "logger signal respects configured filtering");

    auto sensor1 = controller.addTemperatureSensor(13, 10);
    auto sensor2 = controller.addTemperatureSensor(14, 10);
    check(sensor1 && sensor2, "factory creates publisher sensors");
    controller.publisher.update();
    aposi_t column1 = controller.publisher.getColumnIndexStart(sensor1->getKey());
    aposi_t column2 = controller.publisher.getColumnIndexStart(sensor2->getKey());
    check(isValidIndex(column1) && isValidIndex(column2) && column1 != column2,
          "publisher tabulates registered sensors");

    PublishState publishState;
    MethodSlot<PublishState, Pair<uint8_t, const AstroDataColumn *> > publishSlot(&publishState, &PublishState::handle);
    controller.publisher.getPublishSignal().attach(publishSlot);
    controller.launch();
    aframe_t frame = controller.getPollingFrame();
    controller.publisher.publishData(column1, AstroSingleMeasurement(10.0f, Astro_UnitsType_Temperature_Celsius, unixNow(), frame));
    check(publishState.count == 0, "publisher waits for complete frame");
    controller.publisher.publishData(column2, AstroSingleMeasurement(20.0f, Astro_UnitsType_Temperature_Celsius, unixNow(), frame));
    check(publishState.count == 1 && publishState.columns == 2,
          "publisher emits complete polling frame");

    std::cout << "PASS automation" << std::endl;
    return 0;
}
