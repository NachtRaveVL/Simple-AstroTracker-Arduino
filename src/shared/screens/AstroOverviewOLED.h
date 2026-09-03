/*  Astruino: U8g2 OLED Overview Screen.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroOverviewOLED_H
#define AstroOverviewOLED_H

class AstroOverviewOLED;

#include "../AstruinoUI.h"

class AstroOverviewOLED : public AstroOverview {
public:
    AstroOverviewOLED(AstroDisplayU8g2OLED *display, const void *clockFont, const void *detailFont);
    virtual ~AstroOverviewOLED();

    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override;

protected:
    U8G2 &_gfx;
    #ifdef ASTRO_UI_ENABLE_STCHROMA_LDTC
        StChromaArtDrawable &_drawable;
    #else
        U8g2Drawable &_drawable;
    #endif
    const void *_clockFont;
    const void *_detailFont;
};

#endif // /ifndef AstroOverviewOLED_H
#endif
