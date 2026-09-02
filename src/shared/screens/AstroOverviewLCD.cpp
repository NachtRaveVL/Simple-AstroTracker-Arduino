/*  Astruino: LCD Overview Screen.
    Copyright (C) 2026 NachtRaveVL
*/

#include "../AstruinoUI.h"
#ifdef ASTRO_USE_GUI

AstroOverviewLCD::AstroOverviewLCD(AstroDisplayLiquidCrystal *display)
    : AstroOverview(display), _lcd(display->getLCD())
{ ; }

AstroOverviewLCD::~AstroOverviewLCD()
{ ; }

void AstroOverviewLCD::renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize)
{
    // todo
}

#endif
