/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Interface Inlines
*/

#ifndef AstroInterfaces_HPP
#define AstroInterfaces_HPP

#include "AstroInterfaces.h"
#include "AstroAttachments.h"
#include "AstroUtils.h"

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

inline void AstroMeasurementUnitsInterfaceStorageSingle::setMeasurementUnits(Astro_UnitsType measurementUnits, uint8_t measurementRow)
{
    if (measurementRow == 0) { _measurementUnits[0] = measurementUnits; }
}

inline Astro_UnitsType AstroMeasurementUnitsInterfaceStorageSingle::getMeasurementUnits(uint8_t measurementRow) const
{
    return measurementRow == 0 ? _measurementUnits[0] : Astro_UnitsType_Undefined;
}

inline void AstroMeasurementUnitsInterfaceStorageDouble::setMeasurementUnits(Astro_UnitsType measurementUnits, uint8_t measurementRow)
{
    if (measurementRow < 2) { _measurementUnits[measurementRow] = measurementUnits; }
}

inline Astro_UnitsType AstroMeasurementUnitsInterfaceStorageDouble::getMeasurementUnits(uint8_t measurementRow) const
{
    return measurementRow < 2 ? _measurementUnits[measurementRow] : Astro_UnitsType_Undefined;
}

inline void AstroMeasurementUnitsInterfaceStorageTriple::setMeasurementUnits(Astro_UnitsType measurementUnits, uint8_t measurementRow)
{
    if (measurementRow < 3) { _measurementUnits[measurementRow] = measurementUnits; }
}

inline Astro_UnitsType AstroMeasurementUnitsInterfaceStorageTriple::getMeasurementUnits(uint8_t measurementRow) const
{
    return measurementRow < 3 ? _measurementUnits[measurementRow] : Astro_UnitsType_Undefined;
}

inline void AstroParentActuatorAttachmentInterface::setParentActuator(AstroActuator *actuator)
{
    getParentActuatorAttachment().setObject(actuator);
}

inline AstroActuator *AstroParentActuatorAttachmentInterface::getParentActuator()
{
    return getParentActuatorAttachment().getObject();
}

inline void AstroParentSensorAttachmentInterface::setParentSensor(AstroSensor *sensor)
{
    getParentSensorAttachment().setObject(sensor);
}

inline AstroSensor *AstroParentSensorAttachmentInterface::getParentSensor()
{
    return getParentSensorAttachment().getObject();
}

inline void AstroParentMountAttachmentInterface::setParentMount(AstroMount *mount)
{
    getParentMountAttachment().setObject(mount);
}

inline AstroMount *AstroParentMountAttachmentInterface::getParentMount()
{
    return getParentMountAttachment().getObject();
}

inline void AstroParentCoverAttachmentInterface::setParentCover(AstroCover *cover)
{
    getParentCoverAttachment().setObject(cover);
}

inline AstroCover *AstroParentCoverAttachmentInterface::getParentCover()
{
    return getParentCoverAttachment().getObject();
}

inline void AstroParentRailAttachmentInterface::setParentRail(AstroRail *rail)
{
    getParentRailAttachment().setObject(rail);
}

inline AstroRail *AstroParentRailAttachmentInterface::getParentRail()
{
    return getParentRailAttachment().getObject();
}

inline void AstroSensorAttachmentInterface::setSensor(AstroSensor *sensor)
{
    getSensorAttachment().setObject(sensor);
}

inline AstroSensor *AstroSensorAttachmentInterface::getSensor(bool poll, int64_t timestamp, aframe_t frame)
{
    if (poll) { getSensorAttachment().poll(timestamp, frame); }
    return getSensorAttachment().getObject();
}

inline void AstroTemperatureSensorAttachmentInterface::setTemperatureSensor(AstroSensor *sensor)
{
    getTemperatureSensorAttachment().setObject(sensor);
}

inline AstroSensor *AstroTemperatureSensorAttachmentInterface::getTemperatureSensor(bool poll, int64_t timestamp, aframe_t frame)
{
    if (poll) { getTemperatureSensorAttachment().poll(timestamp, frame); }
    return getTemperatureSensorAttachment().getObject();
}

inline void AstroPositionSensorAttachmentInterface::setPositionSensor(AstroSensor *sensor)
{
    getPositionSensorAttachment().setObject(sensor);
}

inline AstroSensor *AstroPositionSensorAttachmentInterface::getPositionSensor(bool poll, int64_t timestamp, aframe_t frame)
{
    if (poll) { getPositionSensorAttachment().poll(timestamp, frame); }
    return getPositionSensorAttachment().getObject();
}

inline void AstroAxisDriverAttachmentInterface::setAxisDriver(AstroAxisDriver *driver)
{
    getAxisDriverAttachment().setObject(driver);
}

inline AstroAxisDriver *AstroAxisDriverAttachmentInterface::getAxisDriver()
{
    return getAxisDriverAttachment().getObject();
}

inline void AstroTriggerAttachmentInterface::setTrigger(AstroTrigger *trigger)
{
    getTriggerAttachment().setObject(trigger);
}

inline AstroTrigger *AstroTriggerAttachmentInterface::getTrigger()
{
    return getTriggerAttachment().getObject();
}

inline void AstroObservationDeviceAttachmentInterface::setObservationDevice(AstroObservationDevice *device)
{
    getObservationDeviceAttachment().setObject(device);
}

inline AstroObservationDevice *AstroObservationDeviceAttachmentInterface::getObservationDevice()
{
    return getObservationDeviceAttachment().getObject();
}

#endif // /ifndef AstroInterfaces_HPP
