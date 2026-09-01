/*  Astruino: Actuators menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroMenuActuators_H
#define AstroMenuActuators_H

#include "../AstruinoUI.h"

// UI stub: Domain-specific actuators menu behavior is intentionally not implemented yet.
class AstroMenuActuators : public AstroMenu
{
public:
    AstroMenuActuators() = default;
    virtual ~AstroMenuActuators() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef AstroMenuActuators_H
#endif
