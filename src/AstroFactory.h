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
// Centralizes common object creation so user sketches and deserialization code follow
// the same construction rules. Constructors remain public for advanced users, while
// these helpers provide the same convenience layer used by the sibling libraries.
class AstroFactory {
public:
    // Generic actuator and sensor creation.
    static AstroActuator *newActuator(Astro_ActuatorType actuatorType,
                                      aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index
    static AstroValueSensor *newSensor(Astro_SensorType sensorType,
                                       Astro_UnitsType units = Astro_UnitsType_Undefined,
                                       aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index

    // Pin-backed actuator creation.
    static AstroDigitalActuator *newDigitalActuator(Astro_ActuatorType actuatorType,
                                                     AstroDigitalPin outputPin,
                                                     aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index
    static AstroAnalogActuator *newAnalogActuator(Astro_ActuatorType actuatorType,
                                                   AstroAnalogPin outputPin,
                                                   aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index

    // Pin-backed sensor creation.
    static AstroDigitalSensor *newDigitalSensor(Astro_SensorType sensorType,
                                                 AstroDigitalPin inputPin,
                                                 aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index
    static AstroAnalogSensor *newAnalogSensor(Astro_SensorType sensorType,
                                               AstroAnalogPin inputPin,
                                               Astro_UnitsType units = Astro_UnitsType_Raw_1,
                                               aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index

    // Convenience builders for common Astruino equipment.

    // Creates a binary dew-heater output, typically driving a relay or MOSFET gate.
    static AstroDigitalActuator *newDewHeaterRelay(pintype_t outputPin,
                                                    bool activeLow = false,
                                                    aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    // Creates a variable/PWM dew-heater output for proportional dew control.
    static AstroAnalogActuator *newDewHeaterPWM(pintype_t outputPin,
                                                 uint8_t outputBitRes = 8,
                                                 aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    // Creates a binary cover motor/relay output. Directional mechanisms can use two outputs or a callback actuator.
    static AstroDigitalActuator *newCoverRelay(pintype_t outputPin,
                                                bool activeLow = false,
                                                aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    // Creates a variable camera-cooler output suitable for a TEC controller input.
    static AstroAnalogActuator *newCameraCoolerPWM(pintype_t outputPin,
                                                    uint8_t outputBitRes = 8,
                                                    aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    // Creates a variable equipment/camera fan output.
    static AstroAnalogActuator *newFanPWM(pintype_t outputPin,
                                          uint8_t outputBitRes = 8,
                                          aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    // Creates a hobby-servo axis driver with the supplied angular travel range.
    static AstroServoAxisDriver *newMountAxisServo(pintype_t outputPin,
                                                    double minDegrees = 0.0,
                                                    double maxDegrees = 180.0,
                                                    uint8_t outputBitRes = 8);

    // Creates a digital mount/cover endstop or home switch input.
    static AstroDigitalSensor *newLimitSwitch(pintype_t inputPin,
                                               bool activeLow = true,
                                               aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    // Creates a digital rain/wet indicator input for safe-stow logic.
    static AstroDigitalSensor *newRainIndicator(pintype_t inputPin,
                                                 bool activeLow = true,
                                                 aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    // Creates a normalized analog light sensor useful for twilight or balancing experiments.
    static AstroAnalogSensor *newLightSensor(pintype_t inputPin,
                                              uint8_t inputBitRes = 10,
                                              aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    // Creates a normalized analog temperature sensor ready for user calibration.
    static AstroAnalogSensor *newTemperatureSensor(pintype_t inputPin,
                                                    uint8_t inputBitRes = 10,
                                                    aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    // Creates a normalized analog position/feedback sensor ready for user calibration.
    static AstroAnalogSensor *newPositionSensor(pintype_t inputPin,
                                                 uint8_t inputBitRes = 10,
                                                 aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);

    // Mechanical/system object creation.
    static AstroMount *newMount(Astro_MountType mountType,
                                aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index
    static AstroRail *newRail(Astro_RailType railType,
                              aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index
    static AstroCover *newCover(aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    static AstroCameraTrigger *newCameraTrigger(aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);

    // Driver and trigger helpers.
    static AstroCallbackAxisDriver *newCallbackAxisDriver(AstroCallbackAxisDriver::TargetCallback targetCallback,
                                                           AstroCallbackAxisDriver::StopCallback stopCallback = nullptr,
                                                           void *context = nullptr); // Context, not owned
    static AstroServoAxisDriver *newServoAxisDriver(AstroAnalogPin outputPin,
                                                     double minDegrees = 0.0,
                                                     double maxDegrees = 180.0); // Max degrees
    static AstroThresholdTrigger *newThresholdTrigger(double threshold, bool triggerBelow = false,
                                                       double tolerance = 0.0, uint32_t stableTimeMs = 0); // Tolerance
    static AstroRangeTrigger *newRangeTrigger(double low, double high, bool triggerOutside = true,
                                               double tolerance = 0.0, uint32_t stableTimeMs = 0); // Tolerance

    // Creates a concrete object from serialized object data.
    static AstroObject *newObjectFromData(const AstroObjectData *dataIn);
    // Creates a pin implementation from serialized pin sub-data.
    static AstroPin *newPinFromData(const AstroPinData *dataIn);
};

#endif // /ifndef AstroFactory_H
