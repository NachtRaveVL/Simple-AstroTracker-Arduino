/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Modules
*/

#include "AstroModules.h"

AstroModule::AstroModule()
    : _initialized(false)
{ ; }

bool AstroModule::begin()
{
    _initialized = true;
    return true;
}

void AstroModule::update()
{ ; }


AstroManualTimeProvider::AstroManualTimeProvider(int64_t unixTime)
    : _unixTime(unixTime)
{ ; }

bool AstroManualTimeProvider::getUnixTime(int64_t *unixTimeOut)
{
    if (!unixTimeOut || _unixTime <= 0) { return false; }
    *unixTimeOut = _unixTime;
    return true;
}

void AstroManualTimeProvider::setUnixTime(int64_t unixTime)
{
    _unixTime = unixTime;
}

AstroFixedLocationProvider::AstroFixedLocationProvider(const AstroObserver &observer)
    : _observer(observer)
{ ; }

bool AstroFixedLocationProvider::getObserver(AstroObserver *observerOut)
{
    if (!observerOut) { return false; }
    *observerOut = _observer;
    return true;
}

void AstroFixedLocationProvider::setObserver(const AstroObserver &observer)
{
    _observer = observer;
}

AstroCallbackTimeProvider::AstroCallbackTimeProvider(TimeCallback callback, void *context)
    : _callback(callback), _context(context)
{ ; }

bool AstroCallbackTimeProvider::getUnixTime(int64_t *unixTimeOut)
{
    return _callback && unixTimeOut ? _callback(_context, unixTimeOut) : false;
}

AstroCallbackLocationProvider::AstroCallbackLocationProvider(LocationCallback callback, void *context)
    : _callback(callback), _context(context)
{ ; }

bool AstroCallbackLocationProvider::getObserver(AstroObserver *observerOut)
{
    return _callback && observerOut ? _callback(_context, observerOut) : false;
}
