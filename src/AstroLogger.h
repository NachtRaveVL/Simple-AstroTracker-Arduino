/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Logger
*/

#ifndef AstroLogger_H
#define AstroLogger_H

#include "AstroDefines.h"
#include <stdint.h>

struct AstroLoggerSubData;

// Logging Event
// Logging event structure used by logger sink callbacks.
struct AstroLogEvent {
    Astro_LogLevel level;                                   // Log level
    int64_t timestamp;                                      // Event timestamp, unix/UTC when available
    char prefix[12];                                        // Generated log prefix
    char message[ASTRO_LOG_MESSAGE_MAXSIZE];                // Log message
};

// Data Logger
// Collects and reports system events while keeping the output sink optional. The sink
// callback keeps offline operation simple while allowing serial, file, network, or user
// supplied logging backends to be attached without changing the astronomy core.
class AstroLogger {
public:
    typedef void (*LogSink)(void *context, const AstroLogEvent &event);

    AstroLogger();

    // Assigns an output sink and optional user context.
    void setSink(LogSink sink, void *context = nullptr);
    // Binds logger settings to system serialization data.
    void setSubData(AstroLoggerSubData *data);

    // Changes the minimum log level accepted by the logger.
    void setLogLevel(Astro_LogLevel logLevel);
    Astro_LogLevel getLogLevel() const;
    inline bool isLoggingEnabled() const { return _sink && getLogLevel() != Astro_LogLevel_None; }

    // Standard message helpers.
    void logMessage(int64_t timestamp, const char *message);
    void logWarning(int64_t timestamp, const char *message);
    void logError(int64_t timestamp, const char *message);
    void logField(int64_t timestamp, const char *fieldName, double value, const char *units = nullptr);

    // Generates the common Astruino environment report.
    void logEnvironment(int64_t timestamp, double ambientC, double humidity, double dewPointC,
                        double opticsC = 999.0, double cameraSensorC = 999.0, double cameraBodyC = 999.0,
                        float dewHeaterPower = -1.0f, float cameraCoolingPower = -1.0f, float cameraFanPower = -1.0f); // Dew heater power

protected:
    LogSink _sink;                                          // Output sink callback
    void *_context;                                         // Output sink user context
    Astro_LogLevel _logLevel;                               // Local fallback log level
    AstroLoggerSubData *_data;                              // Bound logger serialization data, not owned

    void log(Astro_LogLevel level, int64_t timestamp, const char *prefix, const char *message);
};

// Logger Serialization Sub Data
// A part of ASYS system data, modeled after the Hydruino/Helioduino logger settings.
struct AstroLoggerSubData {
    Astro_LogLevel logLevel;                                // Log level filter, default All
    char logFilePrefix[ASTRO_PREFIX_MAXSIZE];               // Base log file prefix/folder
    bool logToSDCard;                                       // SD card logging enabled flag
    bool logToWiFiStorage;                                  // WiFiStorage logging enabled flag

    AstroLoggerSubData();
    bool toJSON(char *bufferOut, size_t bufferSize) const;
    bool fromJSON(const char *jsonIn);
};

#endif // /ifndef AstroLogger_H
