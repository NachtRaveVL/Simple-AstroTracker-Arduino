/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Remote Controls
*/

#include "AstruinoUI.h"
#ifdef ASTRO_USE_GUI

AstroRemoteSerialControl::AstroRemoteSerialControl(UARTDeviceSetup serialSetup)
    : _serialTransport(serialSetup.serial), _serialInitializer(), _serialConnection(_serialTransport, _serialInitializer)
{ ; }

BaseRemoteServerConnection *AstroRemoteSerialControl::getConnection()
{
    return &_serialConnection;
}


AstroRemoteSimhubControl::AstroRemoteSimhubControl(UARTDeviceSetup serialSetup, menuid_t statusMenuId)
    : _simhubConnection(serialSetup.serial, statusMenuId)
{ ; }

BaseRemoteServerConnection *AstroRemoteSimhubControl::getConnection()
{
    return &_simhubConnection;
}


#ifdef ASTRO_USE_WIFI

AstroRemoteWiFiControl::AstroRemoteWiFiControl(uint16_t listeningPort)
    : _rcServer(listeningPort), _netInitialisation(&_rcServer), _netTransport(), _netConnection(_netTransport, _netInitialisation)
{ ; }

BaseRemoteServerConnection *AstroRemoteWiFiControl::getConnection()
{
    return &_netConnection;
}

#endif // /ifdef ASTRO_USE_WIFI


#ifdef ASTRO_USE_ETHERNET

AstroRemoteEthernetControl::AstroRemoteEthernetControl(uint16_t listeningPort)
    : _rcServer(listeningPort), _netInitialisation(&_rcServer), _netTransport(), _netConnection(_netTransport, _netInitialisation)
{ ; }

BaseRemoteServerConnection *AstroRemoteEthernetControl::getConnection()
{
    return &_netConnection;
}

#endif // /ifdef ASTRO_USE_ETHERNET

#endif
