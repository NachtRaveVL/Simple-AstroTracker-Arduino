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

    SharedPtr<AstroActuator> actuator(new AstroActuator(Astro_ActuatorType_Generic, 0));
    AstroActivationHandle forward(actuator, Astro_DirectionMode_Forward, 0.4f);
    AstroActivationHandle reverse(actuator, Astro_DirectionMode_Reverse, 0.8f);
    actuator->setEnableMode(Astro_EnableMode_Highest);
    actuator->update();
    check(isFPEqual(actuator->getPower(), 0.4f), "highest mode uses signed highest request");
    actuator->setEnableMode(Astro_EnableMode_Lowest);
    actuator->update();
    check(isFPEqual(actuator->getPower(), -0.8f), "lowest mode uses signed lowest request");

    AstroLogger logger;
    LogState logState;
    MethodSlot<LogState, const AstroLogEvent> logSlot(&logState, &LogState::handle);
    logger.getLogSignal().attach(logSlot);
    logger.setLogLevel(Astro_LogLevel_Errors);
    logger.logMessage(1, "hidden");
    logger.logWarning(2, "hidden");
    logger.logError(3, "shown");
    check(logState.count == 1 && logState.lastLevel == Astro_LogLevel_Errors,
          "logger signal respects configured filtering");

    AstroPublisher publisher;
    PublishState publishState;
    MethodSlot<PublishState, Pair<uint8_t, const AstroDataColumn *> > publishSlot(&publishState, &PublishState::handle);
    publisher.getPublishSignal().attach(publishSlot);
    check(publisher.addColumn(1) && publisher.addColumn(2), "publisher accepts unique columns");
    check(publisher.publishData(1, 10.0, Astro_UnitsType_Temperature_Celsius, 5, 100),
          "publisher accepts first frame value");
    check(publishState.count == 0, "publisher waits for complete frame");
    check(publisher.publishData(2, 20.0, Astro_UnitsType_Temperature_Celsius, 5, 100),
          "publisher accepts second frame value");
    check(publishState.count == 1 && publishState.columns == 2,
          "publisher emits complete polling frame");

    std::cout << "PASS automation" << std::endl;
    return 0;
}
