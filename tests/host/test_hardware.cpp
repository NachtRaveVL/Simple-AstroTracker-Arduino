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
    check(temperature.setCalibration(0.0, 1.0, -20.0, 80.0, Astro_UnitsType_Temperature_Celsius),
          "analog sensor accepts user calibration");
    astroSetHostAnalogPin(4, 512);
    check(temperature.poll(100, 1), "calibrated analog sensor polls");
    check(temperature.getMeasurement().value > 29.9 && temperature.getMeasurement().value < 30.2,
          "analog sensor calibration is applied once");
    check(temperature.getMeasurement().units == Astro_UnitsType_Temperature_Celsius,
          "calibrated analog sensor reports engineering units");

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
