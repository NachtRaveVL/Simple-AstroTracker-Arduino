#include "Astruino.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

static void check(bool condition, const char *message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << std::endl;
        std::exit(1);
    }
}

int main()
{
    AstroRelayMotorActuator motor(
        AstroDigitalPin(2, Astro_PinMode_Digital_Output, false),
        AstroDigitalPin(3, Astro_PinMode_Digital_Output, false),
        Astro_ActuatorType_Cover, 0);

    motor.setPower(1.0f);
    check(astroGetHostDigitalPin(2) == HIGH && astroGetHostDigitalPin(3) == LOW,
          "relay motor drives forward output");
    motor.setPower(-1.0f);
    check(astroGetHostDigitalPin(2) == LOW && astroGetHostDigitalPin(3) == HIGH,
          "relay motor drives reverse output");
    motor.setPower(0.0f);
    check(astroGetHostDigitalPin(2) == LOW && astroGetHostDigitalPin(3) == LOW,
          "relay motor releases both outputs");

    AstroAnalogSensor temperature(
        AstroAnalogPin(4, Astro_PinMode_Analog_Input, 10),
        Astro_SensorType_Temperature, Astro_UnitsType_Raw_1, 0);
    AstroCalibrationData temperatureCalibration(temperature.getId(), Astro_UnitsType_Temperature_Celsius);
    temperatureCalibration.setFromTwoPoints(0.0, -20.0, 1.0, 80.0);
    temperature.setUserCalibrationData(&temperatureCalibration);
    check(temperature.getUserCalibrationData() == &temperatureCalibration,
          "analog sensor accepts user calibration data");
    astroSetHostAnalogPin(4, 512);
    check(temperature.poll(100, 1), "calibrated analog sensor polls");
    check(temperature.getMeasurement().value > 29.9 && temperature.getMeasurement().value < 30.2,
          "analog sensor calibration is applied once");
    check(temperature.getMeasurement().units == Astro_UnitsType_Temperature_Celsius,
          "calibrated analog sensor reports engineering units");


    auto arbitrationActuator = SharedPtr<AstroActuator>(new AstroActuator(Astro_ActuatorType_Generic, 1));
    AstroActivationHandle forwardRequest(arbitrationActuator, Astro_DirectionMode_Forward, 0.4f);
    AstroActivationHandle reverseRequest(arbitrationActuator, Astro_DirectionMode_Reverse, 0.8f);
    arbitrationActuator->setEnableMode(Astro_EnableMode_Highest);
    arbitrationActuator->update();
    check(isFPEqual(arbitrationActuator->getPower(), 0.4f),
          "highest activation mode compares signed drive intensity like Hydro/Helio");
    arbitrationActuator->setEnableMode(Astro_EnableMode_Lowest);
    arbitrationActuator->update();
    check(isFPEqual(arbitrationActuator->getPower(), -0.8f),
          "lowest activation mode compares signed drive intensity like Hydro/Helio");
    arbitrationActuator->setEnableMode(Astro_EnableMode_InOrder);
    arbitrationActuator->update();
    check(isFPEqual(arbitrationActuator->getPower(), 0.4f),
          "in-order activation mode selects first request");
    forwardRequest.unset();
    arbitrationActuator->update();
    check(isFPEqual(arbitrationActuator->getPower(), -0.8f),
          "in-order activation mode advances after first request is removed");

    auto timedActuator = SharedPtr<AstroActuator>(new AstroActuator(Astro_ActuatorType_Generic, 2));
    timedActuator->setEnableMode(Astro_EnableMode_InOrder);
    AstroActivationHandle timedFirst(timedActuator, Astro_DirectionMode_Forward, 0.5f, 100);
    AstroActivationHandle timedSecond(timedActuator, Astro_DirectionMode_Forward, 0.7f, 100);
    timedActuator->update();
    check(timedFirst.isActive() && !timedSecond.isActive(),
          "serial activation only starts the selected handle");
    millis_t waitingDuration = timedSecond.getTimeLeft();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    timedActuator->update();
    check(timedSecond.getTimeLeft() == waitingDuration,
          "waiting serial activation does not consume duration");

    AstroStepDirAxisDriver stepper(
        AstroDigitalPin(5, Astro_PinMode_Digital_Output, false),
        AstroDigitalPin(6, Astro_PinMode_Digital_Output, false),
        AstroDigitalPin(7, Astro_PinMode_Digital_Output, true),
        10.0, 10000.0);
    stepper.setPositionDegrees(0.0);
    stepper.setTargetDegrees(1.0);
    stepper.update();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    stepper.update();
    double positionDegrees = 0.0;
    check(stepper.getPositionDegrees(&positionDegrees), "stepper reports axis position");
    check(positionDegrees > 0.0 && positionDegrees <= 1.0, "stepper advances toward target");
    check(astroGetHostDigitalPin(6) == HIGH, "stepper forward target selects forward direction");

    Astruino controller;
    controller.init();
    auto controllerTemperature = controller.addTemperatureSensor(13, 10);
    check(controllerTemperature != nullptr, "factory creates controller temperature sensor");
    AstroCalibrationData controllerCalibration(controllerTemperature->getId(), Astro_UnitsType_Temperature_Celsius);
    controllerCalibration.setFromTwoPoints(0.0, -20.0, 1.0, 80.0);
    controllerTemperature->setUserCalibrationData(&controllerCalibration);
    check(controllerTemperature->getUserCalibrationData() != nullptr &&
          controllerTemperature->getUserCalibrationData() != &controllerCalibration,
          "controller owns copied calibration data like Hydro/Helio");
    check(controller.getUserCalibrationData(controllerTemperature->getKey()) == controllerTemperature->getUserCalibrationData(),
          "sensor calibration resolves through controller calibration store");
    auto coverMotor = controller.addCoverMotorRelay(8, 9);
    check(coverMotor != nullptr, "factory creates cover relay motor");
    controller.getCover().setActuator(coverMotor);
    controller.getCover().setPosition(0.5f);
    controller.getCover().open();
    controller.getCover().update();
    check(astroGetHostDigitalPin(8) == HIGH && astroGetHostDigitalPin(9) == LOW,
          "cover open command drives forward relay");
    controller.getCover().close();
    controller.getCover().update();
    check(astroGetHostDigitalPin(8) == LOW && astroGetHostDigitalPin(9) == HIGH,
          "cover close command drives reverse relay");

    auto mountStepper = controller.addMountAxisStepper(10, 11, 12, 100.0, 2000.0);
    check(mountStepper != nullptr, "factory creates STEP/DIR mount driver");
    controller.getMount().setAxisDriver(0, mountStepper);
    check(controller.getMount().getAxisDriver(0) == mountStepper, "mount keeps axis-driver attachment");

    std::cout << "PASS Astruino hardware" << std::endl;
    return 0;
}
