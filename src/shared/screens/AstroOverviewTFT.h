/*  Astruino: TFT_eSPI Overview Screen.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroOverviewTFT_H
#define AstroOverviewTFT_H

class AstroOverviewTFT;

#include "../AstruinoUI.h"

class AstroOverviewTFT : public AstroOverview {
public:
    AstroOverviewTFT(AstroDisplayTFTeSPI *display, const void *clockFont, const void *detailFont);
    virtual ~AstroOverviewTFT();

    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override;

protected:
    TFT_eSPI &_gfx;
    TfteSpiDrawable &_drawable;
    const void *_clockFont;
    const void *_detailFont;
};

#endif // /ifndef AstroOverviewTFT_H
#endif
