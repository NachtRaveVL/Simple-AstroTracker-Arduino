/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Utilities
*/

#ifndef AstroUtils_H
#define AstroUtils_H

template<typename RTCType> class AstroRTCWrapper;
#ifdef ASTRO_USE_MULTITASKING
template<typename ParameterType, int Slots> class SignalFireTask;
template<class ObjectType, typename ParameterType> class MethodSlotCallTask;
#endif

#include "Astruino.h"
#include "AstroObject.h"
#ifdef ASTRO_USE_MULTITASKING
#include "BasicInterruptAbstraction.h"
#endif

// Simple wrapper class for dealing with RTC modules.
// This class is mainly used to abstract which RTC module is used.
template<typename RTCType>
class AstroRTCWrapper : public AstroRTCInterface {
public:
    virtual bool begin(TwoWire *wireInstance) override { return _rtc.begin(wireInstance); }
    virtual void adjust(const DateTime &dt) override { _rtc.adjust(dt); }
    virtual bool lostPower(void) override { return _rtc.lostPower(); }
    virtual DateTime now() override { return _rtc.now(); }
protected:
    RTCType _rtc;
};

// Specialization for older DS1307 that doesn't have lost power tracking.
template<>
class AstroRTCWrapper<RTC_DS1307> : public AstroRTCInterface {
public:
    virtual bool begin(TwoWire *wireInstance) override;
    virtual void adjust(const DateTime &dt) override;
    virtual bool lostPower(void) override;
    virtual DateTime now() override;
protected:
    RTC_DS1307 _rtc;
};

// Scheduling

#ifdef ASTRO_USE_MULTITASKING

// Standard interrupt abstraction
extern BasicArduinoInterruptAbstraction interruptImpl;


// This will schedule an actuator to enable on the next TaskManagerIO runloop using the given intensity and enable time millis.
// Actuator is captured. Returns taskId or TASKMGR_INVALIDID on error.
taskid_t scheduleActuatorTimedEnableOnce(SharedPtr<AstroActuator> actuator, float intensity, time_t duration);

// This will schedule an actuator to enable on the next TaskManagerIO runloop using the given enable time millis.
// Actuator is captured. Returns taskId or TASKMGR_INVALIDID on error.
taskid_t scheduleActuatorTimedEnableOnce(SharedPtr<AstroActuator> actuator, time_t duration);

// This will schedule a signal's fire method on the next TaskManagerIO runloop using the given call/fire parameter.
// Object is captured, if not nullptr. Returns taskId or TASKMGR_INVALIDID on error.
template<typename ParameterType, int Slots> taskid_t scheduleSignalFireOnce(SharedPtr<AstroObjInterface> object, Signal<ParameterType,Slots> &signal, ParameterType fireParam);

// This will schedule a signal's fire method on the next TaskManagerIO runloop using the given call/fire parameter, w/o capturing object.
// Returns taskId or TASKMGR_INVALIDID on error.
template<typename ParameterType, int Slots> taskid_t scheduleSignalFireOnce(Signal<ParameterType,Slots> &signal, ParameterType fireParam);

// This will schedule an object's method to be called on the next TaskManagerIO runloop using the given method slot and call parameter.
// Object is captured. Returns taskId or TASKMGR_INVALIDID on error.
template<class ObjectType, typename ParameterType> taskid_t scheduleObjectMethodCallOnce(SharedPtr<ObjectType> object, void (ObjectType::*method)(ParameterType), ParameterType callParam);

// This will schedule an object's method to be called on the next TaskManagerIO runloop using the given method slot and call parameter, w/o capturing object.
// Object is captured. Returns taskId or TASKMGR_INVALIDID on error.
template<class ObjectType, typename ParameterType> taskid_t scheduleObjectMethodCallOnce(ObjectType *object, void (ObjectType::*method)(ParameterType), ParameterType callParam);

// This will schedule an object's method to be called on the next TaskManagerIO runloop using the taskId that was created.
// Object is captured. Returns taskId or TASKMGR_INVALIDID on error.
template<class ObjectType> taskid_t scheduleObjectMethodCallWithTaskIdOnce(SharedPtr<ObjectType> object, void (ObjectType::*method)(taskid_t));

// This will schedule an object's method to be called on the next TaskManagerIO runloop using the taskId that was created, w/o capturing object.
// Returns taskId or TASKMGR_INVALIDID on error.
template<class ObjectType> taskid_t scheduleObjectMethodCallWithTaskIdOnce(ObjectType *object, void (ObjectType::*method)(taskid_t));


// Signal Fire Task
// This class holds onto the passed signal and parameter to pass it along to the signal's
// fire method upon task execution.
template<typename ParameterType, int Slots>
class SignalFireTask : public Executable {
public:
    taskid_t taskId;

    inline SignalFireTask(SharedPtr<AstroObjInterface> object, Signal<ParameterType,Slots> &signal, ParameterType &param)
        : taskId(TASKMGR_INVALIDID), _object(object), _signal(&signal), _param(param) { ; }

    virtual void exec() override;
private:
    SharedPtr<AstroObjInterface> _object;
    Signal<ParameterType, Slots> *_signal;
    ParameterType _param;
};


// Method Slot Task
// This class holds onto a MethodSlot to call once executed.
template<class ObjectType, typename ParameterType>
class MethodSlotCallTask : public Executable {
public:
    typedef void (ObjectType::*FunctPtr)(ParameterType);
    taskid_t taskId;

    inline MethodSlotCallTask(SharedPtr<ObjectType> object, FunctPtr method, ParameterType callParam)
        : taskId(TASKMGR_INVALIDID), _object(object), _methodSlot(object.get(), method), _callParam(callParam) { ; }
    inline MethodSlotCallTask(ObjectType *object, FunctPtr method, ParameterType callParam)
        : taskId(TASKMGR_INVALIDID), _object(nullptr), _methodSlot(object, method), _callParam(callParam) { ; }

    virtual void exec() override;
private:
    SharedPtr<ObjectType> _object;
    MethodSlot<ObjectType,ParameterType> _methodSlot;
    ParameterType _callParam;

    friend taskid_t scheduleObjectMethodCallWithTaskIdOnce<ObjectType>(SharedPtr<ObjectType> object, void (ObjectType::*method)(taskid_t));
    friend taskid_t scheduleObjectMethodCallWithTaskIdOnce<ObjectType>(ObjectType *object, void (ObjectType::*method)(taskid_t));
};

#endif // /ifdef ASTRO_USE_MULTITASKING

// Assertions

#ifdef ASTRO_USE_DEBUG_ASSERTIONS

// This softly asserts on a failed condition, sending message out to Serial output (if debugging enabled) and/or disk-based logging (if enabled), and then finally continuing program execution.
// See ASTRO_SOFT_ASSERT() macro for usage.
extern void softAssert(bool cond, String msg, const char *file, const char *func, int line);

// This hard asserts on a failed condition, sending message out to Serial output (if debugging enabled) and/or disk-based logging (if enabled), then yielding (to allow comms), and then finally aborting program execution.
// See ASTRO_HARD_ASSERT() macro for usage.
extern void hardAssert(bool cond, String msg, const char *file, const char *func, int line);

#endif // /ifdef ASTRO_USE_DEBUG_ASSERTIONS

// Helpers & Misc

// Returns the active controller instance. Not guaranteed to be non-null.
inline Astruino *getController();
// Returns the active scheduler instance. Not guaranteed to be non-null.
inline AstroScheduler *getScheduler();
// Returns the active logger instance. Not guaranteed to be non-null.
inline AstroLogger *getLogger();
// Returns the active publisher instance. Not guaranteed to be non-null.
inline AstroPublisher *getPublisher();
#ifdef ASTRO_USE_GUI
// Returns the active UI instance. Not guaranteed to be non-null.
inline AstroUIInterface *getUI();
#endif

// Publishes latest data from sensor to Publisher output.
extern void publishData(AstroSensor *sensor);

// Converts from local DateTime (offset by system TZ) back into unix/UTC time_t.
inline time_t unixTime(DateTime localTime);
// Converts from unix/UTC time_t into local DateTime (offset by system TZ).
inline DateTime localTime(time_t unixTime);
// Returns unix/UTC time_t of when today started.
inline time_t unixDayStart(time_t unixTime = unixNow());
// Returns local DateTime (offset by system TZ) of when today started.
inline DateTime localDayStart(time_t unixTime = unixNow());
// Sets global RTC current time to passed unix/UTC time_t.
inline void setUnixTime(time_t unixTime, bool isSigTime = false);
// Sets global RTC current time to passed local DateTime (offset by system TZ).
inline void setLocalTime(DateTime localTime, bool isSigTime = false);

// Returns a proper filename for a storage monitoring file (log, data, etc) that uses YYMMDD as filename.
extern String getYYMMDDFilename(String prefix, String ext);
// Returns a proper filename for a storage library data file that uses ## as filename.
extern String getNNFilename(String prefix, unsigned int value, String ext);
// Creates intermediate folders given a filename. Currently only supports a single folder depth.
extern void createDirectoryFor(SDClass *sd, String filename);

// Computes a hash for a string using a fast and efficient (read as: good enough for our use) hashing algorithm.
extern akey_t stringHash(String string);

// Returns properly formatted address "0xADDR" (size depending on void* size)
extern String addressToString(uintptr_t addr);

// Returns a string from char array with an exact max length.
// Null array or invalid length will abort function before encoding occurs, returning "null".
extern String charsToString(const char *charsIn, size_t length);
// Returns a string formatted to deal with variable time spans.
extern String timeSpanToString(const TimeSpan &span);
// Returns a string formatted to value and unit for dealing with measurements as value/units pair.
extern String measurementToString(float value, Astro_UnitsType units, unsigned int additionalDecPlaces = 0);
// Returns a string formatted to value and unit for dealing with measurements.
inline String measurementToString(const AstroSingleMeasurement &measurement, unsigned int additionalDecPlaces = 0) { return measurementToString(measurement.value, measurement.units, additionalDecPlaces); }

// Encodes a T-typed array to a comma-separated string.
// Null array or invalid length will abort function before encoding occurs, returning "null".
template<typename T> String commaStringFromArray(const T *arrayIn, size_t length);
// Decodes a comma-separated string back to a T-typed array.
// Last value read is repeated on through to last element, with no commas being treated as single value being applied to all elements.
// Empty string, string value of "null", null array, or invalid length will abort function before decoding.
template<typename T> void commaStringToArray(String stringIn, T *arrayOut, size_t length);
// Decodes a comma-separated JSON variant, if not null, object, or array, back to a T-typed array.
// Acts as a redirect for JSON variants so that they receive the additional checks before being converted to string.
template<typename T> void commaStringToArray(JsonVariantConst &variantIn, T *arrayOut, size_t length);

// Encodes a byte array to hexadecimal string.
extern String hexStringFromBytes(const uint8_t *bytesIn, size_t length);
// Decodes a hexadecimal string back to a byte array.
extern void hexStringToBytes(String stringIn, uint8_t *bytesOut, size_t length);
// Decodes a hexadecimal JSON variant, if not null, object, or array, back to a byte array.
extern void hexStringToBytes(JsonVariantConst &variantIn, uint8_t *bytesOut, size_t length);

// Returns # of occurrences of character in string.
extern int occurrencesInString(String string, char singleChar);
// Returns # of occurrences of substring in string.
extern int occurrencesInString(String string, String subString);
// Returns # of occurrences of character in string, ignoring case.
extern int occurrencesInStringIgnoreCase(String string, char singleChar);
// Returns # of occurrences of substring in string, ignoring case.
extern int occurrencesInStringIgnoreCase(String string, String subString);

// Returns whenever all elements of an array are equal to the specified value, or not.
template<typename T> bool arrayElementsEqual(const T *arrayIn, size_t length, T value);

// Similar to the standard map function, but does it on any type.
template<typename T> inline T mapValue(T value, T inMin, T inMax, T outMin, T outMax) { return ((value - inMin) * ((outMax - outMin) / (inMax - inMin))) + outMin; }

// Returns the amount of space between the stack and heap (ie free space left), else -1 if undeterminable.
extern unsigned int freeMemory();

// This delays a finely timed amount, with spin loop nearer to end. Used in finely timed dispensers.
extern void delayFine(millis_t time);

// This will query the active RTC sync device for the current time.
extern time_t rtcNow();

// This will return the current time as unix/UTC time_t (secs since 1970). Uses rtc time if available, otherwise noon 2000-Jan-1 + time since turned on.
// Default storage format. Do not use for data display/night-day-calcs - use localNow() or localTime() for local DateTime (offset by system TZ).
inline time_t unixNow() { return now(); }
// This will return the current time as local DateTime (offset by system TZ). Uses rtc time if available, otherwise noon 2000-Jan-1 + time since turned on.
// Meant to be used as a temporary or for runtime only. Do not use for data storage - use unixNow() or unixTime() for unix/UTC time_t (secs since 1970).
inline DateTime localNow() { return localTime(unixNow()); }

// This will return a non-zero millis time value, so that 0 time values can be reserved for other use.
inline millis_t nzMillis() { return millis() ?: 1; }

// This will handle interrupts for task manager.
extern void handleInterrupt(pintype_t pin);

// This is used to force debug statements through to serial monitor.
inline void flushYield() {
    #if defined(ASTRO_ENABLE_DEBUG_OUTPUT) && ASTRO_SYS_DEBUGOUT_FLUSH_YIELD
        Serial.flush(); yield();
    #else
        return;
    #endif
}

// Units & Conversion

// Tries to convert value from one unit to another (if supported), returning conversion success flag.
// Convert param used in certain unit conversions as external additional value (e.g. voltage for power/current conversion).
// This is the main conversion function that all others wrap around.
extern bool tryConvertUnits(float valueIn, Astro_UnitsType unitsIn, float *valueOut, Astro_UnitsType unitsOut, float convertParam = FLT_UNDEF);

// Attempts to convert value in-place from one unit to another, and if successful then assigns value back overtop of itself.
// Convert param used in certain unit conversions. Returns conversion success flag.
inline bool convertUnits(float *valueInOut, Astro_UnitsType *unitsInOut, Astro_UnitsType outUnits, float convertParam = FLT_UNDEF);
// Attempts to convert value from one unit to another, and if successful then assigns value, and optionally units, to output.
// Convert param used in certain unit conversions. Returns conversion success flag.
inline bool convertUnits(float valueIn, float *valueOut, Astro_UnitsType unitsIn, Astro_UnitsType outUnits, Astro_UnitsType *unitsOut = nullptr, float convertParam = FLT_UNDEF);
// Attempts to convert measurement in-place from one unit to another, and if successful then assigns value and units back overtop of itself.
// Convert param used in certain unit conversions. Returns conversion success flag.
inline bool convertUnits(AstroSingleMeasurement *measureInOut, Astro_UnitsType outUnits, float convertParam = FLT_UNDEF);
// Attemps to convert measurement from one unit to another, and if successful then assigns value and units to output measurement.
// Convert param used in certain unit conversions. Returns conversion success flag.
inline bool convertUnits(const AstroSingleMeasurement *measureIn, AstroSingleMeasurement *measureOut, Astro_UnitsType outUnits, float convertParam = FLT_UNDEF);

// For wrapping of values to positive-only moduli range [0, +range), e.g. [0,360) [0,2pi) etc, used in horizontal coordinates and as default wrap mode
template<typename T> inline T wrapBy(T value, T range) { value = value % range; return value >= 0 ? value : value + range; }
// For wrapping of values to positive-and-negative-split moduli range [-range/2,+range/2), e.g. [-180,180) [-pi,pi] etc, used in vertical coordinates
template<typename T> inline T wrapBySplit(T value, T range) { return wrapBy<T>(value + (range / 2), range) - (range / 2); }
// For wrapping of degree angle values to [0,360)
template<typename T> inline T wrapBy360(T value) { return wrapBy<T>(value, 360); }
// For wrapping of degree angle values to [-180,180)
template<typename T> inline T wrapBy180Neg180(T value) { return wrapBySplit<T>(value, 360); }
// For wrapping of radian angle values to [0,2pi)
template<typename T> inline T wrapBy2Pi(T value) { return wrapBy<T>(value, TWO_PI); }
// For wrapping of radian angle values to [-pi,pi)
template<typename T> inline T wrapByPiNegPi(T value) { return wrapBySplit<T>(value, TWO_PI); }
// For wrapping of minutes angle values to [0,24hr)
template<typename T> inline T wrapBy24Hr(T value) { return wrapBy<T>(value, MIN_PER_DAY); }
// For wrapping of minutes angle values to [-12hr,12hr)
template<typename T> inline T wrapBy12HrNeg12Hr(T value) { return wrapBySplit<T>(value, MIN_PER_DAY); }
// For wrapping of hours values to [0,24)
template<typename T> inline T wrapBy24(T value) { return wrapBy<T>(value, 24); }
// For wrapping of hours values to [-12,12)
template<typename T> inline T wrapBy12Neg12(T value) { return wrapBySplit<T>(value, 24); }

// Returns the base units from a rate unit (e.g. m/s -> m).
extern Astro_UnitsType baseUnits(Astro_UnitsType units);
// Returns the rate units from a base unit (e.g. m -> m/s).
extern Astro_UnitsType rateUnits(Astro_UnitsType units);

// Returns default units based on category and measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
extern Astro_UnitsType defaultUnits(Astro_UnitsCategory unitsCategory, Astro_MeasurementMode measureMode = Astro_MeasurementMode_Undefined);

// Returns default angle units based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline Astro_UnitsType defaultAngleUnits(Astro_MeasurementMode measureMode = Astro_MeasurementMode_Undefined) { return defaultUnits(Astro_UnitsCategory_Angle, measureMode); }
// Returns default distance units based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline Astro_UnitsType defaultDistanceUnits(Astro_MeasurementMode measureMode = Astro_MeasurementMode_Undefined) { return defaultUnits(Astro_UnitsCategory_Distance, measureMode); }
// Returns default percentile units based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline Astro_UnitsType defaultPercentileUnits(Astro_MeasurementMode measureMode = Astro_MeasurementMode_Undefined) { return defaultUnits(Astro_UnitsCategory_Percentile, measureMode); }
// Returns default power units based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline Astro_UnitsType defaultPowerUnits(Astro_MeasurementMode measureMode = Astro_MeasurementMode_Undefined) { return defaultUnits(Astro_UnitsCategory_Power, measureMode); }
// Returns default speed units based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline Astro_UnitsType defaultSpeedUnits(Astro_MeasurementMode measureMode = Astro_MeasurementMode_Undefined) { return rateUnits(defaultDistanceUnits(measureMode)); }
// Returns default temperature units based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline Astro_UnitsType defaultTemperatureUnits(Astro_MeasurementMode measureMode = Astro_MeasurementMode_Undefined) { return defaultUnits(Astro_UnitsCategory_Temperature, measureMode); }
// Returns default humidity units based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline Astro_UnitsType defaultHumidityUnits(Astro_MeasurementMode measureMode = Astro_MeasurementMode_Undefined) { return defaultUnits(Astro_UnitsCategory_Humidity, measureMode); }
// Returns default voltage units based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline Astro_UnitsType defaultVoltageUnits(Astro_MeasurementMode measureMode = Astro_MeasurementMode_Undefined) { return defaultUnits(Astro_UnitsCategory_Voltage, measureMode); }
// Returns default current units based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline Astro_UnitsType defaultCurrentUnits(Astro_MeasurementMode measureMode = Astro_MeasurementMode_Undefined) { return defaultUnits(Astro_UnitsCategory_Current, measureMode); }
// Returns default decimal places rounded to based on measurement mode (if undefined then uses active controller's measurement mode, else default measurement mode).
inline int defaultDecimalPlaces(Astro_MeasurementMode measureMode = Astro_MeasurementMode_Undefined) { return (int)defaultUnits(Astro_UnitsCategory_Count, measureMode); }

// Rounds value according to default decimal places rounding, as typically used for data export, with optional additional decimal places.
inline float roundForExport(float value, unsigned int additionalDecPlaces = 0) { return roundToDecimalPlaces(value, defaultDecimalPlaces() + additionalDecPlaces); }
// Rounds value according to default decimal places rounding, as typically used for data export, to string with optional additional decimal places.
inline String roundToString(float value, unsigned int additionalDecPlaces = 0) { return String(roundToDecimalPlaces(value, defaultDecimalPlaces() + additionalDecPlaces), defaultDecimalPlaces() + additionalDecPlaces); }

// Linkages & Filtering

// Returns linkages list filtered down to just actuators.
template<size_t N = ASTRO_DEFAULT_MAXSIZE> Vector<AstroObject *, N> linksFilterActuators(Pair<uint8_t, Pair<AstroObject *, int8_t> *> links);
// Returns linkages list filtered down to just actuators of a certain type that operate on a specific mount.
template<size_t N = ASTRO_DEFAULT_MAXSIZE> Vector<AstroObject *, N> linksFilterActuatorsByMountAndType(Pair<uint8_t, Pair<AstroObject *, int8_t> *> links, AstroMount *mount, Astro_ActuatorType actuatorType);

// Returns linkages list filtered down to just actuators of a certain motor'ed/or not type that operate on a specific mount.
template<size_t N = ASTRO_DEFAULT_MAXSIZE> Vector<AstroObject *, N>  linksFilterActuatorsByTypeMountAndMotor(Pair<uint8_t, Pair<AstroObject *, int8_t> *> links, Astro_ActuatorType actuatorType, AstroMount *mount, bool isMotor);
// Returns linkages list filtered down to just travel actuators of a motor'ed/or not type that operate on a specific mount and axis.
template<size_t N = ASTRO_DEFAULT_MAXSIZE> Vector<AstroObject *, N>  linksFilterTravelActuatorsByMountAxisAndMotor(Pair<uint8_t, Pair<AstroObject *, int8_t> *> links, AstroMount *mount, aposi_t axisIndex, bool isMotor);
// Returns linkages list filtered down to just actuators of a certain type that operate on a specific mount and axis.
template<size_t N = ASTRO_DEFAULT_MAXSIZE> Vector<AstroObject *, N> linksFilterActuatorsByMountAxisAndType(Pair<uint8_t, Pair<AstroObject *, int8_t> *> links, AstroMount *mount, aposi_t axisIndex, Astro_ActuatorType actuatorType);

// Returns the # of actuators that are travel based as found in the linkages list.
extern int linksCountTravelActuators(Pair<uint8_t, Pair<AstroObject *, int8_t> *> links);
// Returns the # of actuators of a certain type that operate on a specific mount.
extern int linksCountActuatorsByMountAndType(Pair<uint8_t, Pair<AstroObject *, int8_t> *> links, AstroMount *mount, Astro_ActuatorType actuatorType);

// Recombines filtered object list back into SharedPtr actuator list.
template<size_t N = ASTRO_DEFAULT_MAXSIZE> void linksResolveActuatorsByType(Vector<AstroObject *, N> &actuatorsIn, Vector<AstroActuatorAttachment, N> &activationsOut, Astro_ActuatorType actuatorType);

template<size_t N = ASTRO_DEFAULT_MAXSIZE> void linksResolveActuatorsToAttachments(Vector<AstroObject *, N> &actuatorsIn, AstroObjInterface *parent, aposi_t subIndex, Vector<AstroActuatorAttachment, N> &activationsOut);

// Recombines filtered object list back into SharedPtr actuator list paired with rate value.
template<size_t N = ASTRO_DEFAULT_MAXSIZE> void linksResolveActuatorsToAttachmentsByRateAndType(Vector<AstroObject *, N> &actuatorsIn, AstroObjInterface *parent, aposi_t subIndex, float rateMultiplier, Vector<AstroActuatorAttachment, N> &activationsOut, Astro_ActuatorType actuatorType);

// Pins & Checks

// Checks to see if the pin is an analog input pin.
extern bool checkPinIsAnalogInput(pintype_t pin);
// Checks to see if the pin is an analog output pin.
extern bool checkPinIsAnalogOutput(pintype_t pin);
// Checks to see if the pin is a standard digital (non-analog) pin.
inline bool checkPinIsDigital(pintype_t pin);
// Checks to see if the pin can produce a digital PWM output signal.
inline bool checkPinIsPWMOutput(pintype_t pin);
// Checks to see if the pin can be set up with an ISR to handle digital level changes.
inline bool checkPinCanInterrupt(pintype_t pin);

// Enums & Conversions

// Converts from system mode enum to string, with optional exclude for special types (instead returning "").
extern String systemModeToString(Astro_SystemMode systemMode, bool excludeSpecial = false);
// Converts back to system mode enum from string.
extern Astro_SystemMode systemModeFromString(String systemModeStr);

// Converts from measurement mode enum to string, with optional exclude for special types (instead returning "").
extern String measurementModeToString(Astro_MeasurementMode measurementMode, bool excludeSpecial = false);
// Converts back to measurement mode enum from string.
extern Astro_MeasurementMode measurementModeFromString(String measurementModeStr);

// Converts from display output mode enum to string, with optional exclude for special types (instead returning "").
extern String displayOutputModeToString(Astro_DisplayOutputMode displayOutMode, bool excludeSpecial = false);
// Converts back to display output mode enum from string.
extern Astro_DisplayOutputMode displayOutputModeFromString(String displayOutModeStr);

// Converts from control input mode enum to string, with optional exclude for special types (instead returning "").
extern String controlInputModeToString(Astro_ControlInputMode controlInMode, bool excludeSpecial = false);
// Converts back to control input mode enum from string.
extern Astro_ControlInputMode controlInputModeFromString(String controlInModeStr);

// Returns true for actuators whose equipment role is inherently motorized.
inline bool getActuatorIsMotorFromType(Astro_ActuatorType actuatorType) { return actuatorType == Astro_ActuatorType_MountAxis || actuatorType == Astro_ActuatorType_Cover || actuatorType == Astro_ActuatorType_Focuser; }
// Actuator equipment type does not encode the concrete servo/driver implementation.
inline bool getActuatorIsServoFromType(Astro_ActuatorType actuatorType) { (void)actuatorType; return false; }
// Returns true for actuators that operate activation handles serially (as opposed to in-parallel) as derived from enabled mode enumeration.
inline bool getActuatorIsSerialFromMode(Astro_EnableMode enableMode) { return enableMode >= Astro_EnableMode_Serial; }

// Converts from actuator type enum to string, with optional exclude for special types (instead returning "").
extern String actuatorTypeToString(Astro_ActuatorType actuatorType, bool excludeSpecial = false);
// Converts back to actuator type enum from string.
extern Astro_ActuatorType actuatorTypeFromString(String actuatorTypeStr);

// Converts from sensor type enum to string, with optional exclude for special types (instead returning "").
extern String sensorTypeToString(Astro_SensorType sensorType, bool excludeSpecial = false);
// Converts back to sensor type enum from string.
extern Astro_SensorType sensorTypeFromString(String sensorTypeStr);

// Converts from target type enum to string, with optional exclude for special types (instead returning "").
extern String targetTypeToString(Astro_TargetType targetType, bool excludeSpecial = false);
// Converts back to target type enum from string.
extern Astro_TargetType targetTypeFromString(String targetTypeStr);

// Converts from target class enum to string, with optional exclude for special types (instead returning "").
extern String targetClassToString(Astro_TargetClass targetClass, bool excludeSpecial = false);
// Converts back to target class enum from string.
extern Astro_TargetClass targetClassFromString(String targetClassStr);

// Returns axis count as derived from mount type enumeration.
extern aposi_t getMountAxisCountFromType(Astro_MountType mountType);
// Returns if mount coords are in equatorial (RA/dec) mode or not from mount type enumeration.
inline bool getIsEquatorialCoordsFromType(Astro_MountType mountType) { return mountType == Astro_MountType_Equatorial; }
// Returns if mount coords are in horizontal (azi/ele) mode or not from mount type enumeration.
inline bool getIsHorizontalCoordsFromType(Astro_MountType mountType) { return mountType == Astro_MountType_AltAzimuth; }
// Returns if mount type uses a driver for horizontal/primary axis control (azi, RA, or single tracking axis) or not from mount type enumeration.
inline bool getDrivesHorizontalAxis(Astro_MountType mountType) { return mountType == Astro_MountType_Equatorial || mountType == Astro_MountType_AltAzimuth || mountType == Astro_MountType_SingleAxis; }
// Returns if mount type uses a driver for vertical/secondary axis control (ele or dec) or not from mount type enumeration.
inline bool getDrivesVerticalAxis(Astro_MountType mountType) { return mountType == Astro_MountType_Equatorial || mountType == Astro_MountType_AltAzimuth; }

// Converts from mount enum to string, with optional exclude for special types (instead returning "").
extern String mountTypeToString(Astro_MountType mountType, bool excludeSpecial = false);
// Converts back to mount enum from string.
extern Astro_MountType mountTypeFromString(String mountTypeStr);

// Returns nominal rail voltage as derived from rail type enumeration.
extern float getRailVoltageFromType(Astro_RailType railType);

// Converts from power rail enum to string, with optional exclude for special types (instead returning "").
extern String railTypeToString(Astro_RailType railType, bool excludeSpecial = false);
// Converts back to power rail enum from string.
extern Astro_RailType railTypeFromString(String railTypeStr);

// Converts from pin mode enum to string, with optional exclude for special types (instead returning "").
extern String pinModeToString(Astro_PinMode pinMode, bool excludeSpecial = false);
// Converts back to pin mode enum from string.
extern Astro_PinMode pinModeFromString(String pinModeStr);

// Converts from actuator enable mode enum to string, with optional exclude for special types (instead returning "").
extern String enableModeToString(Astro_EnableMode enableMode, bool excludeSpecial = false);
// Converts back to actuator enable mode enum from string.
extern Astro_EnableMode enableModeFromString(String enableModeStr);

// Converts from units category enum to string, with optional exclude for special types (instead returning "").
extern String unitsCategoryToString(Astro_UnitsCategory unitsCategory, bool excludeSpecial = false);
// Converts back to units category enum from string.
extern Astro_UnitsCategory unitsCategoryFromString(String unitsCategoryStr);

// Converts from units type enum to symbol string, with optional exclude for special types (instead returning "").
extern String unitsTypeToSymbol(Astro_UnitsType unitsType, bool excludeSpecial = false);
// Converts back to units type enum from symbol.
extern Astro_UnitsType unitsTypeFromSymbol(String unitsSymbolStr);

// Converts from position index to string, with optional exclude for special types (instead returning "").
extern String positionIndexToString(aposi_t positionIndex, bool excludeSpecial = false);
// Converts back to position index from string.
extern aposi_t positionIndexFromString(String positionIndexStr);

// Converts from boolean value to triggered/not-triggered trigger state.
inline Astro_TriggerState triggerStateFromBool(bool value) { return value ? Astro_TriggerState_Triggered : Astro_TriggerState_NotTriggered; }
// Converts from triggered/not-triggered trigger state back into boolean value.
inline bool triggerStateToBool(Astro_TriggerState state) { return state == Astro_TriggerState_Triggered; }

// Explicit Specializations

template<> String commaStringFromArray<float>(const float *arrayIn, size_t length);
template<> String commaStringFromArray<double>(const double *arrayIn, size_t length);
template<> void commaStringToArray<float>(String stringIn, float *arrayOut, size_t length);
template<> void commaStringToArray<double>(String stringIn, double *arrayOut, size_t length);
template<> void commaStringToArray<String>(String stringIn, String *arrayOut, size_t length);
template<> bool arrayElementsEqual<float>(const float *arrayIn, size_t length, float value);
template<> bool arrayElementsEqual<double>(const double *arrayIn, size_t length, double value);
template<> inline float wrapBy(float value, float range);
template<> inline double wrapBy(double value, double range);

#endif // /ifndef AstroUtils_H
