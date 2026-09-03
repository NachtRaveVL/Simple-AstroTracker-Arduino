/*  Astruino: PowerRails menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroMenuPowerRails_H
#define AstroMenuPowerRails_H

#include "../AstruinoUI.h"

// UI stub: Domain-specific powerrails menu behavior is intentionally not implemented yet.
class AstroMenuPowerRails : public AstroMenu
{
public:
    AstroMenuPowerRails() = default;
    virtual ~AstroMenuPowerRails() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef AstroMenuPowerRails_H
#endif
