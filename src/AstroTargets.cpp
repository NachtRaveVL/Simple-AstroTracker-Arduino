/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Targets
*/

#include "AstroTargets.h"
#include "AstroEphemeris.h"
#include <string.h>

AstroTargetData::AstroTargetData()
    : AstroData('A','T','L','D'), targetId(Astro_Target_Undefined), targetClass(Astro_TargetClass_Unknown),
      catalogId{0}, targetName{0}, rightAscensionSeconds(0), declinationArcseconds(0),
      magnitudeCenti(32767), movingTarget(false)
{
    _size = sizeof(*this);
}

AstroEquatorialCoordinates AstroTargetData::getCoordinates(int64_t unixTime) const
{
    if (movingTarget) {
        AstroEquatorialCoordinates coordinates;
        if (astroResolveSolarSystemTarget(targetId, unixTime, &coordinates)) { return coordinates; }
    }
    return astroPrecessJ2000(getJ2000Coordinates(), unixTime);
}

void AstroTargetData::toJSONObject(JsonObject &objectOut) const
{
    AstroData::toJSONObject(objectOut);
    objectOut["catalogId"] = catalogId;
    objectOut["name"] = targetName;
    objectOut["targetId"] = (unsigned int)targetId;
    objectOut["class"] = (int)targetClass;
    objectOut["raSec"] = rightAscensionSeconds;
    objectOut["decArcSec"] = declinationArcseconds;
    objectOut["mag100"] = magnitudeCenti;
    objectOut["moving"] = movingTarget;
}

void AstroTargetData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroData::fromJSONObject(objectIn);

    unsigned int targetIdIn = objectIn["targetId"] | (unsigned int)targetId;
    if (targetIdIn >= Astro_Target_Count) { targetId = Astro_Target_Undefined; return; }
    targetId = (Astro_TargetId)targetIdIn;
    targetClass = (Astro_TargetClass)(objectIn["class"] | (int)targetClass);

    const char *catalogIdIn = objectIn["catalogId"] | nullptr;
    if (catalogIdIn) {
        strncpy(catalogId, catalogIdIn, sizeof(catalogId) - 1);
        catalogId[sizeof(catalogId) - 1] = '\0';
    }
    const char *targetNameIn = objectIn["name"] | nullptr;
    if (targetNameIn) {
        strncpy(targetName, targetNameIn, sizeof(targetName) - 1);
        targetName[sizeof(targetName) - 1] = '\0';
    }

    rightAscensionSeconds = objectIn["raSec"] | rightAscensionSeconds;
    declinationArcseconds = objectIn["decArcSec"] | declinationArcseconds;
    magnitudeCenti = objectIn["mag100"] | magnitudeCenti;
    movingTarget = objectIn["moving"] | movingTarget;
    unsetModified();
}
