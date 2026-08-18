// Astro Lib to CPP export script - mainly for dev purposes

#include <Astruino.h>

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }

    Serial.println(F("Writing targets library..."));

    char targetId[ASTRO_TARGET_NAME_MAXSIZE];
    char jsonData[256];
    for (unsigned int targetIndex = 0; targetIndex < Astro_Target_Count; ++targetIndex) {
        Astro_TargetId targetType = (Astro_TargetId)targetIndex;
        const AstroTargetData *targetData = astroLib.checkoutTargetData(targetType);

        if (targetData && astroTargetIdToString(targetType, targetId, sizeof(targetId)) &&
            targetData->toJSON(jsonData, sizeof(jsonData))) {
            Serial.print(F("        case Astro_Target_"));
            Serial.print(targetId);
            Serial.println(F(": {"));
            Serial.print(F("            static const char flashStr_"));
            Serial.print(targetId);
            Serial.print(F("[] PROGMEM = {\""));

            for (const char *ch = jsonData; *ch; ++ch) {
                if (*ch == '\\' || *ch == '\"') { Serial.print('\\'); }
                Serial.print(*ch);
            }

            Serial.println(F("\"};"));
            Serial.print(F("            progmemStream = AstroPROGMEMStream(flashStr_"));
            Serial.print(targetId);
            Serial.print(F(", sizeof(flashStr_"));
            Serial.print(targetId);
            Serial.println(F(") - 1);"));
            Serial.println(F("        } break;"));
        }

        if (targetData) { astroLib.returnTargetData(targetData); }
        yield();
    }

    Serial.println(F("Done!"));
}

void loop()
{ ; }
