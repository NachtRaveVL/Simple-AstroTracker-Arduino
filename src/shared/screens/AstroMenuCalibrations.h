/*  Astruino: Calibrations menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroMenuCalibrations_H
#define AstroMenuCalibrations_H

#include "../AstruinoUI.h"

// UI stub: Domain-specific calibrations menu behavior is intentionally not implemented yet.
class AstroMenuCalibrations : public AstroMenu
{
public:
    AstroMenuCalibrations() = default;
    virtual ~AstroMenuCalibrations() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef AstroMenuCalibrations_H
#endif
