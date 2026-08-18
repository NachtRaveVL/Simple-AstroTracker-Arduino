// JSON export tests script - mainly for dev purposes

#include <Astruino.h>

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }

    int failures = 0;
    char jsonData[320];

    for (unsigned int targetIndex = 0; targetIndex < Astro_Target_Count; ++targetIndex) {
        const AstroTargetData *targetData = astroLib.checkoutTargetData((Astro_TargetId)targetIndex);
        if (!targetData || !targetData->toJSON(jsonData, sizeof(jsonData))) {
            ++failures;
        } else {
            AstroTargetData decoded;
            if (!decoded.fromJSON(jsonData) || decoded.targetId != targetData->targetId ||
                decoded.rightAscensionSeconds != targetData->rightAscensionSeconds ||
                decoded.declinationArcseconds != targetData->declinationArcseconds) {
                ++failures;
            }
        }
        if (targetData) { astroLib.returnTargetData(targetData); }
    }

    AstroPinData pinData;
    pinData.type = AstroPin::Digital;
    pinData.pin = 7;
    pinData.mode = Astro_PinMode_Digital_Input_PullUp;
    pinData.activeLow = true;
    if (!pinData.toJSON(jsonData, sizeof(jsonData))) { ++failures; }
    else {
        AstroPinData decodedPin;
        if (!decodedPin.fromJSON(jsonData) || decodedPin.pin != pinData.pin ||
            decodedPin.mode != pinData.mode || decodedPin.activeLow != pinData.activeLow) {
            ++failures;
        }
    }

    Serial.print(F("JSON failures: "));
    Serial.println(failures);
}

void loop()
{ ; }
