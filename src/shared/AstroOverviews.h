/*  Astruino: overview screen scaffolding.
    Copyright (C) 2026 NachtRaveVL
    Astruino Overview Screens
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroOverviews_H
#define AstroOverviews_H

class AstroOverview;
class AstroOverviewLCD;
class AstroOverviewOLED;
template<class T> class AstroOverviewGFX;
class AstroOverviewTFT;

#include "AstruinoUI.h"

class AstroOverview
{
public:
    inline AstroOverview(AstroDisplayDriver *display) : _display(display), _needsFullRedraw(true) { ; }
    virtual ~AstroOverview() = default;
    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) = 0;
    inline void setNeedsFullRedraw() { _needsFullRedraw = true; }
protected:
    AstroDisplayDriver *_display;
    bool _needsFullRedraw;
};

#include "screens/AstroOverviewGFX.h"
#include "screens/AstroOverviewLCD.h"
#include "screens/AstroOverviewOLED.h"
#include "screens/AstroOverviewTFT.h"

#endif // /ifndef AstroOverviews_H
#endif
