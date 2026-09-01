/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Utilities
*/

#include "Astruino.h"
#include <pins_arduino.h>

bool AstroRTCWrapper<RTC_DS1307>::begin(TwoWire *wireInstance)
{
    return _rtc.begin(wireInstance);
}

void AstroRTCWrapper<RTC_DS1307>::adjust(const DateTime &dt)
{
    _rtc.adjust(dt);
}

bool AstroRTCWrapper<RTC_DS1307>::lostPower(void)
{
    return false; // not implemented
}

DateTime AstroRTCWrapper<RTC_DS1307>::now()
{
    return _rtc.now();
}


#ifdef ASTRO_USE_MULTITASKING

BasicArduinoInterruptAbstraction interruptImpl;

#endif // /ifdef ASTRO_USE_MULTITASKING


#ifdef ASTRO_USE_DEBUG_ASSERTIONS

static String fileFromFullPath(String fullPath)
{
    int index = fullPath.lastIndexOf(ASTRO_BLDPATH_SEPARATOR);
    return index != -1 ? fullPath.substring(index+1) : fullPath;
}

static String makeAssertMsg(const char *file, const char *func, int line)
{
    String retVal;

    retVal.concat(SFP(AStr_ColonSpace));
    retVal.concat(fileFromFullPath(String(file)));
    retVal.concat(':');
    retVal.concat(line);
    retVal.concat(F(" in "));
    retVal.concat(func);
    retVal.concat(SFP(AStr_ColonSpace));

    return retVal;
}

void softAssert(bool cond, String msg, const char *file, const char *func, int line)
{
    if (!cond) {
        if (getLogger()) {
            getLogger()->logWarning(SFP(AStr_Err_AssertionFailure), makeAssertMsg(file, func, line), msg);
            getLogger()->flush();
        }
        #ifdef ASTRO_ENABLE_DEBUG_OUTPUT
            else if (Serial) {
                Serial.print(localNow().timestamp(DateTime::TIMESTAMP_FULL));
                Serial.print(' ');
                Serial.print(SFP(AStr_Log_Prefix_Warning));
                Serial.print(SFP(AStr_Err_AssertionFailure));
                Serial.print(makeAssertMsg(file, func, line));
                Serial.println(msg);
                Serial.flush(); yield();
            }
        #endif
    }
}

void hardAssert(bool cond, String msg, const char *file, const char *func, int line)
{
    if (!cond) {
        String assertFail = SFP(AStr_Err_AssertionFailure);
        assertFail.concat(F(" HARD"));
        if (getLogger()) {
            getLogger()->logError(assertFail, makeAssertMsg(file, func, line), msg);
            getLogger()->flush();
        }
        #ifdef ASTRO_ENABLE_DEBUG_OUTPUT
            else if (Serial) {
                Serial.print(localNow().timestamp(DateTime::TIMESTAMP_FULL));
                Serial.print(' ');
                Serial.print(SFP(AStr_Log_Prefix_Error));
                Serial.print(assertFail);
                Serial.print(makeAssertMsg(file, func, line));
                Serial.println(msg);
                Serial.flush(); yield();
            }
        #endif

        if (getController()) { getController()->suspend(); }
        yield(); delay(10);
        abort();
    }
}

#endif // /ifdef ASTRO_USE_DEBUG_ASSERTIONS


void publishData(AstroSensor *sensor)
{
    ASTRO_HARD_ASSERT(sensor, SFP(AStr_Err_InvalidParameter));

    if (getPublisher()) {
        auto measurement = sensor->getMeasurement();
        aposi_t rows = getMeasurementRowCount(measurement);
        aposi_t columnIndexStart = getPublisher()->getColumnIndexStart(sensor->getKey());

        if (columnIndexStart >= 0) {
            for (uint8_t measurementRow = 0; measurementRow < rows; ++measurementRow) {
                getPublisher()->publishData(columnIndexStart + measurementRow, getAsSingleMeasurement(measurement, measurementRow));
            }
        }
    }
}

void _setUnixTime(DateTime unixTime, bool isSigTime)
{
    time_t prevTime = unixNow();

    auto rtc = getController() ? getController()->getRTC() : nullptr;
    if (rtc) {
        rtc->adjust(unixTime);
        getController()->notifyRTCTimeUpdated();
    }
    setTime(unixTime.unixtime());

    if (getController() && (isSigTime ||
        getLogger()->getSystemInit() <= SECS_YR_2000 ||
        abs(prevTime - unixTime.unixtime()) >= SECS_PER_DAY)) {
        getController()->notifySignificantTime(unixTime.unixtime());
    }
}

String getYYMMDDFilename(String prefix, String ext)
{
    DateTime currTime = localNow();
    uint8_t yy = currTime.year() % 100;
    uint8_t mm = currTime.month();
    uint8_t dd = currTime.day();

    String retVal; retVal.reserve(prefix.length() + 10 + 1);

    retVal.concat(prefix);
    if (yy < 10) { retVal.concat('0'); }
    retVal.concat(yy);
    if (mm < 10) { retVal.concat('0'); }
    retVal.concat(mm);
    if (dd < 10) { retVal.concat('0'); }
    retVal.concat(dd);
    retVal.concat('.');
    retVal.concat(ext);

    return retVal;
}

String getNNFilename(String prefix, unsigned int value, String ext)
{
    String retVal; retVal.reserve(prefix.length() + 6 + 1);

    retVal.concat(prefix);
    if (value < 10) { retVal.concat('0'); }
    retVal.concat(value);
    retVal.concat('.');
    retVal.concat(ext);

    return retVal;
}

void createDirectoryFor(SDClass *sd, String filename)
{
    auto slashIndex = filename.indexOf(ASTRO_FSPATH_SEPARATOR);
    String directory = slashIndex != -1 ? filename.substring(0, slashIndex) : String();
    String dirWithSep = directory + String(ASTRO_FSPATH_SEPARATOR);
    if (directory.length() && !sd->exists(dirWithSep.c_str())) {
        sd->mkdir(directory.c_str());
    }
}

akey_t stringHash(String string)
{
    akey_t hash = 5381;
    for(int index = 0; index < string.length(); ++index) {
        hash = ((hash << 5) + hash) + (akey_t)string[index]; // Good 'ol DJB2
    }
    return hash != akey_none ? hash : 5381;
}

String addressToString(uintptr_t addr)
{
    String retVal; retVal.reserve((2 * sizeof(void*)) + 2 + 1);
    if (addr == (uintptr_t)-1) { addr = 0; }
    retVal.concat('0'); retVal.concat('x');

    if (sizeof(void*) >= 4) {
        if (addr < 0x10000000) { retVal.concat('0'); }
        if (addr <  0x1000000) { retVal.concat('0'); }
        if (addr <   0x100000) { retVal.concat('0'); }
        if (addr <    0x10000) { retVal.concat('0'); }
    }
    if (sizeof(void*) >= 2) {
        if (addr <     0x1000) { retVal.concat('0'); }
        if (addr <      0x100) { retVal.concat('0'); }
    }
    if (sizeof(void*) >= 1) {
        if (addr <       0x10) { retVal.concat('0'); }
    }

    retVal.concat(String((unsigned long)addr, 16));

    return retVal;
}

String charsToString(const char *charsIn, size_t length)
{
    if (!charsIn || !length) { return String(SFP(AStr_null)); }
    String retVal; retVal.reserve(length + 1);
    for (size_t index = 0; index < length && charsIn[index] != '\000'; ++index) {
        retVal.concat(charsIn[index]);
    }
    return retVal.length() ? retVal : String(SFP(AStr_null));
}

String timeSpanToString(const TimeSpan &span)
{
    String retVal; retVal.reserve(15 + 1);

    if (span.days()) {
        retVal.concat(span.days());
        retVal.concat('d');
    }
    if (span.hours()) {
        if (retVal.length()) { retVal.concat(' '); }
        retVal.concat(span.hours());
        retVal.concat('h');
    }
    if (span.minutes()) {
        if (retVal.length()) { retVal.concat(' '); }
        retVal.concat(span.minutes());
        retVal.concat('m');
    }
    if (span.seconds()) {
        if (retVal.length()) { retVal.concat(' '); }
        retVal.concat(span.seconds());
        retVal.concat('s');
    }

    return retVal;
}

extern String measurementToString(float value, Astro_UnitsType units, unsigned int additionalDecPlaces)
{
    String retVal; retVal.reserve(15 + 1);
    retVal.concat(roundToString(value, additionalDecPlaces));

    String unitsSym = unitsTypeToSymbol(units, true);
    if (unitsSym.length()) {
        retVal.concat(' ');
        retVal.concat(unitsSym);
    }

    return retVal;
}

template<>
String commaStringFromArray<float>(const float *arrayIn, size_t length)
{
    if (!arrayIn || !length) { return String(SFP(AStr_null)); }
    String retVal; retVal.reserve(length << 1 + length >> 1 + 1);
    for (size_t index = 0; index < length; ++index) {
        if (retVal.length()) { retVal.concat(','); }

        String floatString = String(arrayIn[index], 6);
        int trimIndex = floatString.length() - 1;

        while (floatString[trimIndex] == '0' && trimIndex > 0) { trimIndex--; }
        if (floatString[trimIndex] == '.' && trimIndex > 0) { trimIndex--; }
        if (trimIndex < floatString.length() - 1) {
            floatString = floatString.substring(0, trimIndex+1);
        }

        retVal += floatString;
    }
    return retVal.length() ? retVal : String(SFP(AStr_null));
}

template<>
String commaStringFromArray<double>(const double *arrayIn, size_t length)
{
    if (!arrayIn || !length) { return String(SFP(AStr_null)); }
    String retVal; retVal.reserve(length << 1 + length >> 1 + 1);
    for (size_t index = 0; index < length; ++index) {
        if (retVal.length()) { retVal.concat(','); }

        String doubleString = String(arrayIn[index], 14);
        int trimIndex = doubleString.length() - 1;

        while (doubleString[trimIndex] == '0' && trimIndex > 0) { trimIndex--; }
        if (doubleString[trimIndex] == '.' && trimIndex > 0) { trimIndex--; }
        if (trimIndex < doubleString.length() - 1) {
            doubleString = doubleString.substring(0, trimIndex+1);
        }

        retVal += doubleString;
    }
    return retVal.length() ? retVal : String(SFP(AStr_null));
}

template<>
void commaStringToArray<float>(String stringIn, float *arrayOut, size_t length)
{
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(SFP(AStr_null))) { return; }
    int lastSepPos = -1;
    for (size_t index = 0; index < length; ++index) {
        int nextSepPos = stringIn.indexOf(',', lastSepPos+1);
        if (nextSepPos == -1) { nextSepPos = stringIn.length(); }
        String subString = stringIn.substring(lastSepPos+1, nextSepPos);
        if (nextSepPos < stringIn.length()) { lastSepPos = nextSepPos; }

        arrayOut[index] = subString.toFloat();
    }
}

template<>
void commaStringToArray<double>(String stringIn, double *arrayOut, size_t length)
{
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(SFP(AStr_null))) { return; }
    int lastSepPos = -1;
    for (size_t index = 0; index < length; ++index) {
        int nextSepPos = stringIn.indexOf(',', lastSepPos+1);
        if (nextSepPos == -1) { nextSepPos = stringIn.length(); }
        String subString = stringIn.substring(lastSepPos+1, nextSepPos);
        if (nextSepPos < stringIn.length()) { lastSepPos = nextSepPos; }

        #if !defined(CORE_TEENSY)
            arrayOut[index] = subString.toDouble();
        #else
            arrayOut[index] = subString.toFloat();
        #endif
    }
}

template<>
void commaStringToArray<String>(String stringIn, String *arrayOut, size_t length)
{
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(SFP(AStr_null))) { return; }
    int lastSepPos = -1;
    for (size_t index = 0; index < length; ++index) {
        int nextSepPos = stringIn.indexOf(',', lastSepPos+1);
        if (nextSepPos == -1) { nextSepPos = stringIn.length(); }
        String subString = stringIn.substring(lastSepPos+1, nextSepPos);
        if (nextSepPos < stringIn.length()) { lastSepPos = nextSepPos; }
        arrayOut[index] = subString;
    }
}

String hexStringFromBytes(const uint8_t *bytesIn, size_t length)
{
    if (!bytesIn || !length) { return String(SFP(AStr_null)); }
    String retVal; retVal.reserve((length << 1) + 1);
    for (size_t index = 0; index < length; ++index) {
        String valStr = String(bytesIn[index], 16);
        if (valStr.length() == 1) { retVal.concat('0'); }

        retVal.concat(valStr);
    }
    return retVal.length() ? retVal : String(SFP(AStr_null));
}

void hexStringToBytes(String stringIn, uint8_t *bytesOut, size_t length)
{
    if (!stringIn.length() || !length || stringIn.equalsIgnoreCase(SFP(AStr_null))) { return; }
    for (size_t index = 0; index < length; ++index) {
        String valStr = stringIn.substring(index << 1,(index+1) << 1);
        if (valStr.length() == 2) { bytesOut[index] = strtoul(valStr.c_str(), nullptr, 16); }
        else { bytesOut[index] = 0; }
    }
}

void hexStringToBytes(JsonVariantConst &variantIn, uint8_t *bytesOut, size_t length)
{
    if (variantIn.isNull() || variantIn.is<JsonObjectConst>() || variantIn.is<JsonArrayConst>()) { return; }
    hexStringToBytes(variantIn.as<String>(), bytesOut, length);
}

int occurrencesInString(String string, char singleChar)
{
    int retVal = 0;
    int posIndex = string.indexOf(singleChar);
    while (posIndex != -1) {
        retVal++;
        posIndex = string.indexOf(singleChar, posIndex+1);
    }
    return retVal;
}

int occurrencesInString(String string, String subString)
{
    int retVal = 0;
    int posIndex = string.indexOf(subString[0]);
    while (posIndex != -1) {
        if (subString.equals(string.substring(posIndex, posIndex + subString.length()))) {
            retVal++;
            posIndex += subString.length();
        }
        posIndex = string.indexOf(subString[0], posIndex+1);
    }
    return retVal;
}

int occurrencesInStringIgnoreCase(String string, char singleChar)
{
    int retVal = 0;
    int posIndex = min(string.indexOf(tolower(singleChar)), string.indexOf(toupper(singleChar)));
    while (posIndex != -1) {
        retVal++;
        posIndex = min(string.indexOf(tolower(singleChar), posIndex+1), string.indexOf(toupper(singleChar), posIndex+1));
    }
    return retVal;
}

int occurrencesInStringIgnoreCase(String string, String subString)
{
    int retVal = 0;
    int posIndex = min(string.indexOf(tolower(subString[0])), string.indexOf(toupper(subString[0])));
    while (posIndex != -1) {
        if (subString.equalsIgnoreCase(string.substring(posIndex, posIndex + subString.length()))) {
            retVal++;
            posIndex += subString.length();
        }
        posIndex = min(string.indexOf(tolower(subString[0]), posIndex+1), string.indexOf(toupper(subString[0]), posIndex+1));
    }
    return retVal;
}

template<>
bool arrayElementsEqual<float>(const float *arrayIn, size_t length, float value)
{
    for (size_t index = 0; index < length; ++index) {
        if (!isFPEqual(arrayIn[index], value)) {
            return false;
        }
    }
    return true;
}

template<>
bool arrayElementsEqual<double>(const double *arrayIn, size_t length, double value)
{
    for (size_t index = 0; index < length; ++index) {
        if (!isFPEqual(arrayIn[index], value)) {
            return false;
        }
    }
    return true;
}

// See: https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#elif !defined(ESP_PLATFORM)
extern char *__brkval;
#elif defined(ESP8266)
extern "C" {
#include "user_interface.h"
}
#endif

unsigned int freeMemory() {
    #if defined(ESP32)
        return esp_get_free_heap_size();
    #elif defined(ESP8266)
        return system_get_free_heap_size();
    #else
        char top;
        #ifdef __arm__
            return &top - reinterpret_cast<char*>(sbrk(0));
        #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
            return &top - __brkval;
        #else
            return __brkval ? &top - __brkval : &top - __malloc_heap_start;
        #endif
        return 0;
    #endif
}

void delayFine(millis_t duration) {
    millis_t start = millis();
    millis_t end = start + duration;

    {   millis_t left = max(0, duration - ASTRO_SYS_DELAYFINE_SPINMILLIS);
        if (left > 0) { delay(left); }
    }

    {   millis_t time = millis();
        while ((end >= start && (time < end)) ||
               (end < start && (time >= start || time < end))) {
            time = millis();
        }
    }
}

bool tryConvertUnits(float valueIn, Astro_UnitsType unitsIn, float *valueOut, Astro_UnitsType unitsOut, float convertParam)
{
    if (!valueOut || unitsOut == Astro_UnitsType_Undefined || unitsIn == unitsOut) return false;

    switch (unitsIn) {
        case Astro_UnitsType_Raw_1:
            switch (unitsOut) {
                // Known extents

                case Astro_UnitsType_Percentile_100:
                    *valueOut = valueIn * 100.0;
                    return true;

                case Astro_UnitsType_Angle_Degrees_360:
                    *valueOut = wrapBy360(valueIn * 360.0);
                    return true;

                case Astro_UnitsType_Angle_Radians_2pi:
                    *valueOut = wrapBy2Pi(valueIn * TWO_PI);
                    return true;

                default:
                    if (convertParam != FLT_UNDEF) {
                        *valueOut = valueIn * convertParam;
                        return true;
                    }
                    break;
            }
            break;

        case Astro_UnitsType_Percentile_100:
            switch (unitsOut) {
                case Astro_UnitsType_Raw_1:
                    *valueOut = valueIn / 100.0;
                    return true;

                default:
                    break;
            }
            break;

        case Astro_UnitsType_Angle_Degrees_360:
            switch (unitsOut) {
                case Astro_UnitsType_Angle_Radians_2pi:
                    *valueOut = wrapBy2Pi(valueIn * (TWO_PI / 360.0));
                    return true;

                case Astro_UnitsType_Raw_1:
                    *valueOut = valueIn / 360.0;
                    return true;

                default:
                    break;
            }
            break;

        case Astro_UnitsType_Angle_Radians_2pi:
            switch (unitsOut) {
                case Astro_UnitsType_Angle_Degrees_360:
                    *valueOut = wrapBy360(valueIn * (360.0 / TWO_PI));
                    return true;

                case Astro_UnitsType_Raw_1:
                    *valueOut = valueIn / TWO_PI;
                    return true;

                default:
                    break;
            }
            break;

        case Astro_UnitsType_Distance_Feet:
            switch (unitsOut) {
                case Astro_UnitsType_Distance_Meters:
                    *valueOut = valueIn * 0.3048;
                    return true;

                default:
                    break;
            }
            break;

        case Astro_UnitsType_Distance_Meters:
            switch (unitsOut) {
                case Astro_UnitsType_Distance_Feet:
                    *valueOut = valueIn * 3.28084;
                    return true;

                default:
                    break;
            }
            break;

        case Astro_UnitsType_Current_Amperage:
            switch (unitsOut) {
                case Astro_UnitsType_Power_Wattage:
                    if (convertParam != FLT_UNDEF) { // convertParam = rail voltage
                        *valueOut = valueIn * convertParam;
                        return true;
                    }
                break;
            }
            break;

        case Astro_UnitsType_Power_Wattage:
            switch (unitsOut) {
                case Astro_UnitsType_Current_Amperage:
                    if (convertParam != FLT_UNDEF) { // convertParam = rail voltage
                        *valueOut = valueIn / convertParam;
                        return true;
                    }
                break;
            }
            break;

        case Astro_UnitsType_Speed_FeetPerSec:
            switch (unitsOut) {
                case Astro_UnitsType_Speed_MetersPerSec:
                    *valueOut = valueIn * 0.3048;
                    return true;

                default:
                    break;
            }
            break;

        case Astro_UnitsType_Speed_MetersPerSec:
            switch (unitsOut) {
                case Astro_UnitsType_Speed_FeetPerSec:
                    *valueOut = valueIn * 3.28084;
                    return true;

                default:
                    break;
            }
            break;

        case Astro_UnitsType_Temperature_Celsius:
            switch (unitsOut) {
                case Astro_UnitsType_Temperature_Fahrenheit:
                    *valueOut = valueIn * 1.8 + 32.0;
                    return true;

                case Astro_UnitsType_Temperature_Kelvin:
                    *valueOut = valueIn + 273.15;
                    return true;

                default:
                    break;
            }
            break;

        case Astro_UnitsType_Temperature_Fahrenheit:
            switch (unitsOut) {
                case Astro_UnitsType_Temperature_Celsius:
                    *valueOut = (valueIn - 32.0) / 1.8;
                    return true;

                case Astro_UnitsType_Temperature_Kelvin:
                    *valueOut = ((valueIn + 459.67) * 5.0) / 9.0;
                    return true;

                default:
                    break;
            }
            break;

        case Astro_UnitsType_Temperature_Kelvin:
            switch (unitsOut) {
                case Astro_UnitsType_Temperature_Celsius:
                    *valueOut = valueIn - 273.15;
                    return true;

                case Astro_UnitsType_Temperature_Fahrenheit:
                    *valueOut = ((valueIn * 9.0) / 5.0) - 459.67;
                    return true;

                default:
                    break;
            }
            break;

        case Astro_UnitsType_Undefined:
            *valueOut = valueIn;
            return true;

        default:
            break;
    }

    return false;
}

Astro_UnitsType baseUnits(Astro_UnitsType units)
{
    switch (units) {
        case Astro_UnitsType_Speed_MetersPerSec:
            return Astro_UnitsType_Distance_Meters;
        case Astro_UnitsType_Speed_FeetPerSec:
            return Astro_UnitsType_Distance_Feet;
        default:
            return Astro_UnitsType_Undefined;
    }
}

Astro_UnitsType rateUnits(Astro_UnitsType units)
{
    switch (units) {
        case Astro_UnitsType_Distance_Meters:
            return Astro_UnitsType_Speed_MetersPerSec;
        case Astro_UnitsType_Distance_Feet:
            return Astro_UnitsType_Speed_FeetPerSec;
        default:
            return Astro_UnitsType_Undefined;
    }
}

Astro_UnitsType defaultUnits(Astro_UnitsCategory unitsCategory, Astro_MeasurementMode measureMode)
{
    measureMode = (measureMode == Astro_MeasurementMode_Undefined && getController() ? getController()->getMeasurementMode() : measureMode);

    switch (unitsCategory) {
        case Astro_UnitsCategory_Raw:
            return Astro_UnitsType_Raw_1;

        case Astro_UnitsCategory_Angle:
            switch (measureMode) {
                case Astro_MeasurementMode_Imperial:
                case Astro_MeasurementMode_Metric:
                    return Astro_UnitsType_Angle_Degrees_360;
                case Astro_MeasurementMode_Scientific:
                    return Astro_UnitsType_Angle_Radians_2pi;
                default:
                    return Astro_UnitsType_Undefined;
            }

        case Astro_UnitsCategory_Distance:
            switch (measureMode) {
                case Astro_MeasurementMode_Imperial:
                    return Astro_UnitsType_Distance_Feet;
                case Astro_MeasurementMode_Metric:
                case Astro_MeasurementMode_Scientific:
                    return Astro_UnitsType_Distance_Meters;
                default:
                    return Astro_UnitsType_Undefined;
            }

        case Astro_UnitsCategory_Percentile:
            return Astro_UnitsType_Percentile_100;

        case Astro_UnitsCategory_Speed:
            switch (measureMode) {
                case Astro_MeasurementMode_Imperial:
                    return Astro_UnitsType_Speed_FeetPerSec;
                case Astro_MeasurementMode_Metric:
                case Astro_MeasurementMode_Scientific:
                    return Astro_UnitsType_Speed_MetersPerSec;
                default:
                    return Astro_UnitsType_Undefined;
            }

        case Astro_UnitsCategory_Temperature:
            switch (measureMode) {
                case Astro_MeasurementMode_Imperial:
                    return Astro_UnitsType_Temperature_Fahrenheit;
                case Astro_MeasurementMode_Metric:
                    return Astro_UnitsType_Temperature_Celsius;
                case Astro_MeasurementMode_Scientific:
                    return Astro_UnitsType_Temperature_Kelvin;
                default:
                    return Astro_UnitsType_Undefined;
            }

        case Astro_UnitsCategory_Humidity:
            return Astro_UnitsType_Humidity_RH;

        case Astro_UnitsCategory_Power:
            return Astro_UnitsType_Power_Wattage;

        case Astro_UnitsCategory_Voltage:
            return Astro_UnitsType_Voltage_Volts;

        case Astro_UnitsCategory_Current:
            return Astro_UnitsType_Current_Amperage;

        case Astro_UnitsCategory_Count:
            switch (measureMode) {
                case Astro_MeasurementMode_Scientific:
                    return (Astro_UnitsType)2;
                default:
                    return (Astro_UnitsType)1;
            }

        case Astro_UnitsCategory_Undefined:
            return Astro_UnitsType_Undefined;
    }
    return Astro_UnitsType_Undefined;
}

int linksCountTravelActuators(Pair<uint8_t, Pair<AstroObject *, int8_t> *> links)
{
    int retVal = 0;

    for (aposi_t linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
        if (links.second[linksIndex].first->isActuatorType()) {
            auto actuator = static_cast<AstroActuator *>(links.second[linksIndex].first);

            if (getActuatorIsMotorFromType(actuator->getActuatorType())) {
                retVal++;
            }
        }
    }

    return retVal;
}

int linksCountActuatorsByMountAndType(Pair<uint8_t, Pair<AstroObject *, int8_t> *> links, AstroMount *mount, Astro_ActuatorType actuatorType)
{
    int retVal = 0;

    for (aposi_t linksIndex = 0; linksIndex < links.first && links.second[linksIndex].first; ++linksIndex) {
        if (links.second[linksIndex].first->isActuatorType()) {
            auto actuator = static_cast<AstroActuator *>(links.second[linksIndex].first);

            if (mount && mount->hasLinkage(actuator) && actuator->getActuatorType() == actuatorType) {
                retVal++;
            }
        }
    }

    return retVal;
}


bool checkPinIsAnalogInput(pintype_t pin)
{
    #if !defined(NUM_ANALOG_INPUTS) || NUM_ANALOG_INPUTS == 0
        return false;
    #elif defined(ESP32)
        return checkPinIsDigital(pin); // all digital pins are ADC capable
    #else
        switch (pin) {
            #if NUM_ANALOG_INPUTS > 0
                case (pintype_t)A0:
            #endif
            #if NUM_ANALOG_INPUTS > 1 && !(defined(PIN_A0) && !defined(PIN_A1))
                case (pintype_t)A1:
            #endif
            #if NUM_ANALOG_INPUTS > 2 && !(defined(PIN_A0) && !defined(PIN_A2))
                case (pintype_t)A2:
            #endif
            #if NUM_ANALOG_INPUTS > 3 && !(defined(PIN_A0) && !defined(PIN_A3))
                case (pintype_t)A3:
            #endif
            #if NUM_ANALOG_INPUTS > 4 && !(defined(PIN_A0) && !defined(PIN_A4))
                case (pintype_t)A4:
            #endif
            #if NUM_ANALOG_INPUTS > 5 && !(defined(PIN_A0) && !defined(PIN_A5))
                case (pintype_t)A5:
            #endif
            #if NUM_ANALOG_INPUTS > 6 && !(defined(PIN_A0) && !defined(PIN_A6))
                case (pintype_t)A6:
            #endif
            #if NUM_ANALOG_INPUTS > 7 && !(defined(PIN_A0) && !defined(PIN_A7))
                case (pintype_t)A7:
            #endif
            #if NUM_ANALOG_INPUTS > 8 && !(defined(PIN_A0) && !defined(PIN_A8))
                case (pintype_t)A8:
            #endif
            #if NUM_ANALOG_INPUTS > 9 && !(defined(PIN_A0) && !defined(PIN_A9))
                case (pintype_t)A9:
            #endif
            #if NUM_ANALOG_INPUTS > 10 && !(defined(PIN_A0) && !defined(PIN_A10))
                case (pintype_t)A10:
            #endif
            #if NUM_ANALOG_INPUTS > 11 && !(defined(PIN_A0) && !defined(PIN_A11))
                case (pintype_t)A11:
            #endif
            #if NUM_ANALOG_INPUTS > 12 && !(defined(PIN_A0) && !defined(PIN_A12))
                case (pintype_t)A12:
            #endif
            #if NUM_ANALOG_INPUTS > 13 && !(defined(PIN_A0) && !defined(PIN_A13))
                case (pintype_t)A13:
            #endif
            #if NUM_ANALOG_INPUTS > 14 && !(defined(PIN_A0) && !defined(PIN_A14))
                case (pintype_t)A14:
            #endif
            #if NUM_ANALOG_INPUTS > 15 && !(defined(PIN_A0) && !defined(PIN_A15))
                case (pintype_t)A15:
            #endif
            #if NUM_ANALOG_INPUTS > 16 && !(defined(PIN_A0) && !defined(PIN_A16))
                case (pintype_t)A16:
            #endif
            #if NUM_ANALOG_INPUTS > 17 && !(defined(PIN_A0) && !defined(PIN_A17))
                case (pintype_t)A17:
            #endif
            #if NUM_ANALOG_INPUTS > 18 && !(defined(PIN_A0) && !defined(PIN_A18))
                case (pintype_t)A18:
            #endif
            #if NUM_ANALOG_INPUTS > 19 && !(defined(PIN_A0) && !defined(PIN_A19))
                case (pintype_t)A19:
            #endif
            #if NUM_ANALOG_INPUTS > 20 && !(defined(PIN_A0) && !defined(PIN_A20))
                case (pintype_t)A20:
            #endif
            #if NUM_ANALOG_INPUTS > 21 && !(defined(PIN_A0) && !defined(PIN_A21))
                case (pintype_t)A21:
            #endif
                return true;

            default:
                return false;
        }
    #endif
}

bool checkPinIsAnalogOutput(pintype_t pin)
{
    #if !defined(NUM_ANALOG_OUTPUTS) || NUM_ANALOG_OUTPUTS == 0
        return false;
    #elif defined(ESP32)
        return checkPinIsDigital(pin); // all digital pins are ADC capable
    #else
        switch (pin) {
            #if NUM_ANALOG_OUTPUTS > 0
                #ifndef PIN_DAC0
                    case (pintype_t)A0:
                #else
                    case (pintype_t)DAC0:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 1
                #ifndef PIN_DAC1
                    case (pintype_t)A1:
                #else
                    case (pintype_t)DAC1:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 2
                #ifndef PIN_DAC2
                    case (pintype_t)A2:
                #else
                    case (pintype_t)DAC2:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 3
                #ifndef PIN_DAC3
                    case (pintype_t)A3:
                #else
                    case (pintype_t)DAC3:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 4
                #ifndef PIN_DAC4
                    case (pintype_t)A4:
                #else
                    case (pintype_t)DAC4:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 5
                #ifndef PIN_DAC5
                    case (pintype_t)A5:
                #else
                    case (pintype_t)DAC5:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 6
                #ifndef PIN_DAC6
                    case (pintype_t)A6:
                #else
                    case (pintype_t)DAC6:
                #endif
            #endif
            #if NUM_ANALOG_OUTPUTS > 7
                #ifndef PIN_DAC7
                    case (pintype_t)A7:
                #else
                    case (pintype_t)DAC7:
                #endif
            #endif
                return true;

            default:
                return false;
        }
    #endif
}


String systemModeToString(Astro_SystemMode systemMode, bool excludeSpecial)
{
    switch (systemMode) {
        case Astro_SystemMode_Tracking:
            return SFP(AStr_Enum_Tracking);
        case Astro_SystemMode_Balancing:
            return SFP(AStr_Enum_Balancing);
        case Astro_SystemMode_Manual:
            return SFP(AStr_Enum_Manual);
        case Astro_SystemMode_Count:
            return !excludeSpecial ? SFP(AStr_Enum_Count) : String();
        case Astro_SystemMode_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : String();
}

String measurementModeToString(Astro_MeasurementMode measurementMode, bool excludeSpecial)
{
    switch (measurementMode) {
        case Astro_MeasurementMode_Imperial:
            return SFP(AStr_Enum_Imperial);
        case Astro_MeasurementMode_Metric:
            return SFP(AStr_Enum_Metric);
        case Astro_MeasurementMode_Scientific:
            return SFP(AStr_Enum_Scientific);
        case Astro_MeasurementMode_Count:
            return !excludeSpecial ? SFP(AStr_Enum_Count) : String();
        case Astro_MeasurementMode_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : String();
}

String displayOutputModeToString(Astro_DisplayOutputMode displayOutMode, bool excludeSpecial)
{
    switch (displayOutMode) {
        case Astro_DisplayOutputMode_Disabled:
            return SFP(AStr_Disabled);
        case Astro_DisplayOutputMode_LCD16x2_EN: {
            String retVal(SFP(AStr_Enum_LCD16x2));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('E'); retVal.concat('N');
            return retVal;
        }
        case Astro_DisplayOutputMode_LCD16x2_RS: {
            String retVal(SFP(AStr_Enum_LCD16x2));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('R'); retVal.concat('S');
            return retVal;
        }
        case Astro_DisplayOutputMode_LCD20x4_EN: {
            String retVal(SFP(AStr_Enum_LCD20x4));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('E'); retVal.concat('N');
            return retVal;
        }
        case Astro_DisplayOutputMode_LCD20x4_RS: {
            String retVal(SFP(AStr_Enum_LCD20x4));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('R'); retVal.concat('S');
            return retVal;
        }
        case Astro_DisplayOutputMode_SSD1305:
            return SFP(AStr_Enum_SSD1305);
        case Astro_DisplayOutputMode_SSD1305_x32Ada:
            return SFP(AStr_Enum_SSD1305x32Ada);
        case Astro_DisplayOutputMode_SSD1305_x64Ada:
            return SFP(AStr_Enum_SSD1305x64Ada);
        case Astro_DisplayOutputMode_SSD1306:
            return SFP(AStr_Enum_SSD1306);
        case Astro_DisplayOutputMode_SH1106:
            return SFP(AStr_Enum_SH1106);
        case Astro_DisplayOutputMode_CustomOLED:
            return SFP(AStr_Enum_CustomOLED);
        case Astro_DisplayOutputMode_SSD1607:
            return SFP(AStr_Enum_SSD1607);
        case Astro_DisplayOutputMode_IL3820:
            return SFP(AStr_Enum_IL3820);
        case Astro_DisplayOutputMode_IL3820_V2:
            return SFP(AStr_Enum_IL3820V2);
        case Astro_DisplayOutputMode_ST7735:
            return SFP(AStr_Enum_ST7735);
        case Astro_DisplayOutputMode_ST7789:
            return SFP(AStr_Enum_ST7789);
        case Astro_DisplayOutputMode_ILI9341:
            return SFP(AStr_Enum_ILI9341);
        case Astro_DisplayOutputMode_TFT: {
            String retVal; retVal.reserve(3 + 1);
            retVal.concat('T'); retVal.concat('F'); retVal.concat('T');
            return retVal;
        }
        case Astro_DisplayOutputMode_Count:
            return !excludeSpecial ? SFP(AStr_Enum_Count) : String();
        case Astro_DisplayOutputMode_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : String();
}

String controlInputModeToString(Astro_ControlInputMode controlInMode, bool excludeSpecial)
{
    switch (controlInMode) {
        case Astro_ControlInputMode_Disabled:
            return SFP(AStr_Disabled);
        case Astro_ControlInputMode_RotaryEncoderOk: {
            String retVal(SFP(AStr_Enum_RotaryEncoder));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('O'); retVal.concat('k');
            return retVal;
        }
        case Astro_ControlInputMode_RotaryEncoderOkLR: {
            String retVal(SFP(AStr_Enum_RotaryEncoder));
            retVal.reserve(retVal.length() + 4 + 1);
            retVal.concat('O'); retVal.concat('k');
            retVal.concat('L'); retVal.concat('R');
            return retVal;
        }
        case Astro_ControlInputMode_UpDownButtonsOk: {
            String retVal(SFP(AStr_Enum_UpDownButtons));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('O'); retVal.concat('k');
            return retVal;
        }
        case Astro_ControlInputMode_UpDownButtonsOkLR: {
            String retVal(SFP(AStr_Enum_UpDownButtons));
            retVal.reserve(retVal.length() + 4 + 1);
            retVal.concat('O'); retVal.concat('k');
            retVal.concat('L'); retVal.concat('R');
            return retVal;
        }
        case Astro_ControlInputMode_UpDownESP32TouchOk: {
            String retVal(SFP(AStr_Enum_UpDownESP32Touch));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('O'); retVal.concat('k');
            return retVal;
        }
        case Astro_ControlInputMode_UpDownESP32TouchOkLR: {
            String retVal(SFP(AStr_Enum_UpDownESP32Touch));
            retVal.reserve(retVal.length() + 4 + 1);
            retVal.concat('O'); retVal.concat('k');
            retVal.concat('L'); retVal.concat('R');
            return retVal;
        }
        case Astro_ControlInputMode_AnalogJoystickOk: {
            String retVal(SFP(AStr_Enum_AnalogJoystick));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('O'); retVal.concat('k');
            return retVal;
        }
        case Astro_ControlInputMode_Matrix2x2UpDownButtonsOkL: {
            String retVal(SFP(AStr_Enum_Matrix2x2));
            String concat(SFP(AStr_Enum_UpDownButtons));
            retVal.reserve(retVal.length() + concat.length() + 3 + 1);
            retVal.concat(concat);
            retVal.concat('O'); retVal.concat('k');
            retVal.concat('L');
            return retVal;
        }
        case Astro_ControlInputMode_Matrix3x4Keyboard_OptRotEncOk: {
            String retVal(SFP(AStr_Enum_Matrix3x4));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('O'); retVal.concat('k');
            return retVal;
        }
        case Astro_ControlInputMode_Matrix3x4Keyboard_OptRotEncOkLR: {
            String retVal(SFP(AStr_Enum_Matrix3x4));
            retVal.reserve(retVal.length() + 4 + 1);
            retVal.concat('O'); retVal.concat('k');
            retVal.concat('L'); retVal.concat('R');
            return retVal;
        }
        case Astro_ControlInputMode_Matrix4x4Keyboard_OptRotEncOk: {
            String retVal(SFP(AStr_Enum_Matrix4x4));
            retVal.reserve(retVal.length() + 2 + 1);
            retVal.concat('O'); retVal.concat('k');
            return retVal;
        }
        case Astro_ControlInputMode_Matrix4x4Keyboard_OptRotEncOkLR: {
            String retVal(SFP(AStr_Enum_Matrix4x4));
            retVal.reserve(retVal.length() + 4 + 1);
            retVal.concat('O'); retVal.concat('k');
            retVal.concat('L'); retVal.concat('R');
            return retVal;
        }
        case Astro_ControlInputMode_ResistiveTouch:
            return SFP(AStr_Enum_ResistiveTouch);
        case Astro_ControlInputMode_TouchScreen:
            return SFP(AStr_Enum_TouchScreen);
        case Astro_ControlInputMode_TFTTouch:
            return SFP(AStr_Enum_TFTTouch);
        case Astro_ControlInputMode_RemoteControl:
            return SFP(AStr_Enum_RemoteControl);
        case Astro_ControlInputMode_Count:
            return !excludeSpecial ? SFP(AStr_Enum_Count) : String();
        case Astro_ControlInputMode_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : String();
}

String actuatorTypeToString(Astro_ActuatorType actuatorType, bool excludeSpecial)
{
    switch (actuatorType) {
        case Astro_ActuatorType_MountAxis:
            return SFP(AStr_Enum_MountAxis);
        case Astro_ActuatorType_Cover:
            return SFP(AStr_Enum_Cover);
        case Astro_ActuatorType_DewHeater:
            return SFP(AStr_Enum_DewHeater);
        case Astro_ActuatorType_CameraCooler:
            return SFP(AStr_Enum_CameraCooler);
        case Astro_ActuatorType_Fan:
            return SFP(AStr_Enum_Fan);
        case Astro_ActuatorType_Focuser:
            return SFP(AStr_Enum_Focuser);
        case Astro_ActuatorType_CameraShutter:
            return SFP(AStr_Enum_CameraShutter);
        case Astro_ActuatorType_Count:
            return !excludeSpecial ? SFP(AStr_Enum_Count) : String();
        case Astro_ActuatorType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : String();
}

String sensorTypeToString(Astro_SensorType sensorType, bool excludeSpecial)
{
    switch (sensorType) {
        case Astro_SensorType_Temperature:
            return SFP(AStr_Enum_Temperature);
        case Astro_SensorType_Humidity:
            return SFP(AStr_Enum_Humidity);
        case Astro_SensorType_Position:
            return SFP(AStr_Enum_Position);
        case Astro_SensorType_LimitSwitch:
            return SFP(AStr_Enum_LimitSwitch);
        case Astro_SensorType_Rain:
            return SFP(AStr_Enum_Rain);
        case Astro_SensorType_WindSpeed:
            return SFP(AStr_Enum_WindSpeed);
        case Astro_SensorType_Light:
            return SFP(AStr_Enum_Light);
        case Astro_SensorType_Voltage:
            return SFP(AStr_Enum_Voltage);
        case Astro_SensorType_Current:
            return SFP(AStr_Enum_Current);
        case Astro_SensorType_CameraTemperature:
            return SFP(AStr_Enum_CameraTemperature);
        case Astro_SensorType_Count:
            return !excludeSpecial ? SFP(AStr_Enum_Count) : String();
        case Astro_SensorType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : String();
}

String targetTypeToString(Astro_TargetType targetType, bool excludeSpecial)
{
    switch (targetType) {
        case Astro_TargetType_Sun:
            return SFP(AStr_Enum_Sun);
        case Astro_TargetType_Moon:
            return SFP(AStr_Enum_Moon);
        case Astro_TargetType_Mercury:
            return SFP(AStr_Enum_Mercury);
        case Astro_TargetType_Venus:
            return SFP(AStr_Enum_Venus);
        case Astro_TargetType_Mars:
            return SFP(AStr_Enum_Mars);
        case Astro_TargetType_Jupiter:
            return SFP(AStr_Enum_Jupiter);
        case Astro_TargetType_Saturn:
            return SFP(AStr_Enum_Saturn);
        case Astro_TargetType_Uranus:
            return SFP(AStr_Enum_Uranus);
        case Astro_TargetType_Neptune:
            return SFP(AStr_Enum_Neptune);
        case Astro_TargetType_Sirius:
            return SFP(AStr_Enum_Sirius);
        case Astro_TargetType_Canopus:
            return SFP(AStr_Enum_Canopus);
        case Astro_TargetType_Arcturus:
            return SFP(AStr_Enum_Arcturus);
        case Astro_TargetType_Vega:
            return SFP(AStr_Enum_Vega);
        case Astro_TargetType_Capella:
            return SFP(AStr_Enum_Capella);
        case Astro_TargetType_RigelKentaurus:
            return SFP(AStr_Enum_RigelKentaurus);
        case Astro_TargetType_Procyon:
            return SFP(AStr_Enum_Procyon);
        case Astro_TargetType_Betelgeuse:
            return SFP(AStr_Enum_Betelgeuse);
        case Astro_TargetType_Achernar:
            return SFP(AStr_Enum_Achernar);
        case Astro_TargetType_Hadar:
            return SFP(AStr_Enum_Hadar);
        case Astro_TargetType_Altair:
            return SFP(AStr_Enum_Altair);
        case Astro_TargetType_Acrux:
            return SFP(AStr_Enum_Acrux);
        case Astro_TargetType_Aldebaran:
            return SFP(AStr_Enum_Aldebaran);
        case Astro_TargetType_Spica:
            return SFP(AStr_Enum_Spica);
        case Astro_TargetType_Antares:
            return SFP(AStr_Enum_Antares);
        case Astro_TargetType_Pollux:
            return SFP(AStr_Enum_Pollux);
        case Astro_TargetType_Fomalhaut:
            return SFP(AStr_Enum_Fomalhaut);
        case Astro_TargetType_Deneb:
            return SFP(AStr_Enum_Deneb);
        case Astro_TargetType_Regulus:
            return SFP(AStr_Enum_Regulus);
        case Astro_TargetType_Polaris:
            return SFP(AStr_Enum_Polaris);
        case Astro_TargetType_Castor:
            return SFP(AStr_Enum_Castor);
        case Astro_TargetType_Bellatrix:
            return SFP(AStr_Enum_Bellatrix);
        case Astro_TargetType_Alnilam:
            return SFP(AStr_Enum_Alnilam);
        case Astro_TargetType_Alnitak:
            return SFP(AStr_Enum_Alnitak);
        case Astro_TargetType_Mizar:
            return SFP(AStr_Enum_Mizar);
        case Astro_TargetType_Dubhe:
            return SFP(AStr_Enum_Dubhe);
        case Astro_TargetType_M1:
            return SFP(AStr_Enum_M1);
        case Astro_TargetType_M2:
            return SFP(AStr_Enum_M2);
        case Astro_TargetType_M3:
            return SFP(AStr_Enum_M3);
        case Astro_TargetType_M4:
            return SFP(AStr_Enum_M4);
        case Astro_TargetType_M5:
            return SFP(AStr_Enum_M5);
        case Astro_TargetType_M6:
            return SFP(AStr_Enum_M6);
        case Astro_TargetType_M7:
            return SFP(AStr_Enum_M7);
        case Astro_TargetType_M8:
            return SFP(AStr_Enum_M8);
        case Astro_TargetType_M9:
            return SFP(AStr_Enum_M9);
        case Astro_TargetType_M10:
            return SFP(AStr_Enum_M10);
        case Astro_TargetType_M11:
            return SFP(AStr_Enum_M11);
        case Astro_TargetType_M12:
            return SFP(AStr_Enum_M12);
        case Astro_TargetType_M13:
            return SFP(AStr_Enum_M13);
        case Astro_TargetType_M14:
            return SFP(AStr_Enum_M14);
        case Astro_TargetType_M15:
            return SFP(AStr_Enum_M15);
        case Astro_TargetType_M16:
            return SFP(AStr_Enum_M16);
        case Astro_TargetType_M17:
            return SFP(AStr_Enum_M17);
        case Astro_TargetType_M18:
            return SFP(AStr_Enum_M18);
        case Astro_TargetType_M19:
            return SFP(AStr_Enum_M19);
        case Astro_TargetType_M20:
            return SFP(AStr_Enum_M20);
        case Astro_TargetType_M21:
            return SFP(AStr_Enum_M21);
        case Astro_TargetType_M22:
            return SFP(AStr_Enum_M22);
        case Astro_TargetType_M23:
            return SFP(AStr_Enum_M23);
        case Astro_TargetType_M24:
            return SFP(AStr_Enum_M24);
        case Astro_TargetType_M25:
            return SFP(AStr_Enum_M25);
        case Astro_TargetType_M26:
            return SFP(AStr_Enum_M26);
        case Astro_TargetType_M27:
            return SFP(AStr_Enum_M27);
        case Astro_TargetType_M28:
            return SFP(AStr_Enum_M28);
        case Astro_TargetType_M29:
            return SFP(AStr_Enum_M29);
        case Astro_TargetType_M30:
            return SFP(AStr_Enum_M30);
        case Astro_TargetType_M31:
            return SFP(AStr_Enum_M31);
        case Astro_TargetType_M32:
            return SFP(AStr_Enum_M32);
        case Astro_TargetType_M33:
            return SFP(AStr_Enum_M33);
        case Astro_TargetType_M34:
            return SFP(AStr_Enum_M34);
        case Astro_TargetType_M35:
            return SFP(AStr_Enum_M35);
        case Astro_TargetType_M36:
            return SFP(AStr_Enum_M36);
        case Astro_TargetType_M37:
            return SFP(AStr_Enum_M37);
        case Astro_TargetType_M38:
            return SFP(AStr_Enum_M38);
        case Astro_TargetType_M39:
            return SFP(AStr_Enum_M39);
        case Astro_TargetType_M40:
            return SFP(AStr_Enum_M40);
        case Astro_TargetType_M41:
            return SFP(AStr_Enum_M41);
        case Astro_TargetType_M42:
            return SFP(AStr_Enum_M42);
        case Astro_TargetType_M43:
            return SFP(AStr_Enum_M43);
        case Astro_TargetType_M44:
            return SFP(AStr_Enum_M44);
        case Astro_TargetType_M45:
            return SFP(AStr_Enum_M45);
        case Astro_TargetType_M46:
            return SFP(AStr_Enum_M46);
        case Astro_TargetType_M47:
            return SFP(AStr_Enum_M47);
        case Astro_TargetType_M48:
            return SFP(AStr_Enum_M48);
        case Astro_TargetType_M49:
            return SFP(AStr_Enum_M49);
        case Astro_TargetType_M50:
            return SFP(AStr_Enum_M50);
        case Astro_TargetType_M51:
            return SFP(AStr_Enum_M51);
        case Astro_TargetType_M52:
            return SFP(AStr_Enum_M52);
        case Astro_TargetType_M53:
            return SFP(AStr_Enum_M53);
        case Astro_TargetType_M54:
            return SFP(AStr_Enum_M54);
        case Astro_TargetType_M55:
            return SFP(AStr_Enum_M55);
        case Astro_TargetType_M56:
            return SFP(AStr_Enum_M56);
        case Astro_TargetType_M57:
            return SFP(AStr_Enum_M57);
        case Astro_TargetType_M58:
            return SFP(AStr_Enum_M58);
        case Astro_TargetType_M59:
            return SFP(AStr_Enum_M59);
        case Astro_TargetType_M60:
            return SFP(AStr_Enum_M60);
        case Astro_TargetType_M61:
            return SFP(AStr_Enum_M61);
        case Astro_TargetType_M62:
            return SFP(AStr_Enum_M62);
        case Astro_TargetType_M63:
            return SFP(AStr_Enum_M63);
        case Astro_TargetType_M64:
            return SFP(AStr_Enum_M64);
        case Astro_TargetType_M65:
            return SFP(AStr_Enum_M65);
        case Astro_TargetType_M66:
            return SFP(AStr_Enum_M66);
        case Astro_TargetType_M67:
            return SFP(AStr_Enum_M67);
        case Astro_TargetType_M68:
            return SFP(AStr_Enum_M68);
        case Astro_TargetType_M69:
            return SFP(AStr_Enum_M69);
        case Astro_TargetType_M70:
            return SFP(AStr_Enum_M70);
        case Astro_TargetType_M71:
            return SFP(AStr_Enum_M71);
        case Astro_TargetType_M72:
            return SFP(AStr_Enum_M72);
        case Astro_TargetType_M73:
            return SFP(AStr_Enum_M73);
        case Astro_TargetType_M74:
            return SFP(AStr_Enum_M74);
        case Astro_TargetType_M75:
            return SFP(AStr_Enum_M75);
        case Astro_TargetType_M76:
            return SFP(AStr_Enum_M76);
        case Astro_TargetType_M77:
            return SFP(AStr_Enum_M77);
        case Astro_TargetType_M78:
            return SFP(AStr_Enum_M78);
        case Astro_TargetType_M79:
            return SFP(AStr_Enum_M79);
        case Astro_TargetType_M80:
            return SFP(AStr_Enum_M80);
        case Astro_TargetType_M81:
            return SFP(AStr_Enum_M81);
        case Astro_TargetType_M82:
            return SFP(AStr_Enum_M82);
        case Astro_TargetType_M83:
            return SFP(AStr_Enum_M83);
        case Astro_TargetType_M84:
            return SFP(AStr_Enum_M84);
        case Astro_TargetType_M85:
            return SFP(AStr_Enum_M85);
        case Astro_TargetType_M86:
            return SFP(AStr_Enum_M86);
        case Astro_TargetType_M87:
            return SFP(AStr_Enum_M87);
        case Astro_TargetType_M88:
            return SFP(AStr_Enum_M88);
        case Astro_TargetType_M89:
            return SFP(AStr_Enum_M89);
        case Astro_TargetType_M90:
            return SFP(AStr_Enum_M90);
        case Astro_TargetType_M91:
            return SFP(AStr_Enum_M91);
        case Astro_TargetType_M92:
            return SFP(AStr_Enum_M92);
        case Astro_TargetType_M93:
            return SFP(AStr_Enum_M93);
        case Astro_TargetType_M94:
            return SFP(AStr_Enum_M94);
        case Astro_TargetType_M95:
            return SFP(AStr_Enum_M95);
        case Astro_TargetType_M96:
            return SFP(AStr_Enum_M96);
        case Astro_TargetType_M97:
            return SFP(AStr_Enum_M97);
        case Astro_TargetType_M98:
            return SFP(AStr_Enum_M98);
        case Astro_TargetType_M99:
            return SFP(AStr_Enum_M99);
        case Astro_TargetType_M100:
            return SFP(AStr_Enum_M100);
        case Astro_TargetType_M101:
            return SFP(AStr_Enum_M101);
        case Astro_TargetType_M102:
            return SFP(AStr_Enum_M102);
        case Astro_TargetType_M103:
            return SFP(AStr_Enum_M103);
        case Astro_TargetType_M104:
            return SFP(AStr_Enum_M104);
        case Astro_TargetType_M105:
            return SFP(AStr_Enum_M105);
        case Astro_TargetType_M106:
            return SFP(AStr_Enum_M106);
        case Astro_TargetType_M107:
            return SFP(AStr_Enum_M107);
        case Astro_TargetType_M108:
            return SFP(AStr_Enum_M108);
        case Astro_TargetType_M109:
            return SFP(AStr_Enum_M109);
        case Astro_TargetType_M110:
            return SFP(AStr_Enum_M110);
        case Astro_TargetType_CustomTarget1:
            return SFP(AStr_Enum_CustomTarget1);
        case Astro_TargetType_CustomTarget2:
            return SFP(AStr_Enum_CustomTarget2);
        case Astro_TargetType_CustomTarget3:
            return SFP(AStr_Enum_CustomTarget3);
        case Astro_TargetType_CustomTarget4:
            return SFP(AStr_Enum_CustomTarget4);
        case Astro_TargetType_CustomTarget5:
            return SFP(AStr_Enum_CustomTarget5);
        case Astro_TargetType_CustomTarget6:
            return SFP(AStr_Enum_CustomTarget6);
        case Astro_TargetType_CustomTarget7:
            return SFP(AStr_Enum_CustomTarget7);
        case Astro_TargetType_CustomTarget8:
            return SFP(AStr_Enum_CustomTarget8);
        case Astro_TargetType_Count:
            return !excludeSpecial ? SFP(AStr_Enum_Count) : String();
        case Astro_TargetType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : String();
}

String targetClassToString(Astro_TargetClass targetClass, bool excludeSpecial)
{
    switch (targetClass) {
        case Astro_TargetClass_Star:
            return SFP(AStr_Enum_Star);
        case Astro_TargetClass_OpenCluster:
            return SFP(AStr_Enum_OpenCluster);
        case Astro_TargetClass_GlobularCluster:
            return SFP(AStr_Enum_GlobularCluster);
        case Astro_TargetClass_Nebula:
            return SFP(AStr_Enum_Nebula);
        case Astro_TargetClass_PlanetaryNebula:
            return SFP(AStr_Enum_PlanetaryNebula);
        case Astro_TargetClass_Galaxy:
            return SFP(AStr_Enum_Galaxy);
        case Astro_TargetClass_SolarSystem:
            return SFP(AStr_Enum_SolarSystem);
        case Astro_TargetClass_Other:
            return SFP(AStr_Enum_Other);
        case Astro_TargetClass_Count:
            return !excludeSpecial ? SFP(AStr_Enum_Count) : String();
        case Astro_TargetClass_Unknown:
            break;
    }
    return !excludeSpecial ? SFP(AStr_Enum_Unknown) : String();
}

aposi_t getMountAxisCountFromType(Astro_MountType mountType)
{
    switch (mountType) {
        case Astro_MountType_SingleAxis:
            return 1;
        case Astro_MountType_Equatorial:
        case Astro_MountType_AltAzimuth:
            return 2;
        default:
            return 0;
    }
}

String mountTypeToString(Astro_MountType mountType, bool excludeSpecial)
{
    switch (mountType) {
        case Astro_MountType_Equatorial:
            return SFP(AStr_Enum_Equatorial);
        case Astro_MountType_AltAzimuth:
            return SFP(AStr_Enum_AltAzimuth);
        case Astro_MountType_SingleAxis:
            return SFP(AStr_Enum_SingleAxis);
        case Astro_MountType_Count:
            return !excludeSpecial ? SFP(AStr_Enum_Count) : String();
        case Astro_MountType_Unknown:
            break;
    }
    return !excludeSpecial ? SFP(AStr_Enum_Unknown) : String();
}

float getRailVoltageFromType(Astro_RailType railType)
{
    switch (railType) {
        case Astro_RailType_AC110V:
            return 110.0f;
        case Astro_RailType_AC220V:
            return 220.0f;
        case Astro_RailType_DC3V3:
            return 3.3f;
        case Astro_RailType_DC5V:
            return 5.0f;
        case Astro_RailType_DC12V:
            return 12.0f;
        case Astro_RailType_DC24V:
            return 24.0f;
        case Astro_RailType_DC48V:
            return 48.0f;
        default:
            return 0.0f;
    }
}

String railTypeToString(Astro_RailType railType, bool excludeSpecial)
{
    switch (railType) {
        case Astro_RailType_AC110V:
            return SFP(AStr_Enum_AC110V);
        case Astro_RailType_AC220V:
            return SFP(AStr_Enum_AC220V);
        case Astro_RailType_DC3V3:
            return SFP(AStr_Enum_DC3V3);
        case Astro_RailType_DC5V:
            return SFP(AStr_Enum_DC5V);
        case Astro_RailType_DC12V:
            return SFP(AStr_Enum_DC12V);
        case Astro_RailType_DC24V:
            return SFP(AStr_Enum_DC24V);
        case Astro_RailType_DC48V:
            return SFP(AStr_Enum_DC48V);
        case Astro_RailType_Count:
            return !excludeSpecial ? SFP(AStr_Enum_Count) : String();
        case Astro_RailType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : String();
}

String pinModeToString(Astro_PinMode pinMode, bool excludeSpecial)
{
    switch (pinMode) {
        case Astro_PinMode_Digital_Input:
            return SFP(AStr_Enum_DigitalInput);
        case Astro_PinMode_Digital_Input_PullUp:
            return SFP(AStr_Enum_DigitalInputPullUp);
        case Astro_PinMode_Digital_Input_PullDown:
            return SFP(AStr_Enum_DigitalInputPullDown);
        case Astro_PinMode_Digital_Output:
            return SFP(AStr_Enum_DigitalOutput);
        case Astro_PinMode_Digital_Output_PushPull:
            return SFP(AStr_Enum_DigitalOutputPushPull);
        case Astro_PinMode_Analog_Input:
            return SFP(AStr_Enum_AnalogInput);
        case Astro_PinMode_Analog_Output:
            return SFP(AStr_Enum_AnalogOutput);
        case Astro_PinMode_Count:
            return !excludeSpecial ? SFP(AStr_Enum_Count) : String();
        case Astro_PinMode_Undefined:
            break;
        default:
            return String((int)pinMode);
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : String();
}

String enableModeToString(Astro_EnableMode enableMode, bool excludeSpecial)
{
    switch (enableMode) {
        case Astro_EnableMode_Highest:
            return SFP(AStr_Enum_Highest);
        case Astro_EnableMode_Lowest:
            return SFP(AStr_Enum_Lowest);
        case Astro_EnableMode_Average:
            return SFP(AStr_Enum_Average);
        case Astro_EnableMode_Multiply:
            return SFP(AStr_Enum_Multiply);
        case Astro_EnableMode_InOrder:
            return SFP(AStr_Enum_InOrder);
        case Astro_EnableMode_RevOrder:
            return SFP(AStr_Enum_RevOrder);
        case Astro_EnableMode_DescOrder:
            return SFP(AStr_Enum_DescOrder);
        case Astro_EnableMode_AscOrder:
            return SFP(AStr_Enum_AscOrder);
        case Astro_EnableMode_Count:
            return !excludeSpecial ? SFP(AStr_Enum_Count) : String();
        case Astro_EnableMode_Undefined:
            break;
        default:
            return String((int)enableMode);
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : String();
}

String unitsCategoryToString(Astro_UnitsCategory unitsCategory, bool excludeSpecial)
{
    switch (unitsCategory) {
        case Astro_UnitsCategory_Raw:
            return SFP(AStr_Enum_Raw);
        case Astro_UnitsCategory_Angle:
            return SFP(AStr_Enum_Angle);
        case Astro_UnitsCategory_Distance:
            return SFP(AStr_Enum_Distance);
        case Astro_UnitsCategory_Percentile:
            return SFP(AStr_Enum_Percentile);
        case Astro_UnitsCategory_Speed:
            return SFP(AStr_Enum_Speed);
        case Astro_UnitsCategory_Temperature:
            return SFP(AStr_Enum_Temperature);
        case Astro_UnitsCategory_Humidity:
            return SFP(AStr_Enum_Humidity);
        case Astro_UnitsCategory_Power:
            return SFP(AStr_Enum_Power);
        case Astro_UnitsCategory_Voltage:
            return SFP(AStr_Enum_Voltage);
        case Astro_UnitsCategory_Current:
            return SFP(AStr_Enum_Current);
        case Astro_UnitsCategory_Count:
            return !excludeSpecial ? SFP(AStr_Enum_Count) : String();
        case Astro_UnitsCategory_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(AStr_Undefined) : String();
}

String unitsTypeToSymbol(Astro_UnitsType unitsType, bool excludeSpecial)
{
    switch (unitsType) {
        case Astro_UnitsType_Raw_1:
            return SFP(AStr_raw);
        case Astro_UnitsType_Angle_Degrees_360:
            return SFP(AStr_Unit_Degree);
        case Astro_UnitsType_Angle_Radians_2pi: {
            String retVal(SFP(AStr_Unit_Degree));
            String concat(SFP(AStr_Unit_Radians));
            retVal.reserve(retVal.length() + concat.length() + 1);
            retVal.concat(concat);
            return retVal;
        }
        case Astro_UnitsType_Distance_Meters:
            return String('m');
        case Astro_UnitsType_Distance_Feet:
            return SFP(AStr_Unit_Feet);
        case Astro_UnitsType_Percentile_100:
            return String('%');
        case Astro_UnitsType_Speed_MetersPerSec: {
            String retVal('m');
            String concat(SFP(AStr_Unit_PerSecond));
            retVal.reserve(retVal.length() + concat.length() + 1);
            retVal.concat(concat);
            return retVal;
        }
        case Astro_UnitsType_Speed_FeetPerSec: {
            String retVal(SFP(AStr_Unit_Feet));
            String concat(SFP(AStr_Unit_PerSecond));
            retVal.reserve(retVal.length() + concat.length() + 1);
            retVal.concat(concat);
            return retVal;
        }
        case Astro_UnitsType_Temperature_Celsius: {
            String retVal(SFP(AStr_Unit_Degree));
            retVal.reserve(retVal.length() + 1 + 1);
            retVal.concat('C');
            return retVal;
        }
        case Astro_UnitsType_Temperature_Fahrenheit: {
            String retVal(SFP(AStr_Unit_Degree));
            retVal.reserve(retVal.length() + 1 + 1);
            retVal.concat('F');
            return retVal;
        }
        case Astro_UnitsType_Temperature_Kelvin: {
            String retVal(SFP(AStr_Unit_Degree));
            retVal.reserve(retVal.length() + 1 + 1);
            retVal.concat('K');
            return retVal;
        }
        case Astro_UnitsType_Humidity_RH:
            return String(F("%RH"));
        case Astro_UnitsType_Power_Wattage:
            return String('W'); // alt: J/s
        case Astro_UnitsType_Voltage_Volts:
            return String('V');
        case Astro_UnitsType_Current_Amperage:
            return String('A');
        case Astro_UnitsType_Count:
            return !excludeSpecial ? SFP(AStr_Unit_Count) : String();
        case Astro_UnitsType_Undefined:
            break;
    }
    return !excludeSpecial ? SFP(AStr_Unit_Undefined) : String();
}

String positionIndexToString(aposi_t positionIndex, bool excludeSpecial)
{
    if (positionIndex >= 0 && positionIndex < ASTRO_POS_MAXSIZE) {
        return String(positionIndex + ASTRO_POS_EXPORT_BEGFROM);
    } else if (!excludeSpecial) {
        if (positionIndex == ASTRO_POS_MAXSIZE) {
            return SFP(AStr_Enum_Count);
        } else {
            return SFP(AStr_Undefined);
        }
    }
    return String();
}

aposi_t positionIndexFromString(String positionIndexStr)
{
    if (positionIndexStr == positionIndexToString(ASTRO_POS_MAXSIZE)) {
        return ASTRO_POS_MAXSIZE;
    } else if (positionIndexStr == positionIndexToString(-1)) {
        return -1;
    } else {
        int8_t decode = positionIndexStr.toInt();
        return decode >= 0 && decode < ASTRO_POS_MAXSIZE ? decode : -1;
    }
}


// All remaining methods generated from minimum spanning trie

Astro_SystemMode systemModeFromString(String systemModeStr)
{
    switch (systemModeStr.length() >= 1 ? systemModeStr[0] : '\000') {
        case 'B':
            return Astro_SystemMode_Balancing;
        case 'C':
            return Astro_SystemMode_Count;
        case 'M':
            return Astro_SystemMode_Manual;
        case 'T':
            return Astro_SystemMode_Tracking;
        case 'U':
            return Astro_SystemMode_Undefined;
    }
    return Astro_SystemMode_Undefined;
}

Astro_MeasurementMode measurementModeFromString(String measurementModeStr)
{
    switch (measurementModeStr.length() >= 1 ? measurementModeStr[0] : '\000') {
        case 'C':
            return Astro_MeasurementMode_Count;
        case 'I':
            return Astro_MeasurementMode_Imperial;
        case 'M':
            return Astro_MeasurementMode_Metric;
        case 'S':
            return Astro_MeasurementMode_Scientific;
        case 'U':
            return Astro_MeasurementMode_Undefined;
    }
    return Astro_MeasurementMode_Undefined;
}

Astro_DisplayOutputMode displayOutputModeFromString(String displayOutModeStr)
{
    switch (displayOutModeStr.length() >= 1 ? displayOutModeStr[0] : '\000') {
        case 'C':
            switch (displayOutModeStr.length() >= 2 ? displayOutModeStr[1] : '\000') {
                case 'o':
                    return Astro_DisplayOutputMode_Count;
                case 'u':
                    return Astro_DisplayOutputMode_CustomOLED;
            }
            break;
        case 'D':
            return Astro_DisplayOutputMode_Disabled;
        case 'I':
            switch (displayOutModeStr.length() >= 3 ? displayOutModeStr[2] : '\000') {
                case '3':
                    switch (displayOutModeStr.length() >= 7 ? displayOutModeStr[6] : '\000') {
                        case 'V':
                            return Astro_DisplayOutputMode_IL3820_V2;
                        case '\000':
                            return Astro_DisplayOutputMode_IL3820;
                    }
                    break;
                case 'I':
                    return Astro_DisplayOutputMode_ILI9341;
            }
            break;
        case 'L':
            switch (displayOutModeStr.length() >= 4 ? displayOutModeStr[3] : '\000') {
                case '1':
                    switch (displayOutModeStr.length() >= 8 ? displayOutModeStr[7] : '\000') {
                        case 'E':
                            return Astro_DisplayOutputMode_LCD16x2_EN;
                        case 'R':
                            return Astro_DisplayOutputMode_LCD16x2_RS;
                    }
                    break;
                case '2':
                    switch (displayOutModeStr.length() >= 8 ? displayOutModeStr[7] : '\000') {
                        case 'E':
                            return Astro_DisplayOutputMode_LCD20x4_EN;
                        case 'R':
                            return Astro_DisplayOutputMode_LCD20x4_RS;
                    }
                    break;
            }
            break;
        case 'S':
            switch (displayOutModeStr.length() >= 2 ? displayOutModeStr[1] : '\000') {
                case 'H':
                    return Astro_DisplayOutputMode_SH1106;
                case 'S':
                    switch (displayOutModeStr.length() >= 5 ? displayOutModeStr[4] : '\000') {
                        case '3':
                            switch (displayOutModeStr.length() >= 7 ? displayOutModeStr[6] : '\000') {
                                case '5':
                                    switch (displayOutModeStr.length() >= 8 ? displayOutModeStr[7] : '\000') {
                                        case 'x':
                                            switch (displayOutModeStr.length() >= 9 ? displayOutModeStr[8] : '\000') {
                                                case '3':
                                                    return Astro_DisplayOutputMode_SSD1305_x32Ada;
                                                case '6':
                                                    return Astro_DisplayOutputMode_SSD1305_x64Ada;
                                            }
                                            break;
                                        case '\000':
                                            return Astro_DisplayOutputMode_SSD1305;
                                    }
                                    break;
                                case '6':
                                    return Astro_DisplayOutputMode_SSD1306;
                            }
                            break;
                        case '6':
                            return Astro_DisplayOutputMode_SSD1607;
                    }
                    break;
                case 'T':
                    switch (displayOutModeStr.length() >= 5 ? displayOutModeStr[4] : '\000') {
                        case '3':
                            return Astro_DisplayOutputMode_ST7735;
                        case '8':
                            return Astro_DisplayOutputMode_ST7789;
                    }
                    break;
            }
            break;
        case 'T':
            return Astro_DisplayOutputMode_TFT;
        case 'U':
            return Astro_DisplayOutputMode_Undefined;
    }
    return Astro_DisplayOutputMode_Undefined;
}

Astro_ControlInputMode controlInputModeFromString(String controlInModeStr)
{
    switch (controlInModeStr.length() >= 1 ? controlInModeStr[0] : '\000') {
        case 'A':
            return Astro_ControlInputMode_AnalogJoystickOk;
        case 'C':
            return Astro_ControlInputMode_Count;
        case 'D':
            return Astro_ControlInputMode_Disabled;
        case 'M':
            switch (controlInModeStr.length() >= 7 ? controlInModeStr[6] : '\000') {
                case '2':
                    return Astro_ControlInputMode_Matrix2x2UpDownButtonsOkL;
                case '3':
                    switch (controlInModeStr.length() >= 12 ? controlInModeStr[11] : '\000') {
                        case 'L':
                            return Astro_ControlInputMode_Matrix3x4Keyboard_OptRotEncOkLR;
                        case '\000':
                            return Astro_ControlInputMode_Matrix3x4Keyboard_OptRotEncOk;
                    }
                    break;
                case '4':
                    switch (controlInModeStr.length() >= 12 ? controlInModeStr[11] : '\000') {
                        case 'L':
                            return Astro_ControlInputMode_Matrix4x4Keyboard_OptRotEncOkLR;
                        case '\000':
                            return Astro_ControlInputMode_Matrix4x4Keyboard_OptRotEncOk;
                    }
                    break;
            }
            break;
        case 'R':
            switch (controlInModeStr.length() >= 2 ? controlInModeStr[1] : '\000') {
                case 'e':
                    switch (controlInModeStr.length() >= 3 ? controlInModeStr[2] : '\000') {
                        case 'm':
                            return Astro_ControlInputMode_RemoteControl;
                        case 's':
                            return Astro_ControlInputMode_ResistiveTouch;
                    }
                    break;
                case 'o':
                    switch (controlInModeStr.length() >= 16 ? controlInModeStr[15] : '\000') {
                        case 'L':
                            return Astro_ControlInputMode_RotaryEncoderOkLR;
                        case '\000':
                            return Astro_ControlInputMode_RotaryEncoderOk;
                    }
                    break;
            }
            break;
        case 'T':
            switch (controlInModeStr.length() >= 2 ? controlInModeStr[1] : '\000') {
                case 'F':
                    return Astro_ControlInputMode_TFTTouch;
                case 'o':
                    return Astro_ControlInputMode_TouchScreen;
            }
            break;
        case 'U':
            switch (controlInModeStr.length() >= 2 ? controlInModeStr[1] : '\000') {
                case 'n':
                    return Astro_ControlInputMode_Undefined;
                case 'p':
                    switch (controlInModeStr.length() >= 7 ? controlInModeStr[6] : '\000') {
                        case 'B':
                            switch (controlInModeStr.length() >= 16 ? controlInModeStr[15] : '\000') {
                                case 'L':
                                    return Astro_ControlInputMode_UpDownButtonsOkLR;
                                case '\000':
                                    return Astro_ControlInputMode_UpDownButtonsOk;
                            }
                            break;
                        case 'E':
                            switch (controlInModeStr.length() >= 19 ? controlInModeStr[18] : '\000') {
                                case 'L':
                                    return Astro_ControlInputMode_UpDownESP32TouchOkLR;
                                case '\000':
                                    return Astro_ControlInputMode_UpDownESP32TouchOk;
                            }
                            break;
                    }
                    break;
            }
            break;
    }
    return Astro_ControlInputMode_Undefined;
}

Astro_ActuatorType actuatorTypeFromString(String actuatorTypeStr)
{
    switch (actuatorTypeStr.length() >= 1 ? actuatorTypeStr[0] : '\000') {
        case 'C':
            switch (actuatorTypeStr.length() >= 2 ? actuatorTypeStr[1] : '\000') {
                case 'a':
                    switch (actuatorTypeStr.length() >= 7 ? actuatorTypeStr[6] : '\000') {
                        case 'C':
                            return Astro_ActuatorType_CameraCooler;
                        case 'S':
                            return Astro_ActuatorType_CameraShutter;
                    }
                    break;
                case 'o':
                    switch (actuatorTypeStr.length() >= 3 ? actuatorTypeStr[2] : '\000') {
                        case 'u':
                            return Astro_ActuatorType_Count;
                        case 'v':
                            return Astro_ActuatorType_Cover;
                    }
                    break;
            }
            break;
        case 'D':
            return Astro_ActuatorType_DewHeater;
        case 'F':
            switch (actuatorTypeStr.length() >= 2 ? actuatorTypeStr[1] : '\000') {
                case 'a':
                    return Astro_ActuatorType_Fan;
                case 'o':
                    return Astro_ActuatorType_Focuser;
            }
            break;
        case 'M':
            return Astro_ActuatorType_MountAxis;
        case 'U':
            return Astro_ActuatorType_Undefined;
    }
    return Astro_ActuatorType_Undefined;
}

Astro_SensorType sensorTypeFromString(String sensorTypeStr)
{
    switch (sensorTypeStr.length() >= 1 ? sensorTypeStr[0] : '\000') {
        case 'C':
            switch (sensorTypeStr.length() >= 2 ? sensorTypeStr[1] : '\000') {
                case 'a':
                    return Astro_SensorType_CameraTemperature;
                case 'o':
                    return Astro_SensorType_Count;
                case 'u':
                    return Astro_SensorType_Current;
            }
            break;
        case 'H':
            return Astro_SensorType_Humidity;
        case 'L':
            switch (sensorTypeStr.length() >= 3 ? sensorTypeStr[2] : '\000') {
                case 'g':
                    return Astro_SensorType_Light;
                case 'm':
                    return Astro_SensorType_LimitSwitch;
            }
            break;
        case 'P':
            return Astro_SensorType_Position;
        case 'R':
            return Astro_SensorType_Rain;
        case 'T':
            return Astro_SensorType_Temperature;
        case 'U':
            return Astro_SensorType_Undefined;
        case 'V':
            return Astro_SensorType_Voltage;
        case 'W':
            return Astro_SensorType_WindSpeed;
    }
    return Astro_SensorType_Undefined;
}

Astro_TargetType targetTypeFromString(String targetTypeStr)
{
    switch (targetTypeStr.length() >= 1 ? targetTypeStr[0] : '\000') {
        case 'A':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'c':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case 'h':
                            return Astro_TargetType_Achernar;
                        case 'r':
                            return Astro_TargetType_Acrux;
                    }
                    break;
                case 'l':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case 'd':
                            return Astro_TargetType_Aldebaran;
                        case 'n':
                            switch (targetTypeStr.length() >= 5 ? targetTypeStr[4] : '\000') {
                                case 'l':
                                    return Astro_TargetType_Alnilam;
                                case 't':
                                    return Astro_TargetType_Alnitak;
                            }
                            break;
                        case 't':
                            return Astro_TargetType_Altair;
                    }
                    break;
                case 'n':
                    return Astro_TargetType_Antares;
                case 'r':
                    return Astro_TargetType_Arcturus;
            }
            break;
        case 'B':
            switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                case 'l':
                    return Astro_TargetType_Bellatrix;
                case 't':
                    return Astro_TargetType_Betelgeuse;
            }
            break;
        case 'C':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'a':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case 'n':
                            return Astro_TargetType_Canopus;
                        case 'p':
                            return Astro_TargetType_Capella;
                        case 's':
                            return Astro_TargetType_Castor;
                    }
                    break;
                case 'o':
                    return Astro_TargetType_Count;
                case 'u':
                    switch (targetTypeStr.length() >= 13 ? targetTypeStr[12] : '\000') {
                        case '1':
                            return Astro_TargetType_CustomTarget1;
                        case '2':
                            return Astro_TargetType_CustomTarget2;
                        case '3':
                            return Astro_TargetType_CustomTarget3;
                        case '4':
                            return Astro_TargetType_CustomTarget4;
                        case '5':
                            return Astro_TargetType_CustomTarget5;
                        case '6':
                            return Astro_TargetType_CustomTarget6;
                        case '7':
                            return Astro_TargetType_CustomTarget7;
                        case '8':
                            return Astro_TargetType_CustomTarget8;
                    }
                    break;
            }
            break;
        case 'D':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'e':
                    return Astro_TargetType_Deneb;
                case 'u':
                    return Astro_TargetType_Dubhe;
            }
            break;
        case 'F':
            return Astro_TargetType_Fomalhaut;
        case 'H':
            return Astro_TargetType_Hadar;
        case 'J':
            return Astro_TargetType_Jupiter;
        case 'M':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case '1':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case '0':
                            switch (targetTypeStr.length() >= 4 ? targetTypeStr[3] : '\000') {
                                case '0':
                                    return Astro_TargetType_M100;
                                case '1':
                                    return Astro_TargetType_M101;
                                case '2':
                                    return Astro_TargetType_M102;
                                case '3':
                                    return Astro_TargetType_M103;
                                case '4':
                                    return Astro_TargetType_M104;
                                case '5':
                                    return Astro_TargetType_M105;
                                case '6':
                                    return Astro_TargetType_M106;
                                case '7':
                                    return Astro_TargetType_M107;
                                case '8':
                                    return Astro_TargetType_M108;
                                case '9':
                                    return Astro_TargetType_M109;
                                case '\000':
                                    return Astro_TargetType_M10;
                            }
                            break;
                        case '1':
                            switch (targetTypeStr.length() >= 4 ? targetTypeStr[3] : '\000') {
                                case '0':
                                    return Astro_TargetType_M110;
                                case '\000':
                                    return Astro_TargetType_M11;
                            }
                            break;
                        case '2':
                            return Astro_TargetType_M12;
                        case '3':
                            return Astro_TargetType_M13;
                        case '4':
                            return Astro_TargetType_M14;
                        case '5':
                            return Astro_TargetType_M15;
                        case '6':
                            return Astro_TargetType_M16;
                        case '7':
                            return Astro_TargetType_M17;
                        case '8':
                            return Astro_TargetType_M18;
                        case '9':
                            return Astro_TargetType_M19;
                        case '\000':
                            return Astro_TargetType_M1;
                    }
                    break;
                case '2':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case '0':
                            return Astro_TargetType_M20;
                        case '1':
                            return Astro_TargetType_M21;
                        case '2':
                            return Astro_TargetType_M22;
                        case '3':
                            return Astro_TargetType_M23;
                        case '4':
                            return Astro_TargetType_M24;
                        case '5':
                            return Astro_TargetType_M25;
                        case '6':
                            return Astro_TargetType_M26;
                        case '7':
                            return Astro_TargetType_M27;
                        case '8':
                            return Astro_TargetType_M28;
                        case '9':
                            return Astro_TargetType_M29;
                        case '\000':
                            return Astro_TargetType_M2;
                    }
                    break;
                case '3':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case '0':
                            return Astro_TargetType_M30;
                        case '1':
                            return Astro_TargetType_M31;
                        case '2':
                            return Astro_TargetType_M32;
                        case '3':
                            return Astro_TargetType_M33;
                        case '4':
                            return Astro_TargetType_M34;
                        case '5':
                            return Astro_TargetType_M35;
                        case '6':
                            return Astro_TargetType_M36;
                        case '7':
                            return Astro_TargetType_M37;
                        case '8':
                            return Astro_TargetType_M38;
                        case '9':
                            return Astro_TargetType_M39;
                        case '\000':
                            return Astro_TargetType_M3;
                    }
                    break;
                case '4':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case '0':
                            return Astro_TargetType_M40;
                        case '1':
                            return Astro_TargetType_M41;
                        case '2':
                            return Astro_TargetType_M42;
                        case '3':
                            return Astro_TargetType_M43;
                        case '4':
                            return Astro_TargetType_M44;
                        case '5':
                            return Astro_TargetType_M45;
                        case '6':
                            return Astro_TargetType_M46;
                        case '7':
                            return Astro_TargetType_M47;
                        case '8':
                            return Astro_TargetType_M48;
                        case '9':
                            return Astro_TargetType_M49;
                        case '\000':
                            return Astro_TargetType_M4;
                    }
                    break;
                case '5':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case '0':
                            return Astro_TargetType_M50;
                        case '1':
                            return Astro_TargetType_M51;
                        case '2':
                            return Astro_TargetType_M52;
                        case '3':
                            return Astro_TargetType_M53;
                        case '4':
                            return Astro_TargetType_M54;
                        case '5':
                            return Astro_TargetType_M55;
                        case '6':
                            return Astro_TargetType_M56;
                        case '7':
                            return Astro_TargetType_M57;
                        case '8':
                            return Astro_TargetType_M58;
                        case '9':
                            return Astro_TargetType_M59;
                        case '\000':
                            return Astro_TargetType_M5;
                    }
                    break;
                case '6':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case '0':
                            return Astro_TargetType_M60;
                        case '1':
                            return Astro_TargetType_M61;
                        case '2':
                            return Astro_TargetType_M62;
                        case '3':
                            return Astro_TargetType_M63;
                        case '4':
                            return Astro_TargetType_M64;
                        case '5':
                            return Astro_TargetType_M65;
                        case '6':
                            return Astro_TargetType_M66;
                        case '7':
                            return Astro_TargetType_M67;
                        case '8':
                            return Astro_TargetType_M68;
                        case '9':
                            return Astro_TargetType_M69;
                        case '\000':
                            return Astro_TargetType_M6;
                    }
                    break;
                case '7':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case '0':
                            return Astro_TargetType_M70;
                        case '1':
                            return Astro_TargetType_M71;
                        case '2':
                            return Astro_TargetType_M72;
                        case '3':
                            return Astro_TargetType_M73;
                        case '4':
                            return Astro_TargetType_M74;
                        case '5':
                            return Astro_TargetType_M75;
                        case '6':
                            return Astro_TargetType_M76;
                        case '7':
                            return Astro_TargetType_M77;
                        case '8':
                            return Astro_TargetType_M78;
                        case '9':
                            return Astro_TargetType_M79;
                        case '\000':
                            return Astro_TargetType_M7;
                    }
                    break;
                case '8':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case '0':
                            return Astro_TargetType_M80;
                        case '1':
                            return Astro_TargetType_M81;
                        case '2':
                            return Astro_TargetType_M82;
                        case '3':
                            return Astro_TargetType_M83;
                        case '4':
                            return Astro_TargetType_M84;
                        case '5':
                            return Astro_TargetType_M85;
                        case '6':
                            return Astro_TargetType_M86;
                        case '7':
                            return Astro_TargetType_M87;
                        case '8':
                            return Astro_TargetType_M88;
                        case '9':
                            return Astro_TargetType_M89;
                        case '\000':
                            return Astro_TargetType_M8;
                    }
                    break;
                case '9':
                    switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                        case '0':
                            return Astro_TargetType_M90;
                        case '1':
                            return Astro_TargetType_M91;
                        case '2':
                            return Astro_TargetType_M92;
                        case '3':
                            return Astro_TargetType_M93;
                        case '4':
                            return Astro_TargetType_M94;
                        case '5':
                            return Astro_TargetType_M95;
                        case '6':
                            return Astro_TargetType_M96;
                        case '7':
                            return Astro_TargetType_M97;
                        case '8':
                            return Astro_TargetType_M98;
                        case '9':
                            return Astro_TargetType_M99;
                        case '\000':
                            return Astro_TargetType_M9;
                    }
                    break;
                case 'a':
                    return Astro_TargetType_Mars;
                case 'e':
                    return Astro_TargetType_Mercury;
                case 'i':
                    return Astro_TargetType_Mizar;
                case 'o':
                    return Astro_TargetType_Moon;
            }
            break;
        case 'N':
            return Astro_TargetType_Neptune;
        case 'P':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'o':
                    switch (targetTypeStr.length() >= 4 ? targetTypeStr[3] : '\000') {
                        case 'a':
                            return Astro_TargetType_Polaris;
                        case 'l':
                            return Astro_TargetType_Pollux;
                    }
                    break;
                case 'r':
                    return Astro_TargetType_Procyon;
            }
            break;
        case 'R':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'e':
                    return Astro_TargetType_Regulus;
                case 'i':
                    return Astro_TargetType_RigelKentaurus;
            }
            break;
        case 'S':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'a':
                    return Astro_TargetType_Saturn;
                case 'i':
                    return Astro_TargetType_Sirius;
                case 'p':
                    return Astro_TargetType_Spica;
                case 'u':
                    return Astro_TargetType_Sun;
            }
            break;
        case 'U':
            switch (targetTypeStr.length() >= 2 ? targetTypeStr[1] : '\000') {
                case 'n':
                    return Astro_TargetType_Undefined;
                case 'r':
                    return Astro_TargetType_Uranus;
            }
            break;
        case 'V':
            switch (targetTypeStr.length() >= 3 ? targetTypeStr[2] : '\000') {
                case 'g':
                    return Astro_TargetType_Vega;
                case 'n':
                    return Astro_TargetType_Venus;
            }
            break;
    }
    return Astro_TargetType_Undefined;
}

Astro_TargetClass targetClassFromString(String targetClassStr)
{
    switch (targetClassStr.length() >= 1 ? targetClassStr[0] : '\000') {
        case 'C':
            return Astro_TargetClass_Count;
        case 'G':
            switch (targetClassStr.length() >= 2 ? targetClassStr[1] : '\000') {
                case 'a':
                    return Astro_TargetClass_Galaxy;
                case 'l':
                    return Astro_TargetClass_GlobularCluster;
            }
            break;
        case 'N':
            return Astro_TargetClass_Nebula;
        case 'O':
            switch (targetClassStr.length() >= 2 ? targetClassStr[1] : '\000') {
                case 'p':
                    return Astro_TargetClass_OpenCluster;
                case 't':
                    return Astro_TargetClass_Other;
            }
            break;
        case 'P':
            return Astro_TargetClass_PlanetaryNebula;
        case 'S':
            switch (targetClassStr.length() >= 2 ? targetClassStr[1] : '\000') {
                case 'o':
                    return Astro_TargetClass_SolarSystem;
                case 't':
                    return Astro_TargetClass_Star;
            }
            break;
        case 'U':
            return Astro_TargetClass_Unknown;
    }
    return Astro_TargetClass_Unknown;
}

Astro_MountType mountTypeFromString(String mountTypeStr)
{
    switch (mountTypeStr.length() >= 1 ? mountTypeStr[0] : '\000') {
        case 'A':
            return Astro_MountType_AltAzimuth;
        case 'C':
            return Astro_MountType_Count;
        case 'E':
            return Astro_MountType_Equatorial;
        case 'S':
            return Astro_MountType_SingleAxis;
        case 'U':
            return Astro_MountType_Unknown;
    }
    return Astro_MountType_Unknown;
}

Astro_RailType railTypeFromString(String railTypeStr)
{
    switch (railTypeStr.length() >= 1 ? railTypeStr[0] : '\000') {
        case 'A':
            switch (railTypeStr.length() >= 3 ? railTypeStr[2] : '\000') {
                case '1':
                    return Astro_RailType_AC110V;
                case '2':
                    return Astro_RailType_AC220V;
            }
            break;
        case 'C':
            return Astro_RailType_Count;
        case 'D':
            switch (railTypeStr.length() >= 3 ? railTypeStr[2] : '\000') {
                case '1':
                    return Astro_RailType_DC12V;
                case '2':
                    return Astro_RailType_DC24V;
                case '3':
                    return Astro_RailType_DC3V3;
                case '4':
                    return Astro_RailType_DC48V;
                case '5':
                    return Astro_RailType_DC5V;
            }
            break;
        case 'U':
            return Astro_RailType_Undefined;
    }
    return Astro_RailType_Undefined;
}

Astro_PinMode pinModeFromString(String pinModeStr)
{
    switch (pinModeStr.length() >= 1 ? pinModeStr[0] : '\000') {
        case 'A':
            switch (pinModeStr.length() >= 7 ? pinModeStr[6] : '\000') {
                case 'I':
                    return Astro_PinMode_Analog_Input;
                case 'O':
                    return Astro_PinMode_Analog_Output;
            }
            break;
        case 'C':
            return Astro_PinMode_Count;
        case 'D':
            switch (pinModeStr.length() >= 8 ? pinModeStr[7] : '\000') {
                case 'I':
                    switch (pinModeStr.length() >= 13 ? pinModeStr[12] : '\000') {
                        case 'P':
                            switch (pinModeStr.length() >= 17 ? pinModeStr[16] : '\000') {
                                case 'D':
                                    return Astro_PinMode_Digital_Input_PullDown;
                                case 'U':
                                    return Astro_PinMode_Digital_Input_PullUp;
                            }
                            break;
                        case '\000':
                            return Astro_PinMode_Digital_Input;
                    }
                    break;
                case 'O':
                    switch (pinModeStr.length() >= 14 ? pinModeStr[13] : '\000') {
                        case 'P':
                            return Astro_PinMode_Digital_Output_PushPull;
                        case '\000':
                            return Astro_PinMode_Digital_Output;
                    }
                    break;
            }
            break;
        case 'U':
            return Astro_PinMode_Undefined;
    }
    return Astro_PinMode_Undefined;
}

Astro_EnableMode enableModeFromString(String enableModeStr)
{
    switch (enableModeStr.length() >= 1 ? enableModeStr[0] : '\000') {
        case 'A':
            switch (enableModeStr.length() >= 2 ? enableModeStr[1] : '\000') {
                case 's':
                    return Astro_EnableMode_AscOrder;
                case 'v':
                    return Astro_EnableMode_Average;
            }
            break;
        case 'C':
            return Astro_EnableMode_Count;
        case 'D':
            return Astro_EnableMode_DescOrder;
        case 'H':
            return Astro_EnableMode_Highest;
        case 'I':
            return Astro_EnableMode_InOrder;
        case 'L':
            return Astro_EnableMode_Lowest;
        case 'M':
            return Astro_EnableMode_Multiply;
        case 'R':
            return Astro_EnableMode_RevOrder;
        case 'U':
            return Astro_EnableMode_Undefined;
    }
    return Astro_EnableMode_Undefined;
}

Astro_UnitsCategory unitsCategoryFromString(String unitsCategoryStr)
{
    switch (unitsCategoryStr.length() >= 1 ? unitsCategoryStr[0] : '\000') {
        case 'A':
            return Astro_UnitsCategory_Angle;
        case 'C':
            switch (unitsCategoryStr.length() >= 2 ? unitsCategoryStr[1] : '\000') {
                case 'o':
                    return Astro_UnitsCategory_Count;
                case 'u':
                    return Astro_UnitsCategory_Current;
            }
            break;
        case 'D':
            return Astro_UnitsCategory_Distance;
        case 'H':
            return Astro_UnitsCategory_Humidity;
        case 'P':
            switch (unitsCategoryStr.length() >= 2 ? unitsCategoryStr[1] : '\000') {
                case 'e':
                    return Astro_UnitsCategory_Percentile;
                case 'o':
                    return Astro_UnitsCategory_Power;
            }
            break;
        case 'R':
            return Astro_UnitsCategory_Raw;
        case 'S':
            return Astro_UnitsCategory_Speed;
        case 'T':
            return Astro_UnitsCategory_Temperature;
        case 'U':
            return Astro_UnitsCategory_Undefined;
        case 'V':
            return Astro_UnitsCategory_Voltage;
    }
    return Astro_UnitsCategory_Undefined;
}

Astro_UnitsType unitsTypeFromSymbol(String unitsSymbolStr)
{
    switch (unitsSymbolStr.length() >= 1 ? unitsSymbolStr[0] : '\000') {
        case '%':
            switch (unitsSymbolStr.length() >= 2 ? unitsSymbolStr[1] : '\000') {
                case 'R':
                    return Astro_UnitsType_Humidity_RH;
                case '\000':
                    return Astro_UnitsType_Percentile_100;
            }
            break;
        case 'A':
            return Astro_UnitsType_Current_Amperage;
        case 'J':
            return Astro_UnitsType_Power_Wattage;
        case 'V':
            return Astro_UnitsType_Voltage_Volts;
        case 'W':
            return Astro_UnitsType_Power_Wattage;
        case '[':
            switch (unitsSymbolStr.length() >= 2 ? unitsSymbolStr[1] : '\000') {
                case 'q':
                    return Astro_UnitsType_Count;
                case 'u':
                    return Astro_UnitsType_Undefined;
            }
            break;
        case 'f':
            switch (unitsSymbolStr.length() >= 3 ? unitsSymbolStr[2] : '\000') {
                case '/':
                    return Astro_UnitsType_Speed_FeetPerSec;
                case '\000':
                    return Astro_UnitsType_Distance_Feet;
            }
            break;
        case 'm':
            switch (unitsSymbolStr.length() >= 2 ? unitsSymbolStr[1] : '\000') {
                case '/':
                    return Astro_UnitsType_Speed_MetersPerSec;
                case '\000':
                    return Astro_UnitsType_Distance_Meters;
            }
            break;
        case 'r':
            switch (unitsSymbolStr.length() >= 3 ? unitsSymbolStr[2] : '\000') {
                case 'd':
                    return Astro_UnitsType_Angle_Radians_2pi;
                case 'w':
                    return Astro_UnitsType_Raw_1;
            }
            break;
        default:
            switch (unitsSymbolStr.length() >= 3 ? unitsSymbolStr[2] : '\000') {
                case '\000':
                    return Astro_UnitsType_Angle_Degrees_360;
                case 'C':
                    return Astro_UnitsType_Temperature_Celsius;
                case 'F':
                    return Astro_UnitsType_Temperature_Fahrenheit;
                case 'K':
                    return Astro_UnitsType_Temperature_Kelvin;
                case 'r':
                    return Astro_UnitsType_Angle_Radians_2pi;
            }
            break;
    }
    return Astro_UnitsType_Undefined;
}
