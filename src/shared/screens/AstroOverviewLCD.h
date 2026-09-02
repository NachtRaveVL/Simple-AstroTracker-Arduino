/*  Astruino: LCD Overview Screen.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroOverviewLCD_H
#define AstroOverviewLCD_H

class AstroOverviewLCD;

#include "../AstruinoUI.h"

class AstroOverviewLCD : public AstroOverview {
public:
    AstroOverviewLCD(AstroDisplayLiquidCrystal *display);
    virtual ~AstroOverviewLCD();

    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override;

protected:
    LiquidCrystal &_lcd;
};

#endif // /ifndef AstroOverviewLCD_H
#endif
