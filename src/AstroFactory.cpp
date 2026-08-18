/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Factory
*/

#include "AstroFactory.h"

AstroActuator *AstroFactory::newActuator(Astro_ActuatorType actuatorType, aposi_t positionIndex)
{
    if (actuatorType < Astro_ActuatorType_MountAxis || actuatorType >= Astro_ActuatorType_Count) { return nullptr; }
    return new AstroActuator(actuatorType, positionIndex);
}

AstroValueSensor *AstroFactory::newSensor(Astro_SensorType sensorType, Astro_UnitsType units, aposi_t positionIndex)
{
    if (sensorType < Astro_SensorType_Temperature || sensorType >= Astro_SensorType_Count) { return nullptr; }
    return new AstroValueSensor(sensorType, units, positionIndex);
}

AstroDigitalActuator *AstroFactory::newDigitalActuator(Astro_ActuatorType actuatorType, AstroDigitalPin outputPin,
                                                       aposi_t positionIndex)
{
    if (actuatorType < Astro_ActuatorType_MountAxis || actuatorType >= Astro_ActuatorType_Count) { return nullptr; }
    return new AstroDigitalActuator(outputPin, actuatorType, positionIndex);
}

AstroAnalogActuator *AstroFactory::newAnalogActuator(Astro_ActuatorType actuatorType, AstroAnalogPin outputPin,
                                                     aposi_t positionIndex)
{
    if (actuatorType < Astro_ActuatorType_MountAxis || actuatorType >= Astro_ActuatorType_Count) { return nullptr; }
    return new AstroAnalogActuator(outputPin, actuatorType, positionIndex);
}

AstroDigitalSensor *AstroFactory::newDigitalSensor(Astro_SensorType sensorType, AstroDigitalPin inputPin,
                                                   aposi_t positionIndex)
{
    if (sensorType < Astro_SensorType_Temperature || sensorType >= Astro_SensorType_Count) { return nullptr; }
    return new AstroDigitalSensor(inputPin, sensorType, positionIndex);
}

AstroAnalogSensor *AstroFactory::newAnalogSensor(Astro_SensorType sensorType, AstroAnalogPin inputPin,
                                                 Astro_UnitsType units, aposi_t positionIndex)
{
    if (sensorType < Astro_SensorType_Temperature || sensorType >= Astro_SensorType_Count) { return nullptr; }
    return new AstroAnalogSensor(inputPin, sensorType, units, positionIndex);
}

AstroDigitalActuator *AstroFactory::newDewHeaterRelay(pintype_t outputPin, bool activeLow, aposi_t positionIndex)
{
    return newDigitalActuator(Astro_ActuatorType_DewHeater,
                              AstroDigitalPin(outputPin, Astro_PinMode_Digital_Output, activeLow),
                              positionIndex);
}

AstroAnalogActuator *AstroFactory::newDewHeaterPWM(pintype_t outputPin, uint8_t outputBitRes, aposi_t positionIndex)
{
    return newAnalogActuator(Astro_ActuatorType_DewHeater,
                             AstroAnalogPin(outputPin, Astro_PinMode_Analog_Output, outputBitRes),
                             positionIndex);
}

AstroDigitalActuator *AstroFactory::newCoverRelay(pintype_t outputPin, bool activeLow, aposi_t positionIndex)
{
    return newDigitalActuator(Astro_ActuatorType_Cover,
                              AstroDigitalPin(outputPin, Astro_PinMode_Digital_Output, activeLow),
                              positionIndex);
}

AstroAnalogActuator *AstroFactory::newCameraCoolerPWM(pintype_t outputPin, uint8_t outputBitRes, aposi_t positionIndex)
{
    return newAnalogActuator(Astro_ActuatorType_CameraCooler,
                             AstroAnalogPin(outputPin, Astro_PinMode_Analog_Output, outputBitRes),
                             positionIndex);
}

AstroAnalogActuator *AstroFactory::newFanPWM(pintype_t outputPin, uint8_t outputBitRes, aposi_t positionIndex)
{
    return newAnalogActuator(Astro_ActuatorType_Fan,
                             AstroAnalogPin(outputPin, Astro_PinMode_Analog_Output, outputBitRes),
                             positionIndex);
}

AstroServoAxisDriver *AstroFactory::newMountAxisServo(pintype_t outputPin, double minDegrees, double maxDegrees, uint8_t outputBitRes)
{
    return newServoAxisDriver(AstroAnalogPin(outputPin, Astro_PinMode_Analog_Output, outputBitRes), minDegrees, maxDegrees);
}

AstroDigitalSensor *AstroFactory::newLimitSwitch(pintype_t inputPin, bool activeLow, aposi_t positionIndex)
{
    return newDigitalSensor(Astro_SensorType_LimitSwitch,
                            AstroDigitalPin(inputPin, Astro_PinMode_Digital_Input_PullUp, activeLow),
                            positionIndex);
}

AstroDigitalSensor *AstroFactory::newRainIndicator(pintype_t inputPin, bool activeLow, aposi_t positionIndex)
{
    return newDigitalSensor(Astro_SensorType_Rain,
                            AstroDigitalPin(inputPin, Astro_PinMode_Digital_Input_PullUp, activeLow),
                            positionIndex);
}

AstroAnalogSensor *AstroFactory::newLightSensor(pintype_t inputPin, uint8_t inputBitRes, aposi_t positionIndex)
{
    return newAnalogSensor(Astro_SensorType_Light, AstroAnalogPin(inputPin, Astro_PinMode_Analog_Input, inputBitRes),
                           Astro_UnitsType_Raw_1, positionIndex);
}

AstroAnalogSensor *AstroFactory::newTemperatureSensor(pintype_t inputPin, uint8_t inputBitRes, aposi_t positionIndex)
{
    return newAnalogSensor(Astro_SensorType_Temperature, AstroAnalogPin(inputPin, Astro_PinMode_Analog_Input, inputBitRes),
                           Astro_UnitsType_Raw_1, positionIndex);
}

AstroAnalogSensor *AstroFactory::newPositionSensor(pintype_t inputPin, uint8_t inputBitRes, aposi_t positionIndex)
{
    return newAnalogSensor(Astro_SensorType_Position, AstroAnalogPin(inputPin, Astro_PinMode_Analog_Input, inputBitRes),
                           Astro_UnitsType_Raw_1, positionIndex);
}

AstroMount *AstroFactory::newMount(Astro_MountType mountType, aposi_t positionIndex)
{
    if (mountType < Astro_MountType_Equatorial || mountType >= Astro_MountType_Count) { return nullptr; }
    return new AstroMount(mountType, positionIndex);
}

AstroRail *AstroFactory::newRail(Astro_RailType railType, aposi_t positionIndex)
{
    if (railType < Astro_RailType_DC3V3 || railType >= Astro_RailType_Count) { return nullptr; }

    const double voltage = railType == Astro_RailType_DC3V3 ? 3.3 :
                           railType == Astro_RailType_DC5V ? 5.0 :
                           railType == Astro_RailType_DC12V ? 12.0 : 24.0;
    return new AstroRail(railType, voltage, 0.0, positionIndex);
}

AstroCover *AstroFactory::newCover(aposi_t positionIndex)
{
    return new AstroCover(positionIndex);
}

AstroCameraTrigger *AstroFactory::newCameraTrigger(aposi_t positionIndex)
{
    return new AstroCameraTrigger(nullptr, nullptr, positionIndex);
}

AstroCallbackAxisDriver *AstroFactory::newCallbackAxisDriver(AstroCallbackAxisDriver::TargetCallback targetCallback,
                                                             AstroCallbackAxisDriver::StopCallback stopCallback,
                                                             void *context)
{
    return new AstroCallbackAxisDriver(targetCallback, stopCallback, context);
}

AstroServoAxisDriver *AstroFactory::newServoAxisDriver(AstroAnalogPin outputPin, double minDegrees, double maxDegrees)
{
    return new AstroServoAxisDriver(outputPin, minDegrees, maxDegrees);
}

AstroThresholdTrigger *AstroFactory::newThresholdTrigger(double threshold, bool triggerBelow,
                                                         double tolerance, uint32_t stableTimeMs)
{
    return new AstroThresholdTrigger(threshold, triggerBelow, tolerance, stableTimeMs);
}

AstroRangeTrigger *AstroFactory::newRangeTrigger(double low, double high, bool triggerOutside,
                                                 double tolerance, uint32_t stableTimeMs)
{
    return new AstroRangeTrigger(low, high, triggerOutside, tolerance, stableTimeMs);
}

AstroObject *AstroFactory::newObjectFromData(const AstroObjectData *dataIn)
{
    if (!dataIn) { return nullptr; }

    switch (dataIn->idType) {
        case AstroIdentity::Actuator: return new AstroActuator(dataIn);
        case AstroIdentity::Sensor: return new AstroValueSensor(dataIn);
        case AstroIdentity::Mount: return new AstroMount(dataIn);
        case AstroIdentity::Rail: return new AstroRail(dataIn);
        case AstroIdentity::Cover: return new AstroCover(dataIn);
        case AstroIdentity::ObservationDevice: return new AstroCameraTrigger(dataIn);
        default: return nullptr;
    }
}

AstroPin *AstroFactory::newPinFromData(const AstroPinData *dataIn)
{
    return newPinObjectFromSubData(dataIn);
}
