/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Power Rails
*/

#include "AstroRails.h"

AstroRail::AstroRail(Astro_RailType railType, double voltage, double maxPowerWatts, aposi_t positionIndex)
    : AstroObject(AstroIdentity(railType, positionIndex)), _railType(railType),
      _voltage(voltage), _maxPowerWatts(maxPowerWatts), _powerUsageWatts(0.0)
{ ; }

AstroRail::AstroRail(const AstroObjectData *dataIn)
    : AstroObject(dataIn), _railType(dataIn ? (Astro_RailType)dataIn->objType : Astro_RailType_Undefined),
      _voltage(_railType == Astro_RailType_DC3V3 ? 3.3 : _railType == Astro_RailType_DC5V ? 5.0 : _railType == Astro_RailType_DC24V ? 24.0 : 12.0),
      _maxPowerWatts(0.0), _powerUsageWatts(0.0)
{ ; }

bool AstroRail::requestPower(double watts)
{
    if (watts <= 0.0) { return true; }
    if (_maxPowerWatts > 0.0 && _powerUsageWatts + watts > _maxPowerWatts) { return false; }
    _powerUsageWatts += watts;
    return true;
}

void AstroRail::releasePower(double watts)
{
    if (watts <= 0.0) { return; }
    _powerUsageWatts -= watts;
    if (_powerUsageWatts < 0.0) { _powerUsageWatts = 0.0; }
}

void AstroRail::resetUsage()
{
    _powerUsageWatts = 0.0;
}
