/*  Astruino: Mounts menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroMenuMounts_H
#define AstroMenuMounts_H

#include "../AstruinoUI.h"

// UI stub: Domain-specific mounts menu behavior is intentionally not implemented yet.
class AstroMenuMounts : public AstroMenu
{
public:
    AstroMenuMounts() = default;
    virtual ~AstroMenuMounts() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef AstroMenuMounts_H
#endif
