/*  Astruino: Full/RW UI scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include "AstruinoUI.h"
#ifdef ASTRO_USE_GUI

AstruinoFullUI::AstruinoFullUI(String deviceUUID, UIControlSetup uiControlSetup, UIDisplaySetup uiDisplaySetup, bool isActiveLowIO, bool allowInterruptableIO, bool enableTcUnicodeFonts, bool enableBufferedVRAM)
    : AstruinoBaseUI(deviceUUID, uiControlSetup, uiDisplaySetup, isActiveLowIO, allowInterruptableIO, enableTcUnicodeFonts, enableBufferedVRAM)
{
    // UI stub: Automatic display/input allocation is intentionally deferred with the main UI implementation.
}

AstruinoFullUI::~AstruinoFullUI()
{ ; }

void AstruinoFullUI::addRemote(Astro_RemoteControl rcType, UARTDeviceSetup rcSetup, uint16_t rcServerPort)
{
    AstroRemoteControl *remoteControl = nullptr;
    switch (rcType) {
        case Astro_RemoteControl_Serial: remoteControl = new AstroRemoteSerialControl(rcSetup); break;
        case Astro_RemoteControl_Simhub: remoteControl = new AstroRemoteSimhubControl(rcSetup, ASTRO_UI_SIMHUB_STATUS_MENU_ID); break;
#ifdef ASTRO_USE_WIFI
        case Astro_RemoteControl_WiFi: remoteControl = new AstroRemoteWiFiControl(rcServerPort); break;
#endif
#ifdef ASTRO_USE_ETHERNET
        case Astro_RemoteControl_Ethernet: remoteControl = new AstroRemoteEthernetControl(rcServerPort); break;
#endif
        default: break;
    }

    if (remoteControl && remoteControl->getConnection()) {
        if (!_remoteServer) { _remoteServer = new TcMenuRemoteServer(getApplicationInfo()); }
        if (_remoteServer) { _remoteServer->addConnection(remoteControl->getConnection()); }
        _remotes.push_back(remoteControl);
    } else if (remoteControl) {
        delete remoteControl;
    }
}

bool AstruinoFullUI::isFullUI()
{
    return true;
}

#endif
