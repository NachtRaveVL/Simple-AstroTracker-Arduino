/*  Astruino: LCD overview scaffold.
    Copyright (C) 2026 NachtRaveVL
*/
#ifndef AstroOverviewLCD_H
#define AstroOverviewLCD_H
class AstroOverviewLCD : public AstroOverview {
public:
    explicit AstroOverviewLCD(AstroDisplayDriver *display, const void *clockFont = nullptr, const void *detailFont = nullptr) : AstroOverview(display) { (void)clockFont; (void)detailFont; }
    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override { (void)isLandscape; (void)screenSize; _needsFullRedraw = false; }
};
#endif
