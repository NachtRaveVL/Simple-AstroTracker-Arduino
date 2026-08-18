/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Targets
*/

#include "AstroTargets.h"
#include "AstroEphemeris.h"
#include "AstroUtils.h"
#include <stdio.h>
#include <string.h>

AstroTargetData::AstroTargetData()
    : targetId(Astro_Target_Undefined), targetClass(Astro_TargetClass_Unknown),
      rightAscensionSeconds(0), declinationArcseconds(0), magnitudeCenti(32767),
      movingTarget(false), modified(false)
{
    id[0] = '\0';
    targetName[0] = '\0';
}

AstroEquatorialCoordinates AstroTargetData::getCoordinates(int64_t unixTime) const
{
    if (movingTarget) {
        AstroEquatorialCoordinates coordinates;
        if (astroResolveSolarSystemTarget(targetId, unixTime, &coordinates)) { return coordinates; }
    }
    return astroPrecessJ2000(getJ2000Coordinates(), unixTime);
}

bool AstroTargetData::toJSON(char *bufferOut, size_t bufferSize) const
{
    if (!bufferOut || !bufferSize) { return false; }
    int written = snprintf(bufferOut, bufferSize,
        "{\"type\":\"ATLD\",\"id\":\"%s\",\"name\":\"%s\",\"targetId\":%u,\"class\":%d,\"raSec\":%ld,\"decArcSec\":%ld,\"mag100\":%d,\"moving\":%s}",
        id, targetName, (unsigned int)targetId, (int)targetClass,
        (long)rightAscensionSeconds, (long)declinationArcseconds, (int)magnitudeCenti,
        movingTarget ? "true" : "false");
    return written > 0 && (size_t)written < bufferSize;
}


bool AstroTargetData::fromJSON(const char *jsonIn)
{
    if (!jsonIn) { return false; }

    char typeIn[8] = {0};
    char idIn[ASTRO_TARGET_ID_MAXSIZE] = {0};
    char nameIn[ASTRO_TARGET_NAME_MAXSIZE] = {0};
    unsigned long targetIdIn = 0;
    long targetClassIn = 0, raIn = 0, decIn = 0, magnitudeIn = 32767;
    bool movingIn = false;
    if (!astroJSONGetString(jsonIn, "type", typeIn, sizeof(typeIn)) || strcmp(typeIn, "ATLD") != 0 ||
        !astroJSONGetString(jsonIn, "id", idIn, sizeof(idIn)) ||
        !astroJSONGetString(jsonIn, "name", nameIn, sizeof(nameIn)) ||
        !astroJSONGetUnsignedLong(jsonIn, "targetId", &targetIdIn) ||
        !astroJSONGetLong(jsonIn, "class", &targetClassIn) ||
        !astroJSONGetLong(jsonIn, "raSec", &raIn) ||
        !astroJSONGetLong(jsonIn, "decArcSec", &decIn) ||
        !astroJSONGetLong(jsonIn, "mag100", &magnitudeIn) ||
        !astroJSONGetBool(jsonIn, "moving", &movingIn) ||
        targetIdIn >= Astro_Target_Count) {
        return false;
    }

    targetId = (Astro_TargetId)targetIdIn;
    targetClass = (Astro_TargetClass)targetClassIn;
    snprintf(id, sizeof(id), "%s", idIn);
    snprintf(targetName, sizeof(targetName), "%s", nameIn);
    rightAscensionSeconds = (int32_t)raIn;
    declinationArcseconds = (int32_t)decIn;
    magnitudeCenti = (int16_t)magnitudeIn;
    movingTarget = movingIn;
    modified = false;
    return true;
}
