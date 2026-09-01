/*  Astruino: OLED overview scaffold.
    Copyright (C) 2026 NachtRaveVL
*/
#ifndef AstroOverviewOLED_H
#define AstroOverviewOLED_H
class AstroOverviewOLED : public AstroOverview {
public:
    explicit AstroOverviewOLED(AstroDisplayDriver *display, const void *clockFont = nullptr, const void *detailFont = nullptr) : AstroOverview(display) { (void)clockFont; (void)detailFont; }
    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override { (void)isLandscape; (void)screenSize; _needsFullRedraw = false; }
};
#endif
