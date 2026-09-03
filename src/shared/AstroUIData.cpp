/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino UI Data
*/

#include "Astruino.h"
#include "AstroUIData.h"

AstroUIData::AstroUIData()
    : AstroData('A','U','I','D', 1),
      updatesPerSec(ASTRO_UI_UPDATE_SPEED),
      displayTheme(Astro_DisplayTheme_Undefined),
      titleMode(Astro_TitleMode_Undefined),
      analogSlider(false), editingIcons(false),
      joystickCalib{0.5f,0.5f,0.05f}, touchscreenCalib{0}
{
    _size = sizeof(*this);
}

void AstroUIData::toJSONObject(JsonObject &objectOut) const
{
    AstroData::toJSONObject(objectOut);

    if (updatesPerSec != ASTRO_UI_UPDATE_SPEED) { objectOut[SFP(AStr_Key_UpdatesPerSec)] = updatesPerSec; }
    if (displayTheme != Astro_DisplayTheme_Undefined) { objectOut[SFP(AStr_Key_DisplayTheme)] = displayTheme; }
    if (!isFPEqual(joystickCalib[0], 0.5f) || !isFPEqual(joystickCalib[1], 0.5f) || !isFPEqual(joystickCalib[2], 0.05f)) { objectOut[SFP(AStr_Key_JoystickCalib)] = commaStringFromArray(joystickCalib, 3); }
}

void AstroUIData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroData::fromJSONObject(objectIn);

    updatesPerSec = objectIn[SFP(AStr_Key_UpdatesPerSec)] | updatesPerSec;
    displayTheme = objectIn[SFP(AStr_Key_DisplayTheme)] | displayTheme;
    JsonVariantConst joystickCalibVar = objectIn[SFP(AStr_Key_JoystickCalib)];
    commaStringToArray(joystickCalibVar, joystickCalib, 3);
}
