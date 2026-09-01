/*  Astruino: Scheduling menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroMenuScheduling_H
#define AstroMenuScheduling_H

#include "../AstruinoUI.h"

// UI stub: Domain-specific scheduling menu behavior is intentionally not implemented yet.
class AstroMenuScheduling : public AstroMenu
{
public:
    AstroMenuScheduling() = default;
    virtual ~AstroMenuScheduling() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef AstroMenuScheduling_H
#endif
