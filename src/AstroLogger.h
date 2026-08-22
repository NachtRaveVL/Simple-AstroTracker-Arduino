/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Logger
*/

#ifndef AstroLogger_H
#define AstroLogger_H

#include "AstroDefines.h"
#include "AstroData.h"

struct AstroLoggerSubData;

struct AstroLogEvent {
    Astro_LogLevel level;
    int64_t timestamp;
    char prefix[12];
    char message[ASTRO_LOG_MESSAGE_MAXSIZE];
};

class AstroLogger {
public:
    AstroLogger();
    void setSubData(AstroLoggerSubData *data);
    void setLogLevel(Astro_LogLevel logLevel);
    Astro_LogLevel getLogLevel() const;
    inline bool isLoggingEnabled() const { return getLogLevel() != Astro_LogLevel_None; }
    void logMessage(int64_t timestamp, const char *message);
    void logWarning(int64_t timestamp, const char *message);
    void logError(int64_t timestamp, const char *message);
    void logField(int64_t timestamp, const char *fieldName, double value, const char *units = nullptr);
    void logEnvironment(int64_t timestamp, double ambientC, double humidity, double dewPointC,
                        double opticsC = 999.0, double cameraSensorC = 999.0, double cameraBodyC = 999.0,
                        float dewHeaterPower = -1.0f, float cameraCoolingPower = -1.0f, float cameraFanPower = -1.0f);
    Signal<const AstroLogEvent, ASTRO_DEFAULT_MAXSIZE> &getLogSignal();

protected:
    Astro_LogLevel _logLevel;                               // Active log level
    AstroLoggerSubData *_data;                              // Serialized logger settings, not owned
    Signal<const AstroLogEvent, ASTRO_DEFAULT_MAXSIZE> _logSignal; // Logging signal
    void log(Astro_LogLevel level, int64_t timestamp, const char *prefix, const char *message);
};

struct AstroLoggerSubData : public AstroSubData {
    Astro_LogLevel logLevel;
    char logFilePrefix[ASTRO_PREFIX_MAXSIZE];
    bool logToSDCard;
    bool logToWiFiStorage;

    AstroLoggerSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef AstroLogger_H
