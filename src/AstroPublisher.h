/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Publisher
*/

#ifndef AstroPublisher_H
#define AstroPublisher_H

#include "AstroMeasurements.h"

struct AstroPublisherSubData;

// Publisher Data Column
// A single sensor column collected for one polling frame.
struct AstroDataColumn {
    akey_t sensorKey;                                       // Key to sensor object
    AstroSingleMeasurement measurement;                     // Latest frame measurement

    AstroDataColumn(akey_t sensorKeyIn = akey_none,
                    AstroSingleMeasurement measurementIn = AstroSingleMeasurement())
        : sensorKey(sensorKeyIn), measurement(measurementIn)
    { ; }
};

// Data Publisher
// Collects sensor values into polling frames and emits a complete row through an optional
// sink callback. File/network backends can be layered on without making them mandatory.
class AstroPublisher {
public:
    typedef void (*PublishSink)(void *context, int64_t timestamp, const AstroDataColumn *columns, uint8_t columnCount);

    AstroPublisher();

    // Assigns output sink and optional user context.
    void setSink(PublishSink sink, void *context = nullptr);
    // Binds publishing settings to system serialization data.
    void setSubData(AstroPublisherSubData *data) { _data = data; }

    // Adds a sensor key to the publishing table.
    bool addColumn(akey_t sensorKey);
    // Publishes a sensor measurement into the active frame.
    bool publishData(akey_t sensorKey, const AstroSingleMeasurement &measurement);
    bool publishData(akey_t sensorKey, double value, Astro_UnitsType units, aframe_t frame, int64_t timestamp);
    inline bool publishData(akey_t sensorKey, double value, aframe_t frame, int64_t timestamp) { return publishData(sensorKey, value, Astro_UnitsType_Raw_1, frame, timestamp); }

    // Advances publishing when all configured columns contain the requested frame.
    void advancePollingFrame(aframe_t frame, int64_t timestamp);

    aposi_t getColumnIndexStart(akey_t sensorKey) const;
    inline uint8_t getColumnCount() const { return _columnCount; }
    inline const AstroDataColumn *getColumns() const { return _columns; }
    inline bool isPublishingEnabled() const { return _sink != nullptr; }

protected:
    PublishSink _sink;                                      // Output sink callback
    void *_context;                                         // Output sink user context
    AstroDataColumn _columns[ASTRO_PUBLISH_MAX_COLUMNS];    // Publishing columns
    uint8_t _columnCount;                                   // Number of active columns
    aframe_t _publishedFrame;                               // Last fully published polling frame
    AstroPublisherSubData *_data;                           // Bound publisher serialization data, not owned

    void publishIfReady(aframe_t frame, int64_t timestamp);
};

// Publisher Serialization Sub Data
// A part of ASYS system data, modeled after the Hydruino/Helioduino publisher settings.
struct AstroPublisherSubData {
    char dataFilePrefix[ASTRO_PREFIX_MAXSIZE];              // Base data file prefix/folder
    bool pubToSDCard;                                       // SD card publishing enabled flag
    bool pubToWiFiStorage;                                  // WiFiStorage publishing enabled flag
    bool pubToMQTT;                                         // MQTT publishing enabled flag

    AstroPublisherSubData();
    bool toJSON(char *bufferOut, size_t bufferSize) const;
    bool fromJSON(const char *jsonIn);
};

#endif // /ifndef AstroPublisher_H
