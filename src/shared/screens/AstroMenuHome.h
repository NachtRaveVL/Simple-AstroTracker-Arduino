/*  Astruino: Home menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroMenuHome_H
#define AstroMenuHome_H

#include "../AstruinoUI.h"

// UI stub: Domain-specific home menu behavior is intentionally not implemented yet.
class AstroHomeMenu : public AstroMenu
{
public:
    AstroHomeMenu() = default;
    virtual ~AstroHomeMenu() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }

    void unloadSubMenus() { ; }
};

#endif // /ifndef AstroMenuHome_H
#endif
