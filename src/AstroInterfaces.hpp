/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Interface Inlines
*/

#ifndef AstroInterfaces_HPP
#define AstroInterfaces_HPP

#include "Astruino.h"

inline void AstroDistanceUnitsInterfaceStorage::setSpeedUnits(Astro_UnitsType speedUnits)
{
    setDistanceUnits(baseUnits(speedUnits));
}

inline Astro_UnitsType AstroDistanceUnitsInterfaceStorage::getSpeedUnits() const
{
    return rateUnits(getDistanceUnits());
}

inline Astro_UnitsType AstroMeasurementUnitsInterface::getRateUnits(uint8_t measurementRow) const
{
    return rateUnits(getMeasurementUnits(measurementRow));
}

inline Astro_UnitsType AstroMeasurementUnitsInterface::getBaseUnits(uint8_t measurementRow) const
{
    return baseUnits(getMeasurementUnits(measurementRow));
}


inline void AstroActuatorObjectInterface::setContinuousPowerUsage(float contPowerUsage, Astro_UnitsType contPowerUsageUnits)
{
    setContinuousPowerUsage(AstroSingleMeasurement(contPowerUsage, contPowerUsageUnits));
}


inline void AstroMotorObjectInterface::setContinuousSpeed(float contSpeed, Astro_UnitsType contSpeedUnits)
{
    setContinuousSpeed(AstroSingleMeasurement(contSpeed, contSpeedUnits));
}


template <class U>
inline void AstroParentActuatorAttachmentInterface::setParentActuator(U actuator)
{
    getParentActuatorAttachment().setObject(actuator);
}

template <class U>
inline SharedPtr<U> AstroParentActuatorAttachmentInterface::getParentActuator()
{
    return getParentActuatorAttachment().AstroAttachment::getObject<U>();
}

template <class U>
inline void AstroParentSensorAttachmentInterface::setParentSensor(U sensor)
{
    getParentSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> AstroParentSensorAttachmentInterface::getParentSensor()
{
    return getParentSensorAttachment().AstroAttachment::getObject<U>();
}

template <class U>
inline void AstroParentMountAttachmentInterface::setParentMount(U panel, aposi_t axisIndex)
{
    getParentMountAttachment().setObject(panel);
    getParentMountAttachment().setParentSubIndex(axisIndex);
}

template <class U>
inline SharedPtr<U> AstroParentMountAttachmentInterface::getParentMount()
{
    return getParentMountAttachment().AstroAttachment::getObject<U>();
}

inline aposi_t AstroParentMountAttachmentInterface::getParentMountAxisIndex()
{
    getParentMountAttachment().getParentSubIndex();
}

template <class U>
inline void AstroParentRailAttachmentInterface::setParentRail(U rail)
{
    getParentRailAttachment().setObject(rail);
}

template <class U>
inline SharedPtr<U> AstroParentRailAttachmentInterface::getParentRail()
{
    return getParentRailAttachment().AstroAttachment::getObject<U>();
}


template <class U>
inline void AstroSensorAttachmentInterface::setSensor(U sensor)
{
    getSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> AstroSensorAttachmentInterface::getSensor(bool poll)
{
    getSensorAttachment().updateIfNeeded(poll);
    return getSensorAttachment().AstroAttachment::getObject<U>();
}

template <class U>
inline void AstroAngleSensorAttachmentInterface::setAngleSensor(U sensor)
{
    getAngleSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> AstroAngleSensorAttachmentInterface::getAngleSensor(bool poll)
{
    getAngleSensorAttachment().updateIfNeeded(poll);
    return getAngleSensorAttachment().AstroAttachment::getObject<U>();
}

template <class U>
inline void AstroPositionSensorAttachmentInterface::setPositionSensor(U sensor)
{
    getPositionSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> AstroPositionSensorAttachmentInterface::getPositionSensor(bool poll)
{
    getPositionSensorAttachment().updateIfNeeded(poll);
    return getPositionSensorAttachment().AstroAttachment::getObject<U>();
}

template <class U>
inline void AstroPowerProductionSensorAttachmentInterface::setPowerProductionSensor(U sensor)
{
    getPowerProductionSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> AstroPowerProductionSensorAttachmentInterface::getPowerProductionSensor(bool poll)
{
    getPowerProductionSensorAttachment().updateIfNeeded(poll);
    return getPowerProductionSensorAttachment().AstroAttachment::getObject<U>();
}

template <class U>
inline void AstroPowerUsageSensorAttachmentInterface::setPowerUsageSensor(U sensor)
{
    getPowerUsageSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> AstroPowerUsageSensorAttachmentInterface::getPowerUsageSensor(bool poll)
{
    getPowerUsageSensorAttachment().updateIfNeeded(poll);
    return getPowerUsageSensorAttachment().AstroAttachment::getObject<U>();
}

template <class U>
inline void AstroSpeedSensorAttachmentInterface::setSpeedSensor(U sensor)
{
    getSpeedSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> AstroSpeedSensorAttachmentInterface::getSpeedSensor(bool poll)
{
    getSpeedSensorAttachment().updateIfNeeded(poll);
    return getSpeedSensorAttachment().AstroAttachment::getObject<U>();
}

template <class U>
inline void AstroTemperatureSensorAttachmentInterface::setTemperatureSensor(U sensor)
{
    getTemperatureSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> AstroTemperatureSensorAttachmentInterface::getTemperatureSensor(bool poll)
{
    getTemperatureSensorAttachment().updateIfNeeded(poll);
    return getTemperatureSensorAttachment().AstroAttachment::getObject<U>();
}

template <class U>
inline void AstroWindSpeedSensorAttachmentInterface::setWindSpeedSensor(U sensor)
{
    getWindSpeedSensorAttachment().setObject(sensor);
}

template <class U>
inline SharedPtr<U> AstroWindSpeedSensorAttachmentInterface::getWindSpeedSensor(bool poll)
{
    getWindSpeedSensorAttachment().updateIfNeeded(poll);
    return getWindSpeedSensorAttachment().AstroAttachment::getObject<U>();
}


template <class U>
inline void AstroTriggerAttachmentInterface::setTrigger(U trigger)
{
    getTriggerAttachment().setObject(trigger);
}

template <class U>
inline SharedPtr<U> AstroTriggerAttachmentInterface::getTrigger(bool poll)
{
    getTriggerAttachment().updateIfNeeded(poll);
    return getTriggerAttachment().AstroAttachment::getObject<U>();
}

template <class U>
inline void AstroMinimumTriggerAttachmentInterface::setMinimumTrigger(U trigger)
{
    getMinimumTriggerAttachment().setObject(trigger);
}

template <class U>
inline SharedPtr<U> AstroMinimumTriggerAttachmentInterface::getMinimumTrigger(bool poll)
{
    getMinimumTriggerAttachment().updateIfNeeded(poll);
    return getMinimumTriggerAttachment().AstroAttachment::getObject<U>();
}

template <class U>
inline void AstroMaximumTriggerAttachmentInterface::setMaximumTrigger(U trigger)
{
    getMaximumTriggerAttachment().setObject(trigger);
}

template <class U>
inline SharedPtr<U> AstroMaximumTriggerAttachmentInterface::getMaximumTrigger(bool poll)
{
    getMaximumTriggerAttachment().updateIfNeeded(poll);
    return getMaximumTriggerAttachment().AstroAttachment::getObject<U>();
}

template <class U>
inline void AstroLimitTriggerAttachmentInterface::setLimitTrigger(U trigger)
{
    getLimitTriggerAttachment().setObject(trigger);
}

template <class U>
inline SharedPtr<U> AstroLimitTriggerAttachmentInterface::getLimitTrigger(bool poll)
{
    getLimitTriggerAttachment().updateIfNeeded(poll);
    return getLimitTriggerAttachment().AstroAttachment::getObject<U>();
}

#endif // /ifndef AstroInterfaces_HPP

