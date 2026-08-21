/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Factory
*/

#ifndef AstroFactory_H
#define AstroFactory_H

#include "AstroCamera.h"
#include "AstroCover.h"
#include "AstroDrivers.h"
#include "AstroMounts.h"
#include "AstroRails.h"
#include "AstroSensors.h"
#include "AstroTriggers.h"

// Object Factory
// Centralizes common object creation so application sketches and deserialization code follow
// the same construction rules. Objects created through these helpers are registered with the
// controller, matching the shared-object lifecycle used by the sibling libraries.
class AstroFactory {
public:
    // Generic actuator and sensor creation.
    SharedPtr<AstroActuator> addActuator(Astro_ActuatorType actuatorType);
    SharedPtr<AstroValueSensor> addSensor(Astro_SensorType sensorType,
                                          Astro_UnitsType units = Astro_UnitsType_Undefined);
    SharedPtr<AstroCallbackActuator> addCallbackActuator(AstroCallbackActuator::WriteCallback callback,
                                                         Astro_ActuatorType actuatorType = Astro_ActuatorType_Generic,
                                                         void *context = nullptr); // Context, not owned
    SharedPtr<AstroCallbackSensor> addCallbackSensor(AstroCallbackSensor::ReadCallback callback,
                                                     Astro_SensorType sensorType = Astro_SensorType_Generic,
                                                     Astro_UnitsType units = Astro_UnitsType_Undefined,
                                                     void *context = nullptr); // Context, not owned

    // Pin-backed actuator creation.
    SharedPtr<AstroDigitalActuator> addDigitalActuator(Astro_ActuatorType actuatorType,
                                                       AstroDigitalPin outputPin);
    SharedPtr<AstroAnalogActuator> addAnalogActuator(Astro_ActuatorType actuatorType,
                                                     AstroAnalogPin outputPin);

    // Pin-backed sensor creation.
    SharedPtr<AstroDigitalSensor> addDigitalSensor(Astro_SensorType sensorType,
                                                   AstroDigitalPin inputPin);
    SharedPtr<AstroAnalogSensor> addAnalogSensor(Astro_SensorType sensorType,
                                                 AstroAnalogPin inputPin,
                                                 Astro_UnitsType units = Astro_UnitsType_Raw_1);

    // Convenience builders for common Astruino equipment.

    // Creates a binary dew-heater output, typically driving a relay or MOSFET gate.
    SharedPtr<AstroDigitalActuator> addDewHeaterRelay(pintype_t outputPin,
                                                      bool activeLow = false);
    // Creates a variable/PWM dew-heater output for proportional dew control.
    SharedPtr<AstroAnalogActuator> addDewHeaterPWM(pintype_t outputPin,
                                                   uint8_t outputBitRes = 8);
    // Creates a binary cover motor/relay output. Directional mechanisms can use two outputs or a callback actuator.
    SharedPtr<AstroDigitalActuator> addCoverRelay(pintype_t outputPin,
                                                  bool activeLow = false);
    // Creates a variable camera-cooler output suitable for a TEC controller input.
    SharedPtr<AstroAnalogActuator> addCameraCoolerPWM(pintype_t outputPin,
                                                      uint8_t outputBitRes = 8);
    // Creates a variable equipment/camera fan output.
    SharedPtr<AstroAnalogActuator> addFanPWM(pintype_t outputPin,
                                            uint8_t outputBitRes = 8);
    // Creates a hobby-servo axis driver with the supplied angular travel range.
    SharedPtr<AstroServoAxisDriver> addMountAxisServo(pintype_t outputPin,
                                                      double minDegrees = 0.0,
                                                      double maxDegrees = 180.0,
                                                      uint8_t outputBitRes = 8);
    // Creates an absolute-step telescope focuser ready for a stepper or external driver callback.
    SharedPtr<AstroFocuser> addFocuser(int32_t maximumPosition = 10000);

    // Creates a digital mount/cover endstop or home switch input.
    SharedPtr<AstroDigitalSensor> addLimitSwitch(pintype_t inputPin,
                                                 bool activeLow = true);
    // Creates a digital rain/wet indicator input for safe-stow logic.
    SharedPtr<AstroDigitalSensor> addRainIndicator(pintype_t inputPin,
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

    // Mechanical/system object creation.
    SharedPtr<AstroMount> addMount(Astro_MountType mountType);
    SharedPtr<AstroRail> addRail(Astro_RailType railType);
    SharedPtr<AstroCover> addCover();
    SharedPtr<AstroCameraTrigger> addCameraTrigger();

    // Driver and trigger helpers.
    SharedPtr<AstroCallbackAxisDriver> addCallbackAxisDriver(AstroCallbackAxisDriver::TargetCallback targetCallback,
                                                             AstroCallbackAxisDriver::StopCallback stopCallback = nullptr,
                                                             void *context = nullptr); // Context, not owned
    SharedPtr<AstroServoAxisDriver> addServoAxisDriver(AstroAnalogPin outputPin,
                                                       double minDegrees = 0.0,
                                                       double maxDegrees = 180.0); // Max degrees
    SharedPtr<AstroThresholdTrigger> addThresholdTrigger(SharedPtr<AstroSensor> sensor,
                                                         double threshold, bool triggerBelow = false,
                                                         double tolerance = 0.0, uint32_t stableTimeMs = 0); // Tolerance
    SharedPtr<AstroRangeTrigger> addRangeTrigger(SharedPtr<AstroSensor> sensor,
                                                 double low, double high, bool triggerOutside = true,
                                                 double tolerance = 0.0, uint32_t stableTimeMs = 0); // Tolerance

    // Creates a concrete object from serialized object data (return ownership transfer - user code *must* delete returned object).
    static AstroObject *newObjectFromData(const AstroObjectData *dataIn);
    // Creates a pin implementation from serialized pin sub-data (return ownership transfer - user code *must* delete returned object).
    static AstroPin *newPinFromData(const AstroPinData *dataIn);
};

#endif // /ifndef AstroFactory_H
