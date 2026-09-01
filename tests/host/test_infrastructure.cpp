#include "Astruino.h"
#include <cmath>
#include <cstring>
#include <iostream>

#define CHECK(cond) do { if (!(cond)) { std::cerr << "FAIL line " << __LINE__ << ": " #cond << '\n'; return 1; } } while (0)

static bool nearly(double a, double b, double tolerance = 1.0e-6) { return std::fabs(a - b) <= tolerance; }
static int signalValue = 0;
static void captureSignal(int value) { signalValue = value; }

int main()
{
    CHECK(SFP(AStr_Tracking) == String("Tracking"));
    CHECK(std::strcmp(CFP(AStr_Tracking), "Tracking") == 0);

    AstroIdentity sensorId(Astro_SensorType_Temperature, 2);
    AstroIdentity sensorIdAgain(Astro_SensorType_Temperature, 2);
    AstroIdentity otherSensor(Astro_SensorType_Humidity, 2);
    CHECK(sensorId == sensorIdAgain);
    CHECK(sensorId != otherSensor);
    CHECK(sensorId.keyString == String("Temperature #2"));

    AstroObject objectA(sensorId);
    AstroObject objectB(otherSensor);
    CHECK(objectA.addLinkage(&objectB));
    CHECK(objectA.hasLinkage(&objectB));
    CHECK(objectA.removeLinkage(&objectB));
    CHECK(!objectA.hasLinkage(&objectB));

    Signal<int, 2> signal;
    FunctionSlot<int> slot(captureSignal);
    signal.attach(slot);
    signal.fire(7);
    CHECK(signalValue == 7);
    signal.detach(slot);
    signal.fire(9);
    CHECK(signalValue == 7);

    AstroDigitalPin digital(7, Astro_PinMode_Digital_Output, false);
    digital.init();
    digital.activate();
    CHECK(astroGetHostDigitalPin(7) == HIGH);
    digital.deactivate();
    CHECK(astroGetHostDigitalPin(7) == LOW);

    AstroAnalogPin analog(3, Astro_PinMode_Analog_Output, 10);
    analog.analogWrite(0.5f);
    CHECK(std::abs(astroGetHostAnalogPin(3) - 512) <= 1);

    AstroPinData pinData;
    AstroDigitalPin(8, Astro_PinMode_Digital_Input_PullUp, true, -3).saveToData(&pinData);
    StaticJsonDocument<192> pinDoc;
    JsonObject pinObject = pinDoc.to<JsonObject>();
    pinData.toJSONObject(pinObject);
    AstroPinData pinRoundTrip;
    JsonObjectConst pinObjectConst = pinDoc.as<JsonObjectConst>();
    pinRoundTrip.fromJSONObject(pinObjectConst);
    CHECK(pinRoundTrip.pin == pinData.pin);
    CHECK(pinRoundTrip.mode == pinData.mode);
    CHECK(pinRoundTrip.channel == pinData.channel);
    CHECK(pinRoundTrip.activeLow == pinData.activeLow);
    AstroPin *pinFromData = newPinObjectFromSubData(&pinRoundTrip);
    CHECK(pinFromData != nullptr);
    CHECK(pinFromData->type == AstroPin::Digital);
    delete pinFromData;

    AstroSingleMeasurement temperature(0.0, Astro_UnitsType_Temperature_Celsius, 100, 4);
    CHECK(nearly(temperature.asUnits(Astro_UnitsType_Temperature_Fahrenheit).value, 32.0));

    SharedPtr<AstroValueSensor> sensor(new AstroValueSensor(Astro_SensorType_Temperature,
                                                            Astro_UnitsType_Temperature_Celsius, 0, 11.0));
    AstroMeasurementValueTrigger trigger(sensor, 10.0, false, 0, 1.0, 0);
    trigger.update();
    CHECK(trigger.getTriggerState() == Astro_TriggerState_Triggered);
    sensor->setValue(8.5);
    trigger.update();
    CHECK(trigger.getTriggerState() == Astro_TriggerState_NotTriggered);

    SharedPtr<AstroActuator> actuator(new AstroActuator(Astro_ActuatorType_Generic, 0));
    AstroActivationHandle forward(actuator, Astro_DirectionMode_Forward, 0.4f);
    AstroActivationHandle reverse(actuator, Astro_DirectionMode_Reverse, 0.8f);
    actuator->setEnableMode(Astro_EnableMode_Highest);
    actuator->update();
    CHECK(nearly(actuator->getPower(), 0.4));
    actuator->setEnableMode(Astro_EnableMode_Lowest);
    actuator->update();
    CHECK(nearly(actuator->getPower(), -0.8));

    Astruino controller;
    controller.init(Astro_SystemMode_Tracking, Astro_MeasurementMode_Metric);
    auto controllerTemperature = controller.addTemperatureSensor(13, 10);
    CHECK(controllerTemperature != nullptr);
    AstroCalibrationData calibration(controllerTemperature->getId(), Astro_UnitsType_Temperature_Celsius);
    calibration.setFromTwoPoints(0.0, -20.0, 1.0, 80.0);
    controllerTemperature->setUserCalibrationData(&calibration);
    CHECK(controllerTemperature->getUserCalibrationData() != nullptr);
    CHECK(controllerTemperature->getUserCalibrationData() != &calibration);
    CHECK(controller.getUserCalibrationData(controllerTemperature->getKey()) == controllerTemperature->getUserCalibrationData());

    std::cout << "Infrastructure tests passed\n";
    return 0;
}
