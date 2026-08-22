/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Publisher
*/

#ifndef AstroPublisher_H
#define AstroPublisher_H

#include "AstroMeasurements.h"
#include "AstroData.h"

struct AstroPublisherSubData;

struct AstroDataColumn {
    akey_t sensorKey;
    AstroSingleMeasurement measurement;

    AstroDataColumn(akey_t sensorKeyIn = akey_none,
                    AstroSingleMeasurement measurementIn = AstroSingleMeasurement())
        : sensorKey(sensorKeyIn), measurement(measurementIn) { ; }
};

class AstroPublisher {
public:
    AstroPublisher();
    inline void setSubData(AstroPublisherSubData *data) { _data = data; }
    bool addColumn(akey_t sensorKey);
    bool publishData(akey_t sensorKey, const AstroSingleMeasurement &measurement);
    bool publishData(akey_t sensorKey, double value, Astro_UnitsType units, aframe_t frame, int64_t timestamp);
    inline bool publishData(akey_t sensorKey, double value, aframe_t frame, int64_t timestamp)
        { return publishData(sensorKey, value, Astro_UnitsType_Raw_1, frame, timestamp); }
    void advancePollingFrame(aframe_t frame, int64_t timestamp);
    aposi_t getColumnIndexStart(akey_t sensorKey) const;
    inline uint8_t getColumnCount() const { return _columnCount; }
    inline const AstroDataColumn *getColumns() const { return _columns; }
    inline bool isPublishingEnabled() const { return _columnCount > 0; }
    Signal<Pair<uint8_t, const AstroDataColumn *>, ASTRO_DEFAULT_MAXSIZE> &getPublishSignal();

protected:
    AstroDataColumn _columns[ASTRO_PUBLISH_MAX_COLUMNS];    // Publisher data columns
    uint8_t _columnCount;                                   // Active column count
    aframe_t _publishedFrame;                               // Last published polling frame
    AstroPublisherSubData *_data;                           // Serialized publisher settings, not owned
    Signal<Pair<uint8_t, const AstroDataColumn *>, ASTRO_DEFAULT_MAXSIZE> _publishSignal; // Data publishing signal

    void publishIfReady(aframe_t frame, int64_t timestamp);
};

struct AstroPublisherSubData : public AstroSubData {
    char dataFilePrefix[ASTRO_PREFIX_MAXSIZE];
    bool pubToSDCard;
    bool pubToWiFiStorage;
    bool pubToMQTT;

    AstroPublisherSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef AstroPublisher_H
