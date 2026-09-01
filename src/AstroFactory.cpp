/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Factory
*/

#include "Astruino.h"

SharedPtr<AstroDigitalActuator> AstroFactory::addDewHeaterRelay(pintype_t outputPin, bool activeLow)
{
    if (!getController()) { return nullptr; }
    aposi_t positionIndex = getController()->firstPositionOpen(AstroIdentity(Astro_ActuatorType_DewHeater));
    ASTRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(AStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto actuator = SharedPtr<AstroDigitalActuator>(new AstroDigitalActuator(
            AstroDigitalPin(outputPin, Astro_PinMode_Digital_Output, activeLow),
            Astro_ActuatorType_DewHeater, positionIndex));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<AstroAnalogActuator> AstroFactory::addDewHeaterPWM(pintype_t outputPin, uint8_t outputBitRes)
{
    if (!getController()) { return nullptr; }
    aposi_t positionIndex = getController()->firstPositionOpen(AstroIdentity(Astro_ActuatorType_DewHeater));
    ASTRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(AStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto actuator = SharedPtr<AstroAnalogActuator>(new AstroAnalogActuator(
            AstroAnalogPin(outputPin, Astro_PinMode_Analog_Output, outputBitRes),
            Astro_ActuatorType_DewHeater, positionIndex));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<AstroRelayMotorActuator> AstroFactory::addCoverMotorRelay(pintype_t forwardPin, pintype_t reversePin, bool activeLow)
{
    if (!getController()) { return nullptr; }
    aposi_t positionIndex = getController()->firstPositionOpen(AstroIdentity(Astro_ActuatorType_Cover));
    ASTRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(AStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto actuator = SharedPtr<AstroRelayMotorActuator>(new AstroRelayMotorActuator(
            AstroDigitalPin(forwardPin, Astro_PinMode_Digital_Output, activeLow),
            AstroDigitalPin(reversePin, Astro_PinMode_Digital_Output, activeLow),
            Astro_ActuatorType_Cover, positionIndex));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<AstroDigitalActuator> AstroFactory::addCameraShutterRelay(pintype_t outputPin, bool activeLow)
{
    if (!getController()) { return nullptr; }
    aposi_t positionIndex = getController()->firstPositionOpen(AstroIdentity(Astro_ActuatorType_CameraShutter));
    ASTRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(AStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto actuator = SharedPtr<AstroDigitalActuator>(new AstroDigitalActuator(
            AstroDigitalPin(outputPin, Astro_PinMode_Digital_Output, activeLow),
            Astro_ActuatorType_CameraShutter, positionIndex));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<AstroAnalogActuator> AstroFactory::addCameraCoolerPWM(pintype_t outputPin, uint8_t outputBitRes)
{
    if (!getController()) { return nullptr; }
    aposi_t positionIndex = getController()->firstPositionOpen(AstroIdentity(Astro_ActuatorType_CameraCooler));
    ASTRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(AStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto actuator = SharedPtr<AstroAnalogActuator>(new AstroAnalogActuator(
            AstroAnalogPin(outputPin, Astro_PinMode_Analog_Output, outputBitRes),
            Astro_ActuatorType_CameraCooler, positionIndex));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<AstroAnalogActuator> AstroFactory::addFanPWM(pintype_t outputPin, uint8_t outputBitRes)
{
    if (!getController()) { return nullptr; }
    aposi_t positionIndex = getController()->firstPositionOpen(AstroIdentity(Astro_ActuatorType_Fan));
    ASTRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(AStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto actuator = SharedPtr<AstroAnalogActuator>(new AstroAnalogActuator(
            AstroAnalogPin(outputPin, Astro_PinMode_Analog_Output, outputBitRes),
            Astro_ActuatorType_Fan, positionIndex));
        if (getController()->registerObject(actuator)) { return actuator; }
    }

    return nullptr;
}

SharedPtr<AstroFocuser> AstroFactory::addFocuser(int32_t maximumPosition)
{
    if (!getController()) { return nullptr; }
    aposi_t positionIndex = getController()->firstPositionOpen(AstroIdentity(Astro_ActuatorType_Focuser));
    ASTRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(AStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto focuser = SharedPtr<AstroFocuser>(new AstroFocuser(maximumPosition, positionIndex));
        if (getController()->registerObject(focuser)) { return focuser; }
    }

    return nullptr;
}

SharedPtr<AstroDigitalSensor> AstroFactory::addLimitSwitch(pintype_t inputPin, bool activeLow)
{
    if (!getController()) { return nullptr; }
    aposi_t positionIndex = getController()->firstPositionOpen(AstroIdentity(Astro_SensorType_LimitSwitch));
    ASTRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(AStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto sensor = SharedPtr<AstroDigitalSensor>(new AstroDigitalSensor(
            AstroDigitalPin(inputPin, Astro_PinMode_Digital_Input_PullUp, activeLow),
            Astro_SensorType_LimitSwitch, positionIndex));
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<AstroDigitalSensor> AstroFactory::addRainIndicator(pintype_t inputPin, bool activeLow)
{
    if (!getController()) { return nullptr; }
    aposi_t positionIndex = getController()->firstPositionOpen(AstroIdentity(Astro_SensorType_Rain));
    ASTRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(AStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto sensor = SharedPtr<AstroDigitalSensor>(new AstroDigitalSensor(
            AstroDigitalPin(inputPin, Astro_PinMode_Digital_Input_PullUp, activeLow),
            Astro_SensorType_Rain, positionIndex));
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<AstroAnalogSensor> AstroFactory::addLightSensor(pintype_t inputPin, uint8_t inputBitRes)
{
    if (!getController()) { return nullptr; }
    aposi_t positionIndex = getController()->firstPositionOpen(AstroIdentity(Astro_SensorType_Light));
    ASTRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(AStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto sensor = SharedPtr<AstroAnalogSensor>(new AstroAnalogSensor(
            AstroAnalogPin(inputPin, Astro_PinMode_Analog_Input, inputBitRes),
            Astro_SensorType_Light, Astro_UnitsType_Raw_1, positionIndex));
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<AstroAnalogSensor> AstroFactory::addTemperatureSensor(pintype_t inputPin, uint8_t inputBitRes)
{
    if (!getController()) { return nullptr; }
    aposi_t positionIndex = getController()->firstPositionOpen(AstroIdentity(Astro_SensorType_Temperature));
    ASTRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(AStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto sensor = SharedPtr<AstroAnalogSensor>(new AstroAnalogSensor(
            AstroAnalogPin(inputPin, Astro_PinMode_Analog_Input, inputBitRes),
            Astro_SensorType_Temperature, Astro_UnitsType_Raw_1, positionIndex));
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<AstroAnalogSensor> AstroFactory::addPositionSensor(pintype_t inputPin, uint8_t inputBitRes)
{
    if (!getController()) { return nullptr; }
    aposi_t positionIndex = getController()->firstPositionOpen(AstroIdentity(Astro_SensorType_Position));
    ASTRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(AStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto sensor = SharedPtr<AstroAnalogSensor>(new AstroAnalogSensor(
            AstroAnalogPin(inputPin, Astro_PinMode_Analog_Input, inputBitRes),
            Astro_SensorType_Position, Astro_UnitsType_Raw_1, positionIndex));
        if (getController()->registerObject(sensor)) { return sensor; }
    }

    return nullptr;
}

SharedPtr<AstroTarget> AstroFactory::addTarget(Astro_TargetType targetType)
{
    if (!getController()) { return nullptr; }
    aposi_t positionIndex = getController()->firstPositionOpen(AstroIdentity(targetType));
    ASTRO_SOFT_ASSERT((int)targetType >= 0 && targetType < Astro_TargetType_Count, SFP(AStr_Err_InvalidParameter));
    ASTRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(AStr_Err_NoPositionsAvailable));

    if ((int)targetType >= 0 && targetType < Astro_TargetType_Count && isValidIndex(positionIndex)) {
        SharedPtr<AstroTarget> target;
        if (targetType <= Astro_TargetType_Neptune) {
            target = SharedPtr<AstroTarget>(new AstroDynamicTarget(targetType, positionIndex));
        } else {
            target = SharedPtr<AstroTarget>(new AstroStaticTarget(targetType, positionIndex));
        }
        if (getController()->registerObject(target)) { return target; }
    }

    return nullptr;
}

SharedPtr<AstroMount> AstroFactory::addMount(Astro_MountType mountType)
{
    if (!getController()) { return nullptr; }
    aposi_t positionIndex = getController()->firstPositionOpen(AstroIdentity(mountType));
    ASTRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(AStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto mount = SharedPtr<AstroMount>(new AstroMount(mountType, positionIndex));
        if (getController()->registerObject(mount)) { return mount; }
    }

    return nullptr;
}

SharedPtr<AstroSimpleRail> AstroFactory::addSimplePowerRail(Astro_RailType railType, int maxActiveAtOnce)
{
    if (!getController()) { return nullptr; }
    aposi_t positionIndex = getController()->firstPositionOpen(AstroIdentity(railType));
    ASTRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(AStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto rail = SharedPtr<AstroSimpleRail>(new AstroSimpleRail(railType, positionIndex, maxActiveAtOnce));
        if (getController()->registerObject(rail)) { return rail; }
    }

    return nullptr;
}

SharedPtr<AstroRegulatedRail> AstroFactory::addRegulatedPowerRail(Astro_RailType railType, float maxPower)
{
    if (!getController()) { return nullptr; }
    aposi_t positionIndex = getController()->firstPositionOpen(AstroIdentity(railType));
    ASTRO_SOFT_ASSERT(isValidIndex(positionIndex), SFP(AStr_Err_NoPositionsAvailable));

    if (isValidIndex(positionIndex)) {
        auto rail = SharedPtr<AstroRegulatedRail>(new AstroRegulatedRail(railType, positionIndex, maxPower));
        if (getController()->registerObject(rail)) { return rail; }
    }

    return nullptr;
}

SharedPtr<AstroCallbackAxisDriver> AstroFactory::addCallbackAxisDriver(AstroCallbackAxisDriver::TargetCallback targetCallback,
                                                                       AstroCallbackAxisDriver::StopCallback stopCallback,
                                                                       void *context)
{
    return SharedPtr<AstroCallbackAxisDriver>(new AstroCallbackAxisDriver(targetCallback, stopCallback, context));
}

SharedPtr<AstroServoAxisDriver> AstroFactory::addMountAxisServo(pintype_t outputPin, double minDegrees, double maxDegrees, uint8_t outputBitRes)
{
    return SharedPtr<AstroServoAxisDriver>(new AstroServoAxisDriver(
        AstroAnalogPin(outputPin, Astro_PinMode_Analog_Output, outputBitRes), minDegrees, maxDegrees));
}

SharedPtr<AstroStepDirAxisDriver> AstroFactory::addMountAxisStepper(pintype_t stepPin, pintype_t directionPin,
                                                                    pintype_t enablePin, double stepsPerDegree,
                                                                    double maxStepsPerSecond, bool reverseDirection,
                                                                    bool enableActiveLow)
{
    return SharedPtr<AstroStepDirAxisDriver>(new AstroStepDirAxisDriver(
        AstroDigitalPin(stepPin, Astro_PinMode_Digital_Output, false),
        AstroDigitalPin(directionPin, Astro_PinMode_Digital_Output, reverseDirection),
        AstroDigitalPin(enablePin, Astro_PinMode_Digital_Output, enableActiveLow),
        stepsPerDegree, maxStepsPerSecond));
}

SharedPtr<AstroMeasurementValueTrigger> AstroFactory::addThresholdTrigger(SharedPtr<AstroSensor> sensor, double threshold,
                                                                          bool triggerBelow, double detriggerTolerance,
                                                                          millis_t detriggerDelay)
{
    return sensor ? SharedPtr<AstroMeasurementValueTrigger>(
        new AstroMeasurementValueTrigger(sensor, threshold, triggerBelow, 0, detriggerTolerance, detriggerDelay)) : nullptr;
}

SharedPtr<AstroMeasurementRangeTrigger> AstroFactory::addRangeTrigger(SharedPtr<AstroSensor> sensor, double low, double high,
                                                                      bool triggerOutside, double detriggerTolerance,
                                                                      millis_t detriggerDelay)
{
    return sensor ? SharedPtr<AstroMeasurementRangeTrigger>(
        new AstroMeasurementRangeTrigger(sensor, low, high, triggerOutside, 0, detriggerTolerance, detriggerDelay)) : nullptr;
}
