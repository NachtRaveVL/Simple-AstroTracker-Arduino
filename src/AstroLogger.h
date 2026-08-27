/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Logger
*/

#ifndef AstroLogger_H
#define AstroLogger_H

class AstroLogger;
struct AstroLoggerSubData;

#include "Astruino.h"

// Logging Level
// Log levels that can be filtered upon if desired.
enum Astro_LogLevel : signed char {
    Astro_LogLevel_All,                                     // All (info, warn, err)
    Astro_LogLevel_Warnings,                                // Warnings & errors (warn, err)
    Astro_LogLevel_Errors,                                  // Just errors (err)
    Astro_LogLevel_None = -1,                               // None / disabled
    Astro_LogLevel_Info = Astro_LogLevel_All                // Info alias
};

// Logging Events
// Logging event structure that is used in signaling.
struct AstroLogEvent {
    Astro_LogLevel level;                                   // Log level
    String timestamp;                                       // Timestamp (generated)
    String prefix;                                          // Prefix
    String msg;                                             // Message
    String suffix1;                                         // Suffix1 (optional)
    String suffix2;                                         // Suffix2 (optional)

    AstroLogEvent(Astro_LogLevel levelIn,
                  const String &prefixIn,
                  const String &msgIn,
                  const String &suffix1In = String(),
                  const String &suffix2In = String());
};

// Data Logger
// The Logger acts as the system's event monitor that collects and reports on the various
// processes of interest inside of the system. It allows for different log levels to be
// used that can help filter out unwanted noise, as well as attempts to be more optimized
// for embedded systems by spreading string data out over multiple call parameters to
// avoid large string concatenations that can overstress and crash constrained devices.
// Logging to SD card .txt log files (via SPI card reader) is supported as is logging to
// WiFiStorage .txt log files (via OS/OTA filesystem / WiFiNINA_Generic only).
class AstroLogger {
public:
    AstroLogger();
    ~AstroLogger();

    bool beginLoggingToSDCard(String logFilePrefix);
    inline bool isLoggingToSDCard() const;

#ifdef ASTRO_USE_WIFI_STORAGE
    bool beginLoggingToWiFiStorage(String logFilePrefix);
    inline bool isLoggingToWiFiStorage() const;
#endif

    inline void logActivation(const AstroActuator *actuator);
    inline void logDeactivation(const AstroActuator *actuator);
    inline void logProcess(const AstroObjInterface *obj, const String &processString = String(), const String &statusString = String());
    inline void logStatus(const AstroObjInterface *obj, const String &statusString = String());

    void logSystemUptime();
    inline void logSystemSave() { logMessage(SFP(Str_Log_SystemDataSaved)); }

    void logMessage(const String &msg, const String &suffix1 = String(), const String &suffix2 = String());
    void logWarning(const String &warn, const String &suffix1 = String(), const String &suffix2 = String());
    void logError(const String &err, const String &suffix1 = String(), const String &suffix2 = String());
    void flush();

    void setLogLevel(Astro_LogLevel logLevel);
    inline Astro_LogLevel getLogLevel() const;

    inline bool isLoggingEnabled() const;
    inline time_t getSystemInit() const { return _initTime; }
    inline time_t getSystemUptime() const { return unixNow() - (_initTime ?: SECS_YR_2000); }

    Signal<const AstroLogEvent, ASTRO_LOG_SIGNAL_SLOTS> &getLogSignal();

    void notifyDateChanged();

protected:
#if ASTRO_SYS_LEAVE_FILES_OPEN
    File *_logFileSD;                                       // SD card log file instance (owned)
#ifdef ASTRO_USE_WIFI_STORAGE
    WiFiStorageFile *_logFileWS;                            // WiFiStorageFile log file instance (owned)
#endif
#endif
    String _logFilename;                                    // Resolved log file name (based on day)
    time_t _initTime;                                       // Time of init, for uptime (UTC)
    time_t _lastSpaceCheck;                                 // Last time enough space was checked (UTC)

    Signal<const AstroLogEvent, ASTRO_LOG_SIGNAL_SLOTS> _logSignal; // Logging signal

    friend class Astruino;
    
    void log(const AstroLogEvent &event);

public: // consider protected
    inline AstroLoggerSubData *loggerData() const;
    inline bool hasLoggerData() const;

    inline void updateInitTracking(time_t time = unixNow()) { _initTime = time; }
    void cleanupOldestLogs(bool force = false);
};

// Logger Serialization Sub Data
// A part of ASYS system data.
struct AstroLoggerSubData : public AstroSubData {
    Astro_LogLevel logLevel;                                // Log level filter (default: All)
    char logFilePrefix[ASTRO_PREFIX_MAXSIZE];               // Base log file name prefix / folder (default: "logs/he")
    bool logToSDCard;                                       // If system logging to SD card is enabled (default: false)
    bool logToWiFiStorage;                                  // If system logging to WiFiStorage is enabled (default: false)

    AstroLoggerSubData();
    void toJSONObject(JsonObject &objectOut) const;
    void fromJSONObject(JsonObjectConst &objectIn);
};

#endif // /ifndef AstroLogger_H
