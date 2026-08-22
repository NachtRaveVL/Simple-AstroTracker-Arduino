// Simple-AstroTracker-Arduino Data Writer Example
//
// In this example we export the built-in AstroTargetsLibrary target records as compact JSON. This is
// useful when preparing external SD/EEPROM storage for a constrained controller, or when
// regenerating the built-in data after catalog changes.
//
// Astruino can operate with all built-in target data kept in Flash. External storage is
// optional and is mainly useful when program space matters or user catalog data is desired.
//
// The companion tests/AstroTargetsLibraryExportToCPP sketch performs the opposite development task:
// it exports checked-out target data as C++ PROGMEM cases for inclusion in the library.

#include <Astruino.h>

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }

    Serial.println(F("Astruino external data export"));

    Serial.println(F("=== Library strings ==="));
    for (int stringIndex = 0; stringIndex < AStr_Count; ++stringIndex) {
        Serial.print(stringIndex);
        Serial.print(':');
        Serial.println(SFP((Astro_String)stringIndex));
        yield();
    }

    Serial.println(F("=== AstroTargetsLibrary targets ==="));

    char jsonBuffer[256];
    char targetId[ASTRO_TARGET_ID_MAXSIZE];

    for (int targetIndex = 0; targetIndex < Astro_Target_Count; ++targetIndex) {
        Astro_TargetId target = (Astro_TargetId)targetIndex;
        const AstroTargetData *targetData = astroLib.checkoutTargetData(target);

        if (targetData) {
            if (astroTargetIdToString(target, targetId, sizeof(targetId)) &&
                targetData->toJSON(jsonBuffer, sizeof(jsonBuffer))) {
                Serial.print(targetId);
                Serial.print(':');
                Serial.println(jsonBuffer);
            }

            astroLib.returnTargetData(targetData);
        }

        yield();
    }

    Serial.println(F("Done!"));
}

void loop()
{ ; }
