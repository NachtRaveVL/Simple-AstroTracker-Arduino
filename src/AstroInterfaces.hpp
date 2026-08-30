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


template<class U>
inline void AstroParentActuatorAttachmentInterface::setParentActuator(U actuator)
{
    getParentActuatorAttachment().setObject(actuator);
}

template<class U>
inline SharedPtr<U> AstroParentActuatorAttachmentInterface::getParentActuator()
{
    return getParentActuatorAttachment().AstroAttachment::getObject<U>();
}

template<class U>
inline void AstroParentSensorAttachmentInterface::setParentSensor(U sensor)
{
    getParentSensorAttachment().setObject(sensor);
}

template<class U>
inline SharedPtr<U> AstroParentSensorAttachmentInterface::getParentSensor()
{
    return getParentSensorAttachment().AstroAttachment::getObject<U>();
}

template<class U>
inline void AstroParentMountAttachmentInterface::setParentMount(U mount, aposi_t axisIndex)
{
    getParentMountAttachment().setObject(mount);
    getParentMountAttachment().setParentSubIndex(axisIndex);
}

template<class U>
inline SharedPtr<U> AstroParentMountAttachmentInterface::getParentMount()
{
    return getParentMountAttachment().AstroAttachment::getObject<U>();
}

inline aposi_t AstroParentMountAttachmentInterface::getParentMountAxisIndex()
{
    return getParentMountAttachment().getParentSubIndex();
}

template<class U>
inline void AstroParentRailAttachmentInterface::setParentRail(U rail)
{
    getParentRailAttachment().setObject(rail);
}

template<class U>
inline SharedPtr<U> AstroParentRailAttachmentInterface::getParentRail()
{
    return getParentRailAttachment().AstroAttachment::getObject<U>();
}


template<class U>
inline void AstroSensorAttachmentInterface::setSensor(U sensor)
{
    getSensorAttachment().setObject(sensor);
}

template<class U>
inline SharedPtr<U> AstroSensorAttachmentInterface::getSensor(bool poll)
{
    getSensorAttachment().updateIfNeeded(poll);
    return getSensorAttachment().AstroAttachment::getObject<U>();
}

template<class U>
inline void AstroTemperatureSensorAttachmentInterface::setTemperatureSensor(U sensor)
{
    getTemperatureSensorAttachment().setObject(sensor);
}

template<class U>
inline SharedPtr<U> AstroTemperatureSensorAttachmentInterface::getTemperatureSensor(bool poll)
{
    getTemperatureSensorAttachment().updateIfNeeded(poll);
    return getTemperatureSensorAttachment().AstroAttachment::getObject<U>();
}

template<class U>
inline void AstroHumiditySensorAttachmentInterface::setHumiditySensor(U sensor)
{
    getHumiditySensorAttachment().setObject(sensor);
}

template<class U>
inline SharedPtr<U> AstroHumiditySensorAttachmentInterface::getHumiditySensor(bool poll)
{
    getHumiditySensorAttachment().updateIfNeeded(poll);
    return getHumiditySensorAttachment().AstroAttachment::getObject<U>();
}

template<class U>
inline void AstroWindSpeedSensorAttachmentInterface::setWindSpeedSensor(U sensor)
{
    getWindSpeedSensorAttachment().setObject(sensor);
}

template<class U>
inline SharedPtr<U> AstroWindSpeedSensorAttachmentInterface::getWindSpeedSensor(bool poll)
{
    getWindSpeedSensorAttachment().updateIfNeeded(poll);
    return getWindSpeedSensorAttachment().AstroAttachment::getObject<U>();
}

template<class U>
inline void AstroRainSensorAttachmentInterface::setRainSensor(U sensor)
{
    getRainSensorAttachment().setObject(sensor);
}

template<class U>
inline SharedPtr<U> AstroRainSensorAttachmentInterface::getRainSensor(bool poll)
{
    getRainSensorAttachment().updateIfNeeded(poll);
    return getRainSensorAttachment().AstroAttachment::getObject<U>();
}

template<class U>
inline void AstroLightSensorAttachmentInterface::setLightSensor(U sensor)
{
    getLightSensorAttachment().setObject(sensor);
}

template<class U>
inline SharedPtr<U> AstroLightSensorAttachmentInterface::getLightSensor(bool poll)
{
    getLightSensorAttachment().updateIfNeeded(poll);
    return getLightSensorAttachment().AstroAttachment::getObject<U>();
}

template<class U>
inline void AstroPositionSensorAttachmentInterface::setPositionSensor(U sensor)
{
    getPositionSensorAttachment().setObject(sensor);
}

template<class U>
inline SharedPtr<U> AstroPositionSensorAttachmentInterface::getPositionSensor(bool poll)
{
    getPositionSensorAttachment().updateIfNeeded(poll);
    return getPositionSensorAttachment().AstroAttachment::getObject<U>();
}

template<class U>
inline void AstroPowerUsageSensorAttachmentInterface::setPowerUsageSensor(U sensor)
{
    getPowerUsageSensorAttachment().setObject(sensor);
}

template<class U>
inline SharedPtr<U> AstroPowerUsageSensorAttachmentInterface::getPowerUsageSensor(bool poll)
{
    getPowerUsageSensorAttachment().updateIfNeeded(poll);
    return getPowerUsageSensorAttachment().AstroAttachment::getObject<U>();
}


inline void AstroAxisDriverAttachmentInterface::setAxisDriver(SharedPtr<AstroAxisDriver> driver)
{
    getAxisDriverAttachment().setObject(driver);
}

inline SharedPtr<AstroAxisDriver> AstroAxisDriverAttachmentInterface::getAxisDriver()
{
    return getAxisDriverAttachment().getObject();
}

inline void AstroTriggerAttachmentInterface::setTrigger(SharedPtr<AstroTrigger> trigger)
{
    getTriggerAttachment().setObject(trigger);
}

inline SharedPtr<AstroTrigger> AstroTriggerAttachmentInterface::getTrigger(bool poll)
{
    getTriggerAttachment().updateIfNeeded(poll);
    return getTriggerAttachment().getObject();
}

inline void AstroLimitTriggerAttachmentInterface::setLimitTrigger(SharedPtr<AstroTrigger> trigger)
{
    getLimitTriggerAttachment().setObject(trigger);
}

inline SharedPtr<AstroTrigger> AstroLimitTriggerAttachmentInterface::getLimitTrigger(bool poll)
{
    getLimitTriggerAttachment().updateIfNeeded(poll);
    return getLimitTriggerAttachment().getObject();
}

template<class U>
inline void AstroObservationDeviceAttachmentInterface::setObservationDevice(SharedPtr<U> device)
{
    getObservationDeviceAttachment().setObject(device);
}

template<class U>
inline SharedPtr<U> AstroObservationDeviceAttachmentInterface::getObservationDevice()
{
    return getObservationDeviceAttachment().AstroAttachment::getObject<U>();
}

#endif // /ifndef AstroInterfaces_HPP
