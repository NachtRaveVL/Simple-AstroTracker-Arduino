/*  Astruino: Minimal/RO UI scaffold.
    Copyright (C) 2026 NachtRaveVL
*/

#include "AstruinoUI.h"
#ifdef ASTRO_USE_GUI

AstruinoMinUI::AstruinoMinUI(String deviceUUID, UIControlSetup uiControlSetup, UIDisplaySetup uiDisplaySetup, bool isActiveLowIO, bool allowInterruptableIO, bool enableTcUnicodeFonts, bool enableBufferedVRAM)
    : AstruinoBaseUI(deviceUUID, uiControlSetup, uiDisplaySetup, isActiveLowIO, allowInterruptableIO, enableTcUnicodeFonts, enableBufferedVRAM)
{ ; }

AstruinoMinUI::~AstruinoMinUI()
{ ; }

void AstruinoMinUI::allocateStandardControls() { /* UI stub: device allocation intentionally deferred. */ }
void AstruinoMinUI::allocateESP32TouchControl() { /* UI stub: device allocation intentionally deferred. */ }
void AstruinoMinUI::allocateResistiveTouchControl() { /* UI stub: device allocation intentionally deferred. */ }
void AstruinoMinUI::allocateTouchscreenControl() { /* UI stub: device allocation intentionally deferred. */ }
void AstruinoMinUI::allocateTFTTouchControl() { /* UI stub: device allocation intentionally deferred. */ }
void AstruinoMinUI::allocateLCDDisplay() { /* UI stub: device allocation intentionally deferred. */ }
void AstruinoMinUI::allocateSSD1305Display() { /* UI stub: device allocation intentionally deferred. */ }
void AstruinoMinUI::allocateSSD1305x32AdaDisplay() { /* UI stub: device allocation intentionally deferred. */ }
void AstruinoMinUI::allocateSSD1305x64AdaDisplay() { /* UI stub: device allocation intentionally deferred. */ }
void AstruinoMinUI::allocateSSD1306Display() { /* UI stub: device allocation intentionally deferred. */ }
void AstruinoMinUI::allocateSH1106Display() { /* UI stub: device allocation intentionally deferred. */ }
void AstruinoMinUI::allocateCustomOLEDDisplay() { /* UI stub: device allocation intentionally deferred. */ }
void AstruinoMinUI::allocateSSD1607Display() { /* UI stub: device allocation intentionally deferred. */ }
void AstruinoMinUI::allocateIL3820Display() { /* UI stub: device allocation intentionally deferred. */ }
void AstruinoMinUI::allocateIL3820V2Display() { /* UI stub: device allocation intentionally deferred. */ }
void AstruinoMinUI::allocateST7735Display() { /* UI stub: device allocation intentionally deferred. */ }
void AstruinoMinUI::allocateST7789Display() { /* UI stub: device allocation intentionally deferred. */ }
void AstruinoMinUI::allocateILI9341Display() { /* UI stub: device allocation intentionally deferred. */ }
void AstruinoMinUI::allocateTFTDisplay() { /* UI stub: device allocation intentionally deferred. */ }

void AstruinoMinUI::addSerialRemote(UARTDeviceSetup rcSetup)
{
    auto remoteControl = new AstroRemoteSerialControl(rcSetup);
    if (remoteControl && remoteControl->getConnection()) {
        if (!_remoteServer) { _remoteServer = new TcMenuRemoteServer(getApplicationInfo()); }
        if (_remoteServer) { _remoteServer->addConnection(remoteControl->getConnection()); }
        _remotes.push_back(remoteControl);
    } else if (remoteControl) { delete remoteControl; }
}

void AstruinoMinUI::addSimhubRemote(UARTDeviceSetup rcSetup)
{
    auto remoteControl = new AstroRemoteSimhubControl(rcSetup, ASTRO_UI_SIMHUB_STATUS_MENU_ID);
    if (remoteControl && remoteControl->getConnection()) {
        if (!_remoteServer) { _remoteServer = new TcMenuRemoteServer(getApplicationInfo()); }
        if (_remoteServer) { _remoteServer->addConnection(remoteControl->getConnection()); }
        _remotes.push_back(remoteControl);
    } else if (remoteControl) { delete remoteControl; }
}

void AstruinoMinUI::addWiFiRemote(uint16_t rcServerPort)
{
#ifdef ASTRO_USE_WIFI
    auto remoteControl = new AstroRemoteWiFiControl(rcServerPort);
    if (remoteControl && remoteControl->getConnection()) {
        if (!_remoteServer) { _remoteServer = new TcMenuRemoteServer(getApplicationInfo()); }
        if (_remoteServer) { _remoteServer->addConnection(remoteControl->getConnection()); }
        _remotes.push_back(remoteControl);
    } else if (remoteControl) { delete remoteControl; }
#else
    (void)rcServerPort;
#endif
}

void AstruinoMinUI::addEthernetRemote(uint16_t rcServerPort)
{
#ifdef ASTRO_USE_ETHERNET
    auto remoteControl = new AstroRemoteEthernetControl(rcServerPort);
    if (remoteControl && remoteControl->getConnection()) {
        if (!_remoteServer) { _remoteServer = new TcMenuRemoteServer(getApplicationInfo()); }
        if (_remoteServer) { _remoteServer->addConnection(remoteControl->getConnection()); }
        _remotes.push_back(remoteControl);
    } else if (remoteControl) { delete remoteControl; }
#else
    (void)rcServerPort;
#endif
}

bool AstruinoMinUI::isFullUI()
{
    return false;
}

#endif
