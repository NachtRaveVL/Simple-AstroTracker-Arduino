/*  Astruino: U8g2 OLED Overview Screen.
    Copyright (C) 2026 NachtRaveVL
*/

#include "../AstruinoUI.h"
#ifdef ASTRO_USE_GUI

AstroOverviewOLED::AstroOverviewOLED(AstroDisplayU8g2OLED *display, const void *clockFont, const void *detailFont)
    : AstroOverview(display), _gfx(display->getGfx()), _drawable(display->getDrawable()), _clockFont(clockFont), _detailFont(detailFont)
{ ; }

AstroOverviewOLED::~AstroOverviewOLED()
{ ; }

void AstroOverviewOLED::renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize)
{
    // todo
}

#endif
