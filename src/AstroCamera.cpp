/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Observation Devices
*/

#include "AstroCamera.h"

AstroCameraTrigger::AstroCameraTrigger(TriggerCallback callback, void *context, aposi_t positionIndex)
    : AstroObject(AstroIdentity(AstroIdentity::ObservationDevice, 0, positionIndex)), _callback(callback), _context(context), _ready(true), _capturing(false)
{ ; }

AstroCameraTrigger::AstroCameraTrigger(const AstroObjectData *dataIn)
    : AstroObject(dataIn), _callback(nullptr), _context(nullptr), _ready(true), _capturing(false)
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
