/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Targets
*/

#ifndef AstroTargets_H
#define AstroTargets_H

#include "AstroCoordinates.h"
#include "AstroData.h"

// Astronomy Target Data
// Serializable catalog entry containing identity, display name, and coordinate data.
struct AstroTargetData : public AstroData {
    Astro_TargetId targetId;                                 // Target ID
    Astro_TargetClass targetClass;                           // Target class
    char catalogId[ASTRO_TARGET_ID_MAXSIZE];                 // Catalog/object ID
    char targetName[ASTRO_TARGET_NAME_MAXSIZE];              // Display target name
    int32_t rightAscensionSeconds;                           // J2000 right ascension, in seconds of time
    int32_t declinationArcseconds;                           // J2000 declination, in arcseconds
    int16_t magnitudeCenti;                                  // Visual magnitude multiplied by 100
    bool movingTarget;                                       // Target requires time-dependent ephemeris resolution

    AstroTargetData();

    inline AstroEquatorialCoordinates getJ2000Coordinates() const {
        return AstroEquatorialCoordinates((double)rightAscensionSeconds / 3600.0,
                                           (double)declinationArcseconds / 3600.0);
    }

    AstroEquatorialCoordinates getCoordinates(int64_t unixTime) const;
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef AstroTargets_H
