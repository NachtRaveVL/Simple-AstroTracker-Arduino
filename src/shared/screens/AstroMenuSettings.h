/*  Astruino: Settings menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroMenuSettings_H
#define AstroMenuSettings_H

#include "../AstruinoUI.h"

// UI stub: Domain-specific settings menu behavior is intentionally not implemented yet.
class AstroMenuSettings : public AstroMenu
{
public:
    AstroMenuSettings() = default;
    virtual ~AstroMenuSettings() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef AstroMenuSettings_H
#endif
