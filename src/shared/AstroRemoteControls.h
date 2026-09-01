/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Remote Controls
*/

#include <Astruino.h>
#ifdef ASTRO_USE_GUI
#ifndef AstroRemoteControls_H
#define AstroRemoteControls_H

class AstroRemoteControl;
class AstroRemoteSerialControl;
class AstroRemoteSimhubControl;
#ifdef ASTRO_USE_WIFI
class AstroRemoteWiFiControl;
#endif
#ifdef ASTRO_USE_ETHERNET
class AstroRemoteEthernetControl;
#endif

#include "AstruinoUI.h"

// Remote Control Base
// Base remote control class.
class AstroRemoteControl {
public:
    virtual ~AstroRemoteControl() = default;

    // Connection accessor
    virtual BaseRemoteServerConnection *getConnection() = 0;
};


// Serial UART Remote Control
// Manages remote control over serial UART.
class AstroRemoteSerialControl : public AstroRemoteControl {
public:
    AstroRemoteSerialControl(UARTDeviceSetup serialSetup);
    virtual ~AstroRemoteSerialControl() = default;

    virtual BaseRemoteServerConnection *getConnection() override;

protected:
    SerialTagValueTransport _serialTransport;
    NoInitialisationNeeded _serialInitializer;
    TagValueRemoteServerConnection _serialConnection;
};


// Simhub Connector Remote Control
// Manages remote control over simhub connector.
class AstroRemoteSimhubControl : public AstroRemoteControl {
public:
    AstroRemoteSimhubControl(UARTDeviceSetup serialSetup, menuid_t statusMenuId);
    virtual ~AstroRemoteSimhubControl() = default;

    virtual BaseRemoteServerConnection *getConnection() override;

protected:
    SimHubRemoteConnection _simhubConnection;
};


#ifdef ASTRO_USE_WIFI
// WiFi Remote Control
// Manages remote control over a WiFi connection.
class AstroRemoteWiFiControl : public AstroRemoteControl {
public:
    AstroRemoteWiFiControl(uint16_t listeningPort = ASTRO_UI_REMOTESERVER_PORT);
    virtual ~AstroRemoteWiFiControl() = default;

    virtual BaseRemoteServerConnection *getConnection() override;

    inline WiFiServer &getRCServer() { return _rcServer; }

protected:
    WiFiServer _rcServer;
    WiFiInitialisation _netInitialisation;
    WiFiTagValTransport _netTransport;
    TagValueRemoteServerConnection _netConnection;
};
#endif


#ifdef ASTRO_USE_ETHERNET
// Ethernet Remote Control
// Manages remote control over an Ethernet connection.
class AstroRemoteEthernetControl : public AstroRemoteControl {
public:
    AstroRemoteEthernetControl(uint16_t listeningPort = ASTRO_UI_REMOTESERVER_PORT);
    virtual ~AstroRemoteEthernetControl() = default;

    virtual BaseRemoteServerConnection *getConnection() override;

    inline EthernetServer &getRCServer() { return _rcServer; }

protected:
    EthernetServer _rcServer;
    EthernetInitialisation _netInitialisation;
    EthernetTagValTransport _netTransport;
    TagValueRemoteServerConnection _netConnection;
};
#endif

#endif // /ifndef AstroRemoteControls_H
#endif
