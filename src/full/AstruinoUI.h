/*  Astruino: Full/RW UI scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstruinoUI_H
#define AstruinoUI_H

class AstruinoFullUI;
typedef AstruinoFullUI AstruinoUI;

#include "../shared/AstruinoUI.h"

class AstruinoFullUI : public AstruinoBaseUI {
public:
    AstruinoFullUI(String deviceUUID,
                 UIControlSetup uiControlSetup = UIControlSetup(),
                 UIDisplaySetup uiDisplaySetup = UIDisplaySetup(),
                 bool isActiveLowIO = true,
                 bool allowInterruptableIO = true,
                 bool enableTcUnicodeFonts = false,
                 bool enableBufferedVRAM = false);
    virtual ~AstruinoFullUI();

    void addRemote(Astro_RemoteControl rcType,
                   UARTDeviceSetup rcSetup = UARTDeviceSetup(),
                   uint16_t rcServerPort = ASTRO_UI_REMOTESERVER_PORT);

    virtual bool isFullUI() override;
};

#endif // /ifndef AstruinoUI_H
#endif
