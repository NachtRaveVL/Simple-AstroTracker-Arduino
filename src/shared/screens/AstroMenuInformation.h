/*  Astruino: Information menu screen scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroMenuInformation_H
#define AstroMenuInformation_H

#include "../AstruinoUI.h"

// UI stub: Domain-specific information menu behavior is intentionally not implemented yet.
class AstroMenuInformation : public AstroMenu
{
public:
    AstroMenuInformation() = default;
    virtual ~AstroMenuInformation() = default;

    virtual void loadMenu(MenuItem *addFrom = nullptr) override { (void)addFrom; _loaded = true; }
    virtual MenuItem *getRootItem() override { return nullptr; }
};

#endif // /ifndef AstroMenuInformation_H
#endif
