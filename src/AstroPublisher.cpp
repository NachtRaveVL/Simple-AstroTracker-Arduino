/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Publisher
*/

#include "AstroPublisher.h"
#include "AstroUtils.h"
#include <stdio.h>
#include <string.h>

AstroPublisher::AstroPublisher()
    : _sink(nullptr), _context(nullptr), _columns(), _columnCount(0),
      _publishedFrame(aframe_none), _data(nullptr)
{ ; }

void AstroPublisher::setSink(PublishSink sink, void *context)
{
    _sink = sink;
    _context = context;
}

bool AstroPublisher::addColumn(akey_t sensorKey)
{
    if (!sensorKey || _columnCount >= ASTRO_PUBLISH_MAX_COLUMNS) { return false; }
    if (getColumnIndexStart(sensorKey) >= 0) { return true; }

    _columns[_columnCount++] = AstroDataColumn(sensorKey);
    return true;
}

bool AstroPublisher::publishData(akey_t sensorKey, const AstroSingleMeasurement &measurement)
{
    aposi_t columnIndex = getColumnIndexStart(sensorKey);
    if (columnIndex < 0) { return false; }

    _columns[columnIndex].measurement = measurement;
    publishIfReady(measurement.frame, measurement.timestamp);
    return true;
}

bool AstroPublisher::publishData(akey_t sensorKey, double value, Astro_UnitsType units, aframe_t frame, int64_t timestamp)
{
    return publishData(sensorKey, AstroSingleMeasurement(value, units, timestamp, frame));
}

void AstroPublisher::advancePollingFrame(aframe_t frame, int64_t timestamp)
{
    publishIfReady(frame, timestamp);
}

aposi_t AstroPublisher::getColumnIndexStart(akey_t sensorKey) const
{
    for (uint8_t columnIndex = 0; columnIndex < _columnCount; ++columnIndex) {
        if (_columns[columnIndex].sensorKey == sensorKey) { return (aposi_t)columnIndex; }
    }
    return (aposi_t)-1;
}

void AstroPublisher::publishIfReady(aframe_t frame, int64_t timestamp)
{
    if (!_sink || !_columnCount || frame == aframe_none || frame == _publishedFrame) { return; }

    for (uint8_t columnIndex = 0; columnIndex < _columnCount; ++columnIndex) {
        if (_columns[columnIndex].measurement.frame != frame) { return; }
    }

    _publishedFrame = frame;
    _sink(_context, timestamp, _columns, _columnCount);
}

AstroPublisherSubData::AstroPublisherSubData()
    : dataFilePrefix{0}, pubToSDCard(false), pubToWiFiStorage(false), pubToMQTT(false)
{
    snprintf(dataFilePrefix, sizeof(dataFilePrefix), "data/astro");
}

bool AstroPublisherSubData::toJSON(char *bufferOut, size_t bufferSize) const
{
    if (!bufferOut || !bufferSize) { return false; }
    int written = snprintf(bufferOut, bufferSize,
        "{\"dataFilePrefix\":\"%s\",\"pubToSDCard\":%s,\"pubToWiFiStorage\":%s,\"pubToMQTT\":%s}",
        dataFilePrefix, pubToSDCard ? "true" : "false", pubToWiFiStorage ? "true" : "false", pubToMQTT ? "true" : "false");
    return written >= 0 && (size_t)written < bufferSize;
}

bool AstroPublisherSubData::fromJSON(const char *jsonIn)
{
    if (!jsonIn) { return false; }
    char prefixIn[ASTRO_PREFIX_MAXSIZE] = {0};
    bool sdIn, wifiIn, mqttIn;
    if (!astroJSONGetString(jsonIn, "dataFilePrefix", prefixIn, sizeof(prefixIn)) ||
        !astroJSONGetBool(jsonIn, "pubToSDCard", &sdIn) ||
        !astroJSONGetBool(jsonIn, "pubToWiFiStorage", &wifiIn) ||
        !astroJSONGetBool(jsonIn, "pubToMQTT", &mqttIn)) { return false; }
    snprintf(dataFilePrefix, sizeof(dataFilePrefix), "%s", prefixIn);
    pubToSDCard = sdIn;
    pubToWiFiStorage = wifiIn;
    pubToMQTT = mqttIn;
    return true;
}
