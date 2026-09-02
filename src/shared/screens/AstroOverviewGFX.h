/*  Astruino: AdafruitGFX Overview Screen.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroOverviewGFX_H
#define AstroOverviewGFX_H

template<class T> class AstroOverviewGFX;

#include "../AstruinoUI.h"

template<class T>
class AstroOverviewGFX : public AstroOverview {
public:
    AstroOverviewGFX(AstroDisplayAdafruitGFX<T> *display, const void *clockFont = nullptr, const void *detailFont = nullptr);
    virtual ~AstroOverviewGFX();

    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override;

protected:
    T &_gfx;
    AdafruitDrawable<T> &_drawable;
    const void *_clockFont;
    const void *_detailFont;
};

#endif // /ifndef AstroOverviewGFX_H
#endif
