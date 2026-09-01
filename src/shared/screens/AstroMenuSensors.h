/*  Astruino: Sensors menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroMenuSensors_H
#define AstroMenuSensors_H

#include "../AstruinoUI.h"

// UI stub: Domain-specific sensors menu behavior is intentionally not implemented yet.
class AstroMenuSensors : public AstroMenu
{
public:
    AstroMenuSensors() = default;
    virtual ~AstroMenuSensors() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef AstroMenuSensors_H
#endif
