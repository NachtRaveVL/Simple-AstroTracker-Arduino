/*  Astruino: TFT_eSPI Overview Screen.
    Copyright (C) 2026 NachtRaveVL
*/

#include "../AstruinoUI.h"
#ifdef ASTRO_USE_GUI

AstroOverviewTFT::AstroOverviewTFT(AstroDisplayTFTeSPI *display, const void *clockFont, const void *detailFont)
    : AstroOverview(display), _gfx(display->getGfx()), _drawable(display->getDrawable()), _clockFont(clockFont), _detailFont(detailFont)
{ ; }

AstroOverviewTFT::~AstroOverviewTFT()
{ ; }

void AstroOverviewTFT::renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize)
{
    // todo
}

#endif
