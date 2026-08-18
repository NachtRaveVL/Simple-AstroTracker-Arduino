/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Power Rails
*/

#ifndef AstroRails_H
#define AstroRails_H

#include "AstroObject.h"

// Power Rail
// Tracks available power capacity for equipment sharing a supply rail.
class AstroRail : public AstroObject, public AstroRailObjectInterface {
public:
    AstroRail(Astro_RailType railType = Astro_RailType_DC12V,
              double voltage = 12.0,
              double maxPowerWatts = 0.0,
              aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG); // Position index
    AstroRail(const AstroObjectData *dataIn);

    virtual bool requestPower(double watts) override;
    virtual void releasePower(double watts) override;
    void resetUsage();

    inline Astro_RailType getRailType() const { return _railType; }
    inline double getVoltage() const { return _voltage; }
    inline double getMaxPower() const { return _maxPowerWatts; }
    inline double getPowerUsage() const { return _powerUsageWatts; }
    virtual double getAvailablePower() const override {
        return _maxPowerWatts > 0.0 ? (_maxPowerWatts - _powerUsageWatts) : 1.0e30;
    }

protected:
    Astro_RailType _railType;                                // Power rail type
    double _voltage;                                         // Rail voltage
    double _maxPowerWatts;                                   // Maximum rail power, in watts
    double _powerUsageWatts;                                 // Current rail power usage, in watts
};

#endif // /ifndef AstroRails_H
