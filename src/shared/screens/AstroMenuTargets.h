/*  Astruino: Targets menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroMenuTargets_H
#define AstroMenuTargets_H

#include "../AstruinoUI.h"

// UI stub: Domain-specific targets menu behavior is intentionally not implemented yet.
class AstroMenuTargets : public AstroMenu
{
public:
    AstroMenuTargets() = default;
    virtual ~AstroMenuTargets() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef AstroMenuTargets_H
#endif
