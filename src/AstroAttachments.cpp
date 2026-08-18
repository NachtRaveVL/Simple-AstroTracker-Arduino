/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Attachments
*/

#include "AstroAttachments.h"
#include "AstroActuators.h"
#include "AstroDrivers.h"
#include "AstroSensors.h"

AstroActuatorAttachment::AstroActuatorAttachment(AstroActuator *object, aposi_t parentSubIndex)
    : AstroAttachment<AstroActuator>(object, parentSubIndex)
{ ; }

void AstroActuatorAttachment::setPower(float power)
{
    if (_object) { _object->setPower(power); }
}

float AstroActuatorAttachment::getPower() const
{
    return _object ? _object->getPower() : 0.0f;
}

AstroSensorAttachment::AstroSensorAttachment(AstroSensor *object, uint8_t measurementRow,
                                             double convertParam, aposi_t parentSubIndex)
    : AstroAttachment<AstroSensor>(object, parentSubIndex),
      _measurementRow(measurementRow), _convertParam(convertParam)
{ ; }

bool AstroSensorAttachment::poll(int64_t timestamp, aframe_t frame)
{
    return _object ? _object->poll(timestamp, frame) : false;
}

const AstroSingleMeasurement *AstroSensorAttachment::getMeasurement() const
{
    return _object ? &_object->getMeasurement() : nullptr;
}

AstroAxisDriverAttachment::AstroAxisDriverAttachment(AstroAxisDriver *object, aposi_t axisIndex)
    : AstroAttachment<AstroAxisDriver>(object, axisIndex)
{ ; }

void AstroAxisDriverAttachment::setTargetDegrees(double targetDegrees)
{
    if (_object) { _object->setTargetDegrees(targetDegrees); }
}

void AstroAxisDriverAttachment::stop()
{
    if (_object) { _object->stop(); }
}

double AstroAxisDriverAttachment::getTargetDegrees() const
{
    return _object ? _object->getTargetDegrees() : 0.0;
}
