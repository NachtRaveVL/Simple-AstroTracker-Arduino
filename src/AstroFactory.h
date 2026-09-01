/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Factory
*/

#ifndef AstroFactory_H
#define AstroFactory_H

class AstroFactory;

#include "Astruino.h"

// Object Factory
// Contains convenience methods that create the supported Astruino object families,
// select the first available identity position, and register main objects with the controller.
class AstroFactory {
public:

    // Convenience builders for common actuators (shared, nullptr return -> failure).

    // Creates a binary dew-heater output, typically driving a relay or MOSFET gate.
    SharedPtr<AstroDigitalActuator> addDewHeaterRelay(pintype_t outputPin,
                                                      bool activeLow = false);
    // Creates a variable/PWM dew-heater output for proportional dew control.
    SharedPtr<AstroAnalogActuator> addDewHeaterPWM(pintype_t outputPin,
                                                   uint8_t outputBitRes = 8);
    // Creates a two-output forward/reverse cover motor using relays or an H-bridge input pair.
    SharedPtr<AstroRelayMotorActuator> addCoverMotorRelay(pintype_t forwardPin,
                                                          pintype_t reversePin,
                                                          bool activeLow = false);
    // Creates a binary camera-shutter output for a relay, optocoupler, or transistor interface.
    SharedPtr<AstroDigitalActuator> addCameraShutterRelay(pintype_t outputPin,
                                                          bool activeLow = false);
    // Creates a variable camera-cooler output suitable for a TEC controller input.
    SharedPtr<AstroAnalogActuator> addCameraCoolerPWM(pintype_t outputPin,
                                                      uint8_t outputBitRes = 8);
    // Creates a variable equipment/camera fan output.
    SharedPtr<AstroAnalogActuator> addFanPWM(pintype_t outputPin,
                                            uint8_t outputBitRes = 8);
    // Creates an absolute-step telescope focuser ready for a stepper or external driver callback.
    SharedPtr<AstroFocuser> addFocuser(int32_t maximumPosition = 10000);

    // Convenience builders for common sensors (shared, nullptr return -> failure).

    // Creates a binary mount/cover endstop or home switch input.
    SharedPtr<AstroBinarySensor> addLimitSwitch(pintype_t inputPin,
                                                 bool activeLow = true);
    // Creates a binary rain/wet indicator input for safe-stow logic.
    SharedPtr<AstroBinarySensor> addRainIndicator(pintype_t inputPin,
                                                   bool activeLow = true);
    // Creates a normalized analog light sensor useful for twilight or balancing experiments.
    SharedPtr<AstroAnalogSensor> addLightSensor(pintype_t inputPin,
                                                uint8_t inputBitRes = 10);
    // Creates a normalized analog temperature sensor ready for custom calibration.
    SharedPtr<AstroAnalogSensor> addTemperatureSensor(pintype_t inputPin,
                                                      uint8_t inputBitRes = 10);
    // Creates a normalized analog position/feedback sensor ready for custom calibration.
    SharedPtr<AstroAnalogSensor> addPositionSensor(pintype_t inputPin,
                                                   uint8_t inputBitRes = 10);

    // Convenience builders for system objects (shared, nullptr return -> failure).

    // Creates a catalog/ephemeris target object using the selected target type.
    SharedPtr<AstroTarget> addTarget(Astro_TargetType targetType);
    // Creates a mount object using the selected geometry.
    SharedPtr<AstroMount> addMount(Astro_MountType mountType);
    // Creates a simple count-limited power rail.
    SharedPtr<AstroSimpleRail> addSimplePowerRail(Astro_RailType railType,
                                                  int maxActiveAtOnce = 2);
    // Creates a sensor-regulated power rail.
    SharedPtr<AstroRegulatedRail> addRegulatedPowerRail(Astro_RailType railType,
                                                        float maxPower);

    // Driver and trigger helpers (sub-objects, not registered as main system objects).

    // Creates a callback-backed mount axis driver.
    SharedPtr<AstroCallbackAxisDriver> addCallbackAxisDriver(AstroCallbackAxisDriver::TargetCallback targetCallback,
                                                             AstroCallbackAxisDriver::StopCallback stopCallback = nullptr,
                                                             void *context = nullptr); // Context, not owned
    // Creates a hobby-servo axis driver with the supplied angular travel range.
    SharedPtr<AstroServoAxisDriver> addMountAxisServo(pintype_t outputPin,
                                                      double minDegrees = 0.0,
                                                      double maxDegrees = 180.0,
                                                      uint8_t outputBitRes = 8);
    // Creates a STEP/DIR telescope axis driver for common external stepper motor drivers.
    SharedPtr<AstroStepDirAxisDriver> addMountAxisStepper(pintype_t stepPin,
                                                          pintype_t directionPin,
                                                          pintype_t enablePin = apin_none,
                                                          double stepsPerDegree = 200.0,
                                                          double maxStepsPerSecond = 800.0,
                                                          bool reverseDirection = false,
                                                          bool enableActiveLow = true);
    // Creates a single-value measurement trigger.
    SharedPtr<AstroMeasurementValueTrigger> addThresholdTrigger(SharedPtr<AstroSensor> sensor,
                                                                 double threshold, bool triggerBelow = false,
                                                                 double detriggerTolerance = 0.0,
                                                                 millis_t detriggerDelay = 0);
    // Creates a measurement-range trigger.
    SharedPtr<AstroMeasurementRangeTrigger> addRangeTrigger(SharedPtr<AstroSensor> sensor,
                                                             double low, double high, bool triggerOutside = true,
                                                             double detriggerTolerance = 0.0,
                                                             millis_t detriggerDelay = 0);
};

#endif // /ifndef AstroFactory_H
