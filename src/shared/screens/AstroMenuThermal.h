/*  Astruino: Thermal menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroMenuThermal_H
#define AstroMenuThermal_H

#include "../AstruinoUI.h"

// UI stub: Domain-specific thermal menu behavior is intentionally not implemented yet.
class AstroMenuThermal : public AstroMenu
{
public:
    AstroMenuThermal() = default;
    virtual ~AstroMenuThermal() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef AstroMenuThermal_H
#endif
