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
    AstroValueSensor temperature(Astro_SensorType_Temperature,
                                 Astro_UnitsType_Raw_1, 0, 0.5);
    AstroCalibrationData temperatureCalibration(temperature.getId(), Astro_UnitsType_Temperature_Celsius);
    temperatureCalibration.setFromTwoPoints(0.0, -20.0, 1.0, 80.0);
    temperature.setUserCalibrationData(&temperatureCalibration);
    check(temperature.getUserCalibrationData() == &temperatureCalibration,
          "value sensor accepts user calibration data without controller ownership");
    check(temperature.takeMeasurement(true), "calibrated value sensor measures");
    const AstroMeasurement *temperatureMeasurementBase = temperature.getMeasurement();
    const AstroSingleMeasurement *temperatureMeasurement = static_cast<const AstroSingleMeasurement *>(temperatureMeasurementBase);
    check(temperatureMeasurement->value > 29.9 && temperatureMeasurement->value < 30.2,
          "sensor calibration is applied once");
    check(temperatureMeasurement->units == Astro_UnitsType_Temperature_Celsius,
          "calibrated sensor reports engineering units");

    Astruino controller;
    controller.init();

    auto limitSwitch = controller.addLimitSwitch(14, true);
    check(limitSwitch != nullptr, "factory creates binary limit switch");
    check(limitSwitch->isBinaryClass() && !limitSwitch->isDigitalClass(),
          "limit switch uses binary sensor class instead of digital protocol class");

    auto motor = SharedPtr<AstroRelayMotorActuator>(new AstroRelayMotorActuator(
        AstroDigitalPin(2, Astro_PinMode_Digital_Output, false),
        AstroDigitalPin(3, Astro_PinMode_Digital_Output, false),
        Astro_ActuatorType_Cover, 0));

    AstroActivationHandle motorForward(motor, Astro_DirectionMode_Forward, 1.0f);
    motor->update();
    check(digitalRead(2) == HIGH && digitalRead(3) == LOW,
          "relay motor drives forward output");
    motorForward.unset();
    AstroActivationHandle motorReverse(motor, Astro_DirectionMode_Reverse, 1.0f);
    motor->update();
    check(digitalRead(2) == LOW && digitalRead(3) == HIGH,
          "relay motor drives reverse output");
    motorReverse.unset();
    motor->update();
    check(digitalRead(2) == LOW && digitalRead(3) == LOW,
          "relay motor releases both outputs");

    auto arbitrationActuator = SharedPtr<AstroActuator>(new AstroActuator(Astro_ActuatorType_Cover, 1));
    AstroActivationHandle forwardRequest(arbitrationActuator, Astro_DirectionMode_Forward, 0.4f);
    AstroActivationHandle reverseRequest(arbitrationActuator, Astro_DirectionMode_Reverse, 0.8f);
    arbitrationActuator->setEnableMode(Astro_EnableMode_Highest);
    arbitrationActuator->update();
    check(isFPEqual(arbitrationActuator->getDriveIntensity(), 0.4f),
          "highest activation mode compares signed drive intensity like Hydro/Helio");
    arbitrationActuator->setEnableMode(Astro_EnableMode_Lowest);
    arbitrationActuator->update();
    check(isFPEqual(arbitrationActuator->getDriveIntensity(), -0.8f),
          "lowest activation mode compares signed drive intensity like Hydro/Helio");
    arbitrationActuator->setEnableMode(Astro_EnableMode_InOrder);
    arbitrationActuator->update();
    check(isFPEqual(arbitrationActuator->getDriveIntensity(), 0.4f),
          "in-order activation mode selects first request");
    forwardRequest.unset();
    arbitrationActuator->update();
    check(isFPEqual(arbitrationActuator->getDriveIntensity(), -0.8f),
          "in-order activation mode advances after first request is removed");

    auto timedActuator = SharedPtr<AstroActuator>(new AstroActuator(Astro_ActuatorType_Cover, 2));
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
    check(digitalRead(6) == HIGH, "stepper forward target selects forward direction");

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
    auto mount = controller.addMount(Astro_MountType_Equatorial);
    check(mount != nullptr, "factory creates mount");
    auto coverMotor = controller.addCoverMotorRelay(8, 9);
    check(coverMotor != nullptr, "factory creates cover relay motor");
    mount->getMountCover().setActuator(coverMotor);
    mount->getMountCover().setPosition(0.5f);
    mount->getMountCover().open();
    mount->getMountCover().update();
    coverMotor->update();
    check(digitalRead(8) == HIGH && digitalRead(9) == LOW,
          "cover open command drives forward relay");
    mount->getMountCover().close();
    mount->getMountCover().update();
    coverMotor->update();
    check(digitalRead(8) == LOW && digitalRead(9) == HIGH,
          "cover close command drives reverse relay");

    auto mountStepper = controller.addMountAxisStepper(10, 11, 12, 100.0, 2000.0);
    check(mountStepper != nullptr, "factory creates STEP/DIR mount driver");
    mount->setAxisDriver(0, mountStepper);
    check(mount->getAxisDriver(0) == mountStepper, "mount keeps axis-driver attachment");

    std::cout << "PASS Astruino hardware" << std::endl;
    return 0;
}
