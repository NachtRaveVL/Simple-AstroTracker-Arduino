/*  Astruino: GFX overview scaffold.
    Copyright (C) 2026 NachtRaveVL
*/
#ifndef AstroOverviewGFX_H
#define AstroOverviewGFX_H
template<class T> class AstroOverviewGFX : public AstroOverview {
public:
    explicit AstroOverviewGFX(AstroDisplayDriver *display, const void *clockFont = nullptr, const void *detailFont = nullptr) : AstroOverview(display) { (void)clockFont; (void)detailFont; }
    virtual void renderOverview(bool isLandscape, Pair<uint16_t, uint16_t> screenSize) override { (void)isLandscape; (void)screenSize; _needsFullRedraw = false; }
};
#endif
