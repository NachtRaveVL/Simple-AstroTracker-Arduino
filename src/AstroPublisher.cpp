/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Publisher
*/

#include "Astruino.h"
#include <stdio.h>
#include <string.h>

AstroPublisher::AstroPublisher()
    : _columns(), _columnCount(0), _publishedFrame(aframe_none), _data(nullptr), _publishSignal()
{ ; }

bool AstroPublisher::addColumn(akey_t sensorKey)
{
    if (!sensorKey || _columnCount >= ASTRO_PUBLISH_MAX_COLUMNS) { return false; }
    if (getColumnIndexStart(sensorKey) >= 0) { return true; }
    _columns[_columnCount++] = AstroDataColumn(sensorKey);
    return true;
}

bool AstroPublisher::publishData(akey_t sensorKey, const AstroSingleMeasurement &measurement)
{
    aposi_t index = getColumnIndexStart(sensorKey);
    if (index < 0) { return false; }
    _columns[index].measurement = measurement;
    publishIfReady(measurement.frame, measurement.timestamp);
    return true;
}

bool AstroPublisher::publishData(akey_t sensorKey, double value, Astro_UnitsType units,
                                 aframe_t frame, int64_t timestamp)
{
    return publishData(sensorKey, AstroSingleMeasurement(value, units, timestamp, frame));
}

void AstroPublisher::advancePollingFrame(aframe_t frame, int64_t timestamp)
{
    publishIfReady(frame, timestamp);
}

aposi_t AstroPublisher::getColumnIndexStart(akey_t sensorKey) const
{
    for (uint8_t index = 0; index < _columnCount; ++index) {
        if (_columns[index].sensorKey == sensorKey) { return (aposi_t)index; }
    }
    return (aposi_t)-1;
}

void AstroPublisher::publishIfReady(aframe_t frame, int64_t timestamp)
{
    (void)timestamp;
    if (!_columnCount || frame == aframe_none || frame == _publishedFrame) { return; }
    for (uint8_t index = 0; index < _columnCount; ++index) {
        if (_columns[index].measurement.frame != frame) { return; }
    }
    _publishedFrame = frame;
    _publishSignal.fire(Pair<uint8_t, const AstroDataColumn *>(_columnCount, _columns));
}

Signal<Pair<uint8_t, const AstroDataColumn *>, ASTRO_DEFAULT_MAXSIZE> &AstroPublisher::getPublishSignal()
{
    return _publishSignal;
}

AstroPublisherSubData::AstroPublisherSubData()
    : AstroSubData(0), dataFilePrefix{0}, pubToSDCard(false), pubToWiFiStorage(false), pubToMQTT(false)
{
    snprintf(dataFilePrefix, sizeof(dataFilePrefix), "data/astro");
}

void AstroPublisherSubData::toJSONObject(JsonObject &objectOut) const
{
    AstroSubData::toJSONObject(objectOut);
    objectOut["dataFilePrefix"] = dataFilePrefix;
    objectOut["pubToSDCard"] = pubToSDCard;
    objectOut["pubToWiFiStorage"] = pubToWiFiStorage;
    objectOut["pubToMQTT"] = pubToMQTT;
}

void AstroPublisherSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroSubData::fromJSONObject(objectIn);
    const char *prefix = objectIn["dataFilePrefix"] | nullptr;
    if (prefix) {
        strncpy(dataFilePrefix, prefix, ASTRO_PREFIX_MAXSIZE - 1);
        dataFilePrefix[ASTRO_PREFIX_MAXSIZE - 1] = '\0';
    }
    pubToSDCard = objectIn["pubToSDCard"] | pubToSDCard;
    pubToWiFiStorage = objectIn["pubToWiFiStorage"] | pubToWiFiStorage;
    pubToMQTT = objectIn["pubToMQTT"] | pubToMQTT;
}
