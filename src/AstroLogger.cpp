/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Logger
*/

#include "Astruino.h"

AstroLogEvent::AstroLogEvent(Astro_LogLevel levelIn, const String &prefixIn, const String &msgIn, const String &suffix1In, const String &suffix2In)
    : level(levelIn), timestamp(localNow().timestamp(DateTime::TIMESTAMP_FULL)), prefix(prefixIn), msg(msgIn), suffix1(suffix1In), suffix2(suffix2In)
{ ; }


AstroLogger::AstroLogger() :
#if ASTRO_SYS_LEAVE_FILES_OPEN
    _logFileSD(nullptr),
#ifdef ASTRO_USE_WIFI_STORAGE
    _logFileWS(nullptr),
#endif
#endif
    _logFilename(), _initTime(0), _lastSpaceCheck(0)
{ ; }

AstroLogger::~AstroLogger()
{
    flush();

    #if ASTRO_SYS_LEAVE_FILES_OPEN
        if (_logFileSD) {
            _logFileSD->close();
            delete _logFileSD; _logFileSD = nullptr;
            Astroduino::_activeInstance->endSDCard();
        }
        #ifdef ASTRO_USE_WIFI_STORAGE
            if (_logFileWS) {
                _logFileWS->close();
                delete _logFileWS; _logFileWS = nullptr;
            }
        #endif
    #endif
}

bool AstroLogger::beginLoggingToSDCard(String logFilePrefix)
{
    ASTRO_SOFT_ASSERT(hasLoggerData(), SFP(AStr_Err_NotYetInitialized));

    if (hasLoggerData() && !loggerData()->logToSDCard) {
        auto sd = Astroduino::_activeInstance->getSDCard();

        if (sd) {
            String logFilename = getYYMMDDFilename(logFilePrefix, SFP(AStr_txt));
            createDirectoryFor(sd, logFilename);
            #if ASTRO_SYS_LEAVE_FILES_OPEN
                auto &logFile = _logFileSD ? *_logFileSD : *(_logFileSD = new File(sd->open(logFilename.c_str(), FILE_WRITE)));
            #else
                auto logFile = sd->open(logFilename.c_str(), FILE_WRITE);
            #endif

            if (logFile) {
                #if !ASTRO_SYS_LEAVE_FILES_OPEN
                    logFile.close();
                    Astroduino::_activeInstance->endSDCard(sd);
                #endif

                strncpy(loggerData()->logFilePrefix, logFilePrefix.c_str(), 16);
                loggerData()->logToSDCard = true;
                _logFilename = logFilename;
                Astroduino::_activeInstance->_systemData->bumpRevisionIfNeeded();

                return true;
            }
        }

        #if !ASTRO_SYS_LEAVE_FILES_OPEN
            Astroduino::_activeInstance->endSDCard(sd);
        #endif
    }

    return false;
}

#ifdef ASTRO_USE_WIFI_STORAGE

bool AstroLogger::beginLoggingToWiFiStorage(String logFilePrefix)
{
    ASTRO_SOFT_ASSERT(hasLoggerData(), SFP(AStr_Err_NotYetInitialized));

    if (hasLoggerData() && !loggerData()->logToWiFiStorage) {
        String logFilename = getYYMMDDFilename(logFilePrefix, SFP(AStr_txt));
        #if ASTRO_SYS_LEAVE_FILES_OPEN
            auto &logFile = _logFileWS ? *_logFileWS : *(_logFileWS = new WiFiStorageFile(WiFiStorage.open(logFilename.c_str())));
        #else
            auto logFile = WiFiStorage.open(logFilename.c_str());
        #endif

        if (logFile) {
            #if !ASTRO_SYS_LEAVE_FILES_OPEN
                logFile.close();
            #endif

            strncpy(loggerData()->logFilePrefix, logFilePrefix.c_str(), 16);
            loggerData()->logToWiFiStorage = true;
            _logFilename = logFilename;
            Astroduino::_activeInstance->_systemData->bumpRevisionIfNeeded();

            return true;
        }
    }

    return false;
}

#endif

void AstroLogger::logSystemUptime()
{
    TimeSpan elapsed(getSystemUptime());
    if (elapsed.totalseconds()) {
        logMessage(SFP(AStr_Log_SystemUptime), timeSpanToString(elapsed));
    }
}

void AstroLogger::logMessage(const String &msg, const String &suffix1, const String &suffix2)
{
    if (!hasLoggerData() || (loggerData()->logLevel != Astro_LogLevel_None && loggerData()->logLevel <= Astro_LogLevel_All)) {
        log(AstroLogEvent(Astro_LogLevel_Info, SFP(AStr_Log_Prefix_Info), msg, suffix1, suffix2));
    }
}

void AstroLogger::logWarning(const String &warn, const String &suffix1, const String &suffix2)
{
    if (!hasLoggerData() || (loggerData()->logLevel != Astro_LogLevel_None && loggerData()->logLevel <= Astro_LogLevel_Warnings)) {
        log(AstroLogEvent(Astro_LogLevel_Warnings, SFP(AStr_Log_Prefix_Warning), warn, suffix1, suffix2));
    }
}

void AstroLogger::logError(const String &err, const String &suffix1, const String &suffix2)
{
    if (!hasLoggerData() || (loggerData()->logLevel != Astro_LogLevel_None && loggerData()->logLevel <= Astro_LogLevel_Errors)) {
        log(AstroLogEvent(Astro_LogLevel_Errors, SFP(AStr_Log_Prefix_Error), err, suffix1, suffix2));
    }
}

void AstroLogger::log(const AstroLogEvent &event)
{
    #ifdef ASTRO_ENABLE_DEBUG_OUTPUT
        if (Serial) {
            Serial.print(event.timestamp);
            Serial.print(' ');
            Serial.print(event.prefix);
            Serial.print(event.msg);
            Serial.print(event.suffix1);
            Serial.println(event.suffix2);
        }
    #endif

    if (isLoggingToSDCard()) {
        auto sd = Astroduino::_activeInstance->getSDCard(ASTRO_LOFS_BEGIN);

        if (sd) {
            #if ASTRO_SYS_LEAVE_FILES_OPEN
                auto &logFile = _logFileSD ? *_logFileSD : *(_logFileSD = new File(sd->open(_logFilename.c_str(), FILE_WRITE)));
            #else
                createDirectoryFor(sd, _logFilename);
                auto logFile = sd->open(_logFilename.c_str(), FILE_WRITE);
            #endif

            if (logFile) {
                logFile.print(event.timestamp);
                logFile.print(' ');
                logFile.print(event.prefix);
                logFile.print(event.msg);
                logFile.print(event.suffix1);
                logFile.println(event.suffix2);

                #if !ASTRO_SYS_LEAVE_FILES_OPEN
                    logFile.flush();
                    logFile.close();
                #endif
            }

            #if !ASTRO_SYS_LEAVE_FILES_OPEN
                Astroduino::_activeInstance->endSDCard(sd);
            #endif
        }
    }

#ifdef ASTRO_USE_WIFI_STORAGE

    if (isLoggingToWiFiStorage()) {
        #if ASTRO_SYS_LEAVE_FILES_OPEN
            auto &logFile = _logFileWS ? *_logFileWS : *(_logFileWS = new WiFiStorageFile(WiFiStorage.open(_logFilename.c_str())));
        #else
            auto logFile = WiFiStorage.open(_logFilename.c_str());
        #endif

        if (logFile) {
            auto logFileStream = AstroWiFiStorageFileStream(logFile, logFile.size());

            logFileStream.print(event.timestamp);
            logFileStream.print(' ');
            logFileStream.print(event.prefix);
            logFileStream.print(event.msg);
            logFileStream.print(event.suffix1);
            logFileStream.println(event.suffix2);

            #if !ASTRO_SYS_LEAVE_FILES_OPEN
                logFileStream.flush();
                logFile.close();
            #endif
        }
    }

#endif

    #ifdef ASTRO_USE_MULTITASKING
        scheduleSignalFireOnce<const AstroLogEvent>(_logSignal, event);
    #else
        _logSignal.fire(event);
    #endif
}

void AstroLogger::flush()
{
    #ifdef ASTRO_ENABLE_DEBUG_OUTPUT
        if (Serial) { Serial.flush(); }
    #endif
    #if ASTRO_SYS_LEAVE_FILES_OPEN
        if(_logFileSD) { _logFileSD->flush(); }
    #endif
    yield();
}

void AstroLogger::setLogLevel(Astro_LogLevel logLevel)
{
    ASTRO_SOFT_ASSERT(hasLoggerData(), SFP(AStr_Err_NotYetInitialized));
    if (hasLoggerData() && loggerData()->logLevel != logLevel) {
        loggerData()->logLevel = logLevel;
        Astroduino::_activeInstance->_systemData->bumpRevisionIfNeeded();
    }
}

Signal<const AstroLogEvent, ASTRO_LOG_SIGNAL_SLOTS> &AstroLogger::getLogSignal()
{
    return _logSignal;
}

void AstroLogger::notifyDateChanged()
{
    if (isLoggingEnabled()) {
        _logFilename = getYYMMDDFilename(charsToString(loggerData()->logFilePrefix, 16), SFP(AStr_txt));
        cleanupOldestLogs();
    }
}

void AstroLogger::cleanupOldestLogs(bool force)
{
    // TODO: Old data cleanup. #17 in Astruino.
}


AstroLoggerSubData::AstroLoggerSubData()
    : AstroSubData(0), logLevel(Astro_LogLevel_All), logFilePrefix{0}, logToSDCard(false), logToWiFiStorage(false)
{ ; }

void AstroLoggerSubData::toJSONObject(JsonObject &objectOut) const
{
    //AstroSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    if (logLevel != Astro_LogLevel_All) { objectOut[SFP(AStr_Key_LogLevel)] = logLevel; }
    if (logFilePrefix[0]) { objectOut[SFP(AStr_Key_LogFilePrefix)] = charsToString(logFilePrefix, 16); }
    if (logToSDCard != false) { objectOut[SFP(AStr_Key_LogToSDCard)] = logToSDCard; }
    if (logToWiFiStorage != false) { objectOut[SFP(AStr_Key_LogToWiFiStorage)] = logToWiFiStorage; }
}

void AstroLoggerSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    //AstroSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    logLevel = objectIn[SFP(AStr_Key_LogLevel)] | logLevel;
    const char *logFilePrefixStr = objectIn[SFP(AStr_Key_LogFilePrefix)];
    if (logFilePrefixStr && logFilePrefixStr[0]) { strncpy(logFilePrefix, logFilePrefixStr, 16); }
    logToSDCard = objectIn[SFP(AStr_Key_LogToSDCard)] | logToSDCard;
    logToWiFiStorage = objectIn[SFP(AStr_Key_LogToWiFiStorage)] | logToWiFiStorage;
}
