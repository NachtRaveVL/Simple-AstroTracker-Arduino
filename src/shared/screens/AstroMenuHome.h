/*  Astruino: Home menu screen.
    Copyright (C) 2026 NachtRaveVL
    Astruino Home Menu Screen
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroMenuHome_H
#define AstroMenuHome_H

class AstroHomeMenu;
#ifdef ASTRO_DISABLE_BUILTIN_DATA
struct AstroHomeMenuInfo;
#endif
struct AstroHomeMenuItems;

#include "../AstruinoUI.h"

class AstroHomeMenu : public AstroMenu
{
public:
    AstroHomeMenu();
    virtual ~AstroHomeMenu();

    virtual void loadMenu(MenuItem *addFrom = nullptr) override;
    virtual MenuItem *getRootItem() override;

    void unloadSubMenus();

    inline AstroHomeMenuItems &getItems() { return *_items; }

protected:
    AstroHomeMenuItems *_items;
};

#ifdef ASTRO_DISABLE_BUILTIN_DATA
struct AstroHomeMenuInfo {
    AstroHomeMenuInfo();

    AnyMenuInfo minfoBackToOverview;
#ifdef ASTRO_UI_ENABLE_DEBUG_MENU
    AnyMenuInfo minfoTriggerSigLocation;
    AnyMenuInfo minfoTriggerSigTime;
    AnyMenuInfo minfoTriggerSDCleanup;
    AnyMenuInfo minfoTriggerLowMem;
    AnyMenuInfo minfoTriggerAutosave;
    BooleanMenuInfo minfoSimhubConnected;
    SubMenuInfo minfoDebug;
#endif
    AnyMenuInfo minfoInformation;
    AnyMenuInfo minfoCalibrations;
    SubMenuInfo minfoLibrary;
    AnyMenuInfo minfoSettings;
    AnyMenuInfo minfoScheduling;
    AnyMenuInfo minfoPowerRails;
    AnyMenuInfo minfoMounts;
    AnyMenuInfo minfoTargets;
    AnyMenuInfo minfoThermal;
    AnyMenuInfo minfoSensors;
    AnyMenuInfo minfoActuators;
    SubMenuInfo minfoSystem;
    AnyMenuInfo minfoAlerts;
};
#endif

struct AstroHomeMenuItems {
    AstroHomeMenuItems();

#ifdef ASTRO_DISABLE_BUILTIN_DATA
    AstroHomeMenuInfo init;
#endif

    ActionMenuItem menuBackToOverview;
#ifdef ASTRO_UI_ENABLE_DEBUG_MENU
    ActionMenuItem menuTriggerSigLocation;
    ActionMenuItem menuTriggerSigTime;
    ActionMenuItem menuTriggerSDCleanup;
    ActionMenuItem menuTriggerLowMem;
    ActionMenuItem menuTriggerAutosave;
    BooleanMenuItem menuSimhubConnected;
    BackMenuItem menuBackDebug;
    SubMenuItem menuDebug;
#endif
    ActionMenuItem menuInformation;
    ActionMenuItem menuCalibrations;
    BackMenuItem menuBackLibrary;
    SubMenuItem menuLibrary;
    ActionMenuItem menuSettings;
    ActionMenuItem menuScheduling;
    ActionMenuItem menuPowerRails;
    ActionMenuItem menuMounts;
    ActionMenuItem menuTargets;
    ActionMenuItem menuThermal;
    ActionMenuItem menuSensors;
    ActionMenuItem menuActuators;
    BackMenuItem menuBackSystem;
    SubMenuItem menuSystem;
    ActionMenuItem menuAlerts;
};

#endif // /ifndef AstroMenuHome_H
#endif
