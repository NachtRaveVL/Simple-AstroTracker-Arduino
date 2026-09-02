/*  Astruino: AdafruitGFX Overview Screen.
    Copyright (C) 2026 NachtRaveVL
*/

#include "../AstruinoUI.h"
#ifdef ASTRO_USE_GUI

template <class T>
AstroOverviewGFX<T>::AstroOverviewGFX(AstroDisplayAdafruitGFX<T> *display, const void *clockFont, const void *detailFont)
    : AstroOverview(display), _gfx(display->getGfx()), _drawable(display->getDrawable()), _clockFont(clockFont), _detailFont(detailFont)
{ ; }

template <class T>
AstroOverviewGFX<T>::~AstroOverviewGFX()
{ ; }

template <class T>
void AstroOverviewGFX<T>::renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize)
{
    // todo
}

#endif
