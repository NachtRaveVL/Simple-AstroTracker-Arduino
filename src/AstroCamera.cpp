/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Observation Devices
*/

#include "Astruino.h"

AstroObject *newObservationDeviceObjectFromData(const AstroObservationDeviceData *dataIn)
{
    if (dataIn && !isValidType(dataIn->id.object.idType)) return nullptr;
    ASTRO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(AStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData() && dataIn->id.object.idType == (aid_t)AstroIdentity::ObservationDevice) {
        switch (dataIn->id.object.classType) {
            case (aid_t)AstroObservationDevice::CameraTrigger:
                return new AstroCameraTrigger(dataIn);
            default: break;
        }
    }

    return nullptr;
}

AstroCameraTrigger::AstroCameraTrigger(TriggerCallback callback, void *context, aposi_t positionIndex)
    : AstroObject(AstroIdentity(AstroIdentity::ObservationDevice, 0, positionIndex)), AstroObservationDevice(CameraTrigger),
      _callback(callback), _context(context), _ready(true), _capturing(false)
{ ; }

AstroCameraTrigger::AstroCameraTrigger(const AstroObservationDeviceData *dataIn)
    : AstroObject(dataIn), AstroObservationDevice(dataIn ? dataIn->id.object.classType : Unknown),
      _callback(nullptr), _context(nullptr), _ready(true), _capturing(false)
{ ; }

void AstroCameraTrigger::startObservation()
{
    if (!_ready || _capturing) { return; }
    _capturing = true;
    if (_callback) { _callback(_context, true); }
}

void AstroCameraTrigger::stopObservation()
{
    if (!_capturing) { return; }
    _capturing = false;
    if (_callback) { _callback(_context, false); }
}

void AstroCameraTrigger::setReady(bool ready)
{
    _ready = ready;
    if (!_ready) { stopObservation(); }
}

void AstroCameraTrigger::setTriggerCallback(TriggerCallback callback, void *context)
{
    _callback = callback;
    _context = context;
}

AstroData *AstroCameraTrigger::allocateData() const
{
    return _allocateDataForObjType((aid_t)AstroIdentity::ObservationDevice, (aid_t)classType);
}

void AstroCameraTrigger::saveToData(AstroData *dataOut)
{
    AstroObject::saveToData(dataOut);
    if (dataOut) { dataOut->id.object.classType = (aid_t)classType; }
}


AstroObservationDeviceData::AstroObservationDeviceData()
    : AstroObjectData()
{
    _size = sizeof(*this);
    id.object.idType = (aid_t)AstroIdentity::ObservationDevice;
    id.object.objType = 0;
    id.object.posIndex = aposi_none;
    id.object.classType = (aid_t)AstroObservationDevice::CameraTrigger;
}
