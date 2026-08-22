/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Factory
*/

#include "Astruino.h"

static aposi_t astroFirstOpen(AstroIdentity id)
{
    return getController() ? getController()->firstPositionOpen(id) : ASTRO_POS_SEARCH_FROMBEG;
}

template<class T>
static SharedPtr<T> astroRegister(SharedPtr<T> object)
{
    return object && getController() && getController()->registerObject(object) ? object : nullptr;
}

SharedPtr<AstroActuator> AstroFactory::addActuator(Astro_ActuatorType actuatorType)
{
    if (actuatorType < Astro_ActuatorType_MountAxis || actuatorType >= Astro_ActuatorType_Count) { return nullptr; }
    aposi_t positionIndex = astroFirstOpen(AstroIdentity(actuatorType));
    return isValidIndex(positionIndex) ? astroRegister(SharedPtr<AstroActuator>(new AstroActuator(actuatorType, positionIndex))) : nullptr;
}

SharedPtr<AstroCallbackActuator> AstroFactory::addCallbackActuator(AstroCallbackActuator::WriteCallback callback,
                                                                      Astro_ActuatorType actuatorType,
                                                                      void *context)
{
    aposi_t positionIndex = astroFirstOpen(AstroIdentity(actuatorType));
    return callback && isValidIndex(positionIndex)
        ? astroRegister(SharedPtr<AstroCallbackActuator>(new AstroCallbackActuator(callback, context, actuatorType, positionIndex)))
        : nullptr;
}

SharedPtr<AstroValueSensor> AstroFactory::addSensor(Astro_SensorType sensorType, Astro_UnitsType units)
{
    if (sensorType < Astro_SensorType_Temperature || sensorType >= Astro_SensorType_Count) { return nullptr; }
    aposi_t positionIndex = astroFirstOpen(AstroIdentity(sensorType));
    return isValidIndex(positionIndex) ? astroRegister(SharedPtr<AstroValueSensor>(new AstroValueSensor(sensorType, units, positionIndex))) : nullptr;
}

SharedPtr<AstroCallbackSensor> AstroFactory::addCallbackSensor(AstroCallbackSensor::ReadCallback callback,
                                                                  Astro_SensorType sensorType, Astro_UnitsType units,
                                                                  void *context)
{
    aposi_t positionIndex = astroFirstOpen(AstroIdentity(sensorType));
    return callback && isValidIndex(positionIndex)
        ? astroRegister(SharedPtr<AstroCallbackSensor>(new AstroCallbackSensor(callback, context, sensorType, units, positionIndex)))
        : nullptr;
}

SharedPtr<AstroDigitalActuator> AstroFactory::addDigitalActuator(Astro_ActuatorType actuatorType, AstroDigitalPin outputPin)
{
    aposi_t positionIndex = astroFirstOpen(AstroIdentity(actuatorType));
    return isValidIndex(positionIndex) ? astroRegister(SharedPtr<AstroDigitalActuator>(new AstroDigitalActuator(outputPin, actuatorType, positionIndex))) : nullptr;
}

SharedPtr<AstroRelayMotorActuator> AstroFactory::addRelayMotorActuator(Astro_ActuatorType actuatorType,
                                                                       AstroDigitalPin forwardPin,
                                                                       AstroDigitalPin reversePin)
{
    aposi_t positionIndex = astroFirstOpen(AstroIdentity(actuatorType));
    return isValidIndex(positionIndex)
        ? astroRegister(SharedPtr<AstroRelayMotorActuator>(new AstroRelayMotorActuator(forwardPin, reversePin, actuatorType, positionIndex)))
        : nullptr;
}

SharedPtr<AstroAnalogActuator> AstroFactory::addAnalogActuator(Astro_ActuatorType actuatorType, AstroAnalogPin outputPin)
{
    aposi_t positionIndex = astroFirstOpen(AstroIdentity(actuatorType));
    return isValidIndex(positionIndex) ? astroRegister(SharedPtr<AstroAnalogActuator>(new AstroAnalogActuator(outputPin, actuatorType, positionIndex))) : nullptr;
}

SharedPtr<AstroDigitalSensor> AstroFactory::addDigitalSensor(Astro_SensorType sensorType, AstroDigitalPin inputPin)
{
    aposi_t positionIndex = astroFirstOpen(AstroIdentity(sensorType));
    return isValidIndex(positionIndex) ? astroRegister(SharedPtr<AstroDigitalSensor>(new AstroDigitalSensor(inputPin, sensorType, positionIndex))) : nullptr;
}

SharedPtr<AstroAnalogSensor> AstroFactory::addAnalogSensor(Astro_SensorType sensorType, AstroAnalogPin inputPin, Astro_UnitsType units)
{
    aposi_t positionIndex = astroFirstOpen(AstroIdentity(sensorType));
    return isValidIndex(positionIndex) ? astroRegister(SharedPtr<AstroAnalogSensor>(new AstroAnalogSensor(inputPin, sensorType, units, positionIndex))) : nullptr;
}

SharedPtr<AstroDigitalActuator> AstroFactory::addDewHeaterRelay(pintype_t outputPin, bool activeLow)
{
    return addDigitalActuator(Astro_ActuatorType_DewHeater, AstroDigitalPin(outputPin, Astro_PinMode_Digital_Output, activeLow));
}

SharedPtr<AstroAnalogActuator> AstroFactory::addDewHeaterPWM(pintype_t outputPin, uint8_t outputBitRes)
{
    return addAnalogActuator(Astro_ActuatorType_DewHeater, AstroAnalogPin(outputPin, Astro_PinMode_Analog_Output, outputBitRes));
}

SharedPtr<AstroRelayMotorActuator> AstroFactory::addCoverMotorRelay(pintype_t forwardPin, pintype_t reversePin, bool activeLow)
{
    return addRelayMotorActuator(Astro_ActuatorType_Cover,
                                 AstroDigitalPin(forwardPin, Astro_PinMode_Digital_Output, activeLow),
                                 AstroDigitalPin(reversePin, Astro_PinMode_Digital_Output, activeLow));
}

SharedPtr<AstroAnalogActuator> AstroFactory::addCameraCoolerPWM(pintype_t outputPin, uint8_t outputBitRes)
{
    return addAnalogActuator(Astro_ActuatorType_CameraCooler, AstroAnalogPin(outputPin, Astro_PinMode_Analog_Output, outputBitRes));
}

SharedPtr<AstroAnalogActuator> AstroFactory::addFanPWM(pintype_t outputPin, uint8_t outputBitRes)
{
    return addAnalogActuator(Astro_ActuatorType_Fan, AstroAnalogPin(outputPin, Astro_PinMode_Analog_Output, outputBitRes));
}

SharedPtr<AstroFocuser> AstroFactory::addFocuser(int32_t maximumPosition)
{
    aposi_t positionIndex = astroFirstOpen(AstroIdentity(Astro_ActuatorType_Focuser));
    return isValidIndex(positionIndex) ? astroRegister(SharedPtr<AstroFocuser>(new AstroFocuser(maximumPosition, positionIndex))) : nullptr;
}

SharedPtr<AstroDigitalSensor> AstroFactory::addLimitSwitch(pintype_t inputPin, bool activeLow)
{
    return addDigitalSensor(Astro_SensorType_LimitSwitch, AstroDigitalPin(inputPin, Astro_PinMode_Digital_Input_PullUp, activeLow));
}

SharedPtr<AstroDigitalSensor> AstroFactory::addRainIndicator(pintype_t inputPin, bool activeLow)
{
    return addDigitalSensor(Astro_SensorType_Rain, AstroDigitalPin(inputPin, Astro_PinMode_Digital_Input_PullUp, activeLow));
}

SharedPtr<AstroAnalogSensor> AstroFactory::addLightSensor(pintype_t inputPin, uint8_t inputBitRes)
{
    return addAnalogSensor(Astro_SensorType_Light, AstroAnalogPin(inputPin, Astro_PinMode_Analog_Input, inputBitRes), Astro_UnitsType_Raw_1);
}

SharedPtr<AstroAnalogSensor> AstroFactory::addTemperatureSensor(pintype_t inputPin, uint8_t inputBitRes)
{
    return addAnalogSensor(Astro_SensorType_Temperature, AstroAnalogPin(inputPin, Astro_PinMode_Analog_Input, inputBitRes), Astro_UnitsType_Raw_1);
}

SharedPtr<AstroAnalogSensor> AstroFactory::addPositionSensor(pintype_t inputPin, uint8_t inputBitRes)
{
    return addAnalogSensor(Astro_SensorType_Position, AstroAnalogPin(inputPin, Astro_PinMode_Analog_Input, inputBitRes), Astro_UnitsType_Raw_1);
}

SharedPtr<AstroMount> AstroFactory::addMount(Astro_MountType mountType)
{
    aposi_t positionIndex = astroFirstOpen(AstroIdentity(mountType));
    return isValidIndex(positionIndex) ? astroRegister(SharedPtr<AstroMount>(new AstroMount(mountType, positionIndex))) : nullptr;
}

SharedPtr<AstroRail> AstroFactory::addRail(Astro_RailType railType)
{
    aposi_t positionIndex = astroFirstOpen(AstroIdentity(railType));
    if (!isValidIndex(positionIndex)) { return nullptr; }
    const double voltage = railType == Astro_RailType_DC3V3 ? 3.3 : railType == Astro_RailType_DC5V ? 5.0 :
                           railType == Astro_RailType_DC12V ? 12.0 : railType == Astro_RailType_DC24V ? 24.0 : 0.0;
    return voltage > 0.0 ? astroRegister(SharedPtr<AstroRail>(new AstroRail(railType, voltage, 0.0, positionIndex))) : nullptr;
}

SharedPtr<AstroCover> AstroFactory::addCover()
{
    aposi_t positionIndex = astroFirstOpen(AstroIdentity(AstroIdentity::Cover, 0));
    return isValidIndex(positionIndex) ? astroRegister(SharedPtr<AstroCover>(new AstroCover(positionIndex))) : nullptr;
}

SharedPtr<AstroCameraTrigger> AstroFactory::addCameraTrigger()
{
    aposi_t positionIndex = astroFirstOpen(AstroIdentity(AstroIdentity::ObservationDevice, 0));
    return isValidIndex(positionIndex) ? astroRegister(SharedPtr<AstroCameraTrigger>(new AstroCameraTrigger(nullptr, nullptr, positionIndex))) : nullptr;
}

SharedPtr<AstroCallbackAxisDriver> AstroFactory::addCallbackAxisDriver(AstroCallbackAxisDriver::TargetCallback targetCallback,
                                                                       AstroCallbackAxisDriver::StopCallback stopCallback,
                                                                       void *context)
{
    return SharedPtr<AstroCallbackAxisDriver>(new AstroCallbackAxisDriver(targetCallback, stopCallback, context));
}

SharedPtr<AstroServoAxisDriver> AstroFactory::addServoAxisDriver(AstroAnalogPin outputPin, double minDegrees, double maxDegrees)
{
    return SharedPtr<AstroServoAxisDriver>(new AstroServoAxisDriver(outputPin, minDegrees, maxDegrees));
}

SharedPtr<AstroServoAxisDriver> AstroFactory::addMountAxisServo(pintype_t outputPin, double minDegrees, double maxDegrees, uint8_t outputBitRes)
{
    return addServoAxisDriver(AstroAnalogPin(outputPin, Astro_PinMode_Analog_Output, outputBitRes), minDegrees, maxDegrees);
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

AstroObject *AstroFactory::newObjectFromData(const AstroObjectData *dataIn)
{
    if (!dataIn || !dataIn->isObjectData()) { return nullptr; }

    switch (dataIn->id.object.idType) {
        case AstroIdentity::Actuator: {
            const AstroActuatorData *actuatorData = static_cast<const AstroActuatorData *>(dataIn);
            switch (dataIn->id.object.classType) {
                case AstroActuator::Digital: return new AstroDigitalActuator(actuatorData);
                case AstroActuator::RelayMotor: return new AstroRelayMotorActuator(actuatorData);
                case AstroActuator::Analog: return new AstroAnalogActuator(actuatorData);
                case AstroActuator::Focuser: return new AstroFocuser(actuatorData);
                case AstroActuator::Base: return new AstroActuator(actuatorData);
                default: return nullptr;
            }
        }
        case AstroIdentity::Sensor: {
            const AstroSensorData *sensorData = static_cast<const AstroSensorData *>(dataIn);
            switch (dataIn->id.object.classType) {
                case AstroSensor::Value: return new AstroValueSensor(sensorData);
                case AstroSensor::Digital: return new AstroDigitalSensor(sensorData);
                case AstroSensor::Analog: return new AstroAnalogSensor(sensorData);
                default: return nullptr;
            }
        }
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
