/*  Astruino: Alerts menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroMenuAlerts_H
#define AstroMenuAlerts_H

#include "../AstruinoUI.h"

// UI stub: Domain-specific alerts menu behavior is intentionally not implemented yet.
class AstroMenuAlerts : public AstroMenu
{
public:
    AstroMenuAlerts() = default;
    virtual ~AstroMenuAlerts() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef AstroMenuAlerts_H
#endif
