// Simple-AstroTracker-Arduino AstroTargetsLibrary Lookup Example
//
// Shows the built-in target library checkout/return pattern. The same AstroTargetData
// object is used for fixed J2000 catalog objects and moving solar system objects.

#include <Astruino.h>

#define SETUP_UNIX_TIME                 1787101200LL         // Replace with RTC/GPS/NTP time as needed

void printTarget(Astro_TargetId targetId)
{
    const AstroTargetData *targetData = astroLib.checkoutTargetData(targetId);
    if (!targetData) {
        Serial.println(F("Target lookup failed"));
        return;
    }

    AstroEquatorialCoordinates coordinates = targetData->getCoordinates(SETUP_UNIX_TIME);

    Serial.print(targetData->id);
    Serial.print(F(" - "));
    Serial.println(targetData->targetName);
    Serial.print(F("  RA: "));
    Serial.print(coordinates.rightAscensionHours, 5);
    Serial.print(F(" hr, DEC: "));
    Serial.print(coordinates.declinationDegrees, 5);
    Serial.println(F(" deg"));

    astroLib.returnTargetData(targetData);
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }

    Serial.println(F("Astruino AstroTargetsLibrary lookup"));

    printTarget(Astro_Target_M31);
    printTarget(Astro_Target_M42);
    printTarget(Astro_Target_M45);
    printTarget(Astro_Target_Sirius);
    printTarget(Astro_Target_Moon);
    printTarget(Astro_Target_Jupiter);

    char targetName[ASTRO_TARGET_ID_MAXSIZE];
    astroTargetIdToString(Astro_Target_M57, targetName, sizeof(targetName));
    Serial.print(F("M57 string lookup: "));
    Serial.println((int)astroTargetIdFromString(targetName));
}

void loop()
{ ; }
