/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Interfaces
*/

#ifndef AstroInterfaces_H
#define AstroInterfaces_H

class AstroObject;
class AstroActuator;
class AstroSensor;
class AstroMount;
class AstroRail;
class AstroAxisDriver;
class AstroTrigger;
class AstroFocuser;

class AstroAttachment;
class AstroActuatorAttachment;
class AstroSensorAttachment;
class AstroAxisDriverAttachment;
class AstroTriggerAttachment;

struct AstroIdentity;
struct AstroMeasurement;
struct AstroSingleMeasurement;
struct AstroUIData;

struct AstroJSONSerializableInterface;

class AstroObjInterface;
class AstroUIInterface;
class AstroRTCInterface;

struct AstroDigitalInputPinInterface;
struct AstroDigitalOutputPinInterface;
struct AstroAnalogInputPinInterface;
struct AstroAnalogOutputPinInterface;

class AstroAngleUnitsInterfaceStorage;
class AstroDistanceUnitsInterfaceStorage;
class AstroMeasurementUnitsInterface;
template <size_t N = 1> class AstroMeasurementUnitsStorage;
class AstroMeasurementUnitsInterfaceStorageSingle;
class AstroMeasurementUnitsInterfaceStorageDouble;
class AstroMeasurementUnitsInterfaceStorageTriple;
class AstroPowerUnitsInterfaceStorage;
class AstroTemperatureUnitsInterfaceStorage;

class AstroActuatorObjectInterface;
class AstroSensorObjectInterface;
class AstroMountObjectInterface;
class AstroRailObjectInterface;
class AstroDriverObjectInterface;
class AstroTriggerObjectInterface;
class AstroFocuserObjectInterface;

class AstroParentActuatorAttachmentInterface;
class AstroParentSensorAttachmentInterface;
class AstroParentMountAttachmentInterface;
class AstroParentRailAttachmentInterface;

class AstroSensorAttachmentInterface;
class AstroTemperatureSensorAttachmentInterface;
class AstroHumiditySensorAttachmentInterface;
class AstroWindSpeedSensorAttachmentInterface;
class AstroRainSensorAttachmentInterface;
class AstroLightSensorAttachmentInterface;
class AstroPositionSensorAttachmentInterface;
class AstroPowerUsageSensorAttachmentInterface;

class AstroAxisDriverAttachmentInterface;
class AstroTriggerAttachmentInterface;
class AstroLimitTriggerAttachmentInterface;

#include "Astruino.h"

// JSON Serializable Interface
struct AstroJSONSerializableInterface {
    virtual void toJSONObject(JsonObject &objectOut) const = 0;
    virtual void fromJSONObject(JsonObjectConst &objectIn) = 0;
};

// Object Interface
// Common interface shared by system objects and sub-objects so that attachments and
// controller code can refer to them without depending on a concrete implementation.
class AstroObjInterface {
public:

    // Releases references to a system object before it is removed from object storage.
    virtual void unresolveAny(AstroObject *object) = 0;

    // Returns the object's identity, hashed key, and human-readable key string.
    virtual AstroIdentity getId() const = 0;
    virtual akey_t getKey() const = 0;
    virtual String getKeyString() const = 0;
    virtual SharedPtr<AstroObjInterface> getSharedPtr() const = 0;
    virtual SharedPtr<AstroObjInterface> getSharedPtrFor(const AstroObjInterface *object) const = 0;

    // Returns true for main system objects and false for embedded sub-objects.
    virtual bool isObject() const = 0;
    inline bool isSubObject() const { return !isObject(); }
};

// UI Interface
class AstroUIInterface {
public:
    virtual AstroUIData *init(AstroUIData *data = nullptr) = 0;
    virtual void begin() = 0;
    virtual void setNeedsRedraw() = 0;
};

// RTC Module Interface
class AstroRTCInterface {
public:
    virtual bool begin(TwoWire *wireInstance) = 0;
    virtual void adjust(const DateTime &dt) = 0;
    virtual bool lostPower(void) = 0;
    virtual DateTime now() = 0;
};


// Digital Input Pin Interface
struct AstroDigitalInputPinInterface {
    virtual ard_pinstatus_t digitalRead() = 0;
    inline int get() { return digitalRead(); }
};

// Digital Output Pin Interface
struct AstroDigitalOutputPinInterface {
    virtual void digitalWrite(ard_pinstatus_t status) = 0;
    inline void set(ard_pinstatus_t status) { digitalWrite(status); }
};

// Analog Input Pin Interface
struct AstroAnalogInputPinInterface {
    virtual float analogRead() = 0;
    virtual int analogRead_raw() = 0;
    inline float get() { return analogRead(); }
    inline int get_raw() { return analogRead_raw(); }
};

// Analog Output Pin Interface
struct AstroAnalogOutputPinInterface {
    virtual void analogWrite(float amount) = 0;
    virtual void analogWrite_raw(int amount) = 0;
    inline void set(float amount) { analogWrite(amount); }
    inline void set_raw(int amount) { analogWrite_raw(amount); }
};


// Angle Units Interface + Storage
class AstroAngleUnitsInterfaceStorage {
public:
    virtual void setAngleUnits(Astro_UnitsType angleUnits) = 0;
    inline Astro_UnitsType getAngleUnits() const { return _angleUnits; }

protected:
    Astro_UnitsType _angleUnits;                            // Stored angle units
    inline AstroAngleUnitsInterfaceStorage(Astro_UnitsType angleUnits = Astro_UnitsType_Undefined)
        : _angleUnits(angleUnits) { ; }
};

// Distance Units Interface + Storage
class AstroDistanceUnitsInterfaceStorage {
public:
    virtual void setDistanceUnits(Astro_UnitsType distanceUnits) = 0;
    inline Astro_UnitsType getDistanceUnits() const { return _distUnits; }
    inline void setSpeedUnits(Astro_UnitsType speedUnits);
    inline Astro_UnitsType getSpeedUnits() const;

protected:
    Astro_UnitsType _distUnits;                             // Stored distance units
    inline AstroDistanceUnitsInterfaceStorage(Astro_UnitsType distanceUnits = Astro_UnitsType_Undefined)
        : _distUnits(distanceUnits) { ; }
};

// Measurement Units Interface
// Uses virtual accessors so units may be stored locally or shadow another object.
class AstroMeasurementUnitsInterface {
public:
    virtual void setMeasurementUnits(Astro_UnitsType measurementUnits, uint8_t measurementRow = 0) = 0;
    virtual Astro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const = 0;

    inline Astro_UnitsType getRateUnits(uint8_t measurementRow = 0) const;
    inline Astro_UnitsType getBaseUnits(uint8_t measurementRow = 0) const;
};

// Measurement Units Storage
// Provides fixed-size backing storage shared by single, double, and triple measurement interfaces.
template <size_t N>
class AstroMeasurementUnitsStorage {
protected:
    Astro_UnitsType _measurementUnits[N];                   // Stored measurement units by row

    inline AstroMeasurementUnitsStorage(Astro_UnitsType measurementUnits = Astro_UnitsType_Undefined)
    {
        for (size_t index = 0; index < N; ++index) { _measurementUnits[index] = measurementUnits; }
    }
};

// Single Measurement Units Interface + Storage
class AstroMeasurementUnitsInterfaceStorageSingle : public AstroMeasurementUnitsInterface,
                                                    public AstroMeasurementUnitsStorage<1> {
protected:
    inline AstroMeasurementUnitsInterfaceStorageSingle(Astro_UnitsType measurementUnits = Astro_UnitsType_Undefined)
        : AstroMeasurementUnitsStorage<1>(measurementUnits) { ; }
};

// Double Measurement Units Interface + Storage
class AstroMeasurementUnitsInterfaceStorageDouble : public AstroMeasurementUnitsInterface,
                                                    public AstroMeasurementUnitsStorage<2> {
protected:
    inline AstroMeasurementUnitsInterfaceStorageDouble(Astro_UnitsType measurementUnits = Astro_UnitsType_Undefined)
        : AstroMeasurementUnitsStorage<2>(measurementUnits) { ; }
};

// Triple Measurement Units Interface + Storage
class AstroMeasurementUnitsInterfaceStorageTriple : public AstroMeasurementUnitsInterface,
                                                    public AstroMeasurementUnitsStorage<3> {
protected:
    inline AstroMeasurementUnitsInterfaceStorageTriple(Astro_UnitsType measurementUnits = Astro_UnitsType_Undefined)
        : AstroMeasurementUnitsStorage<3>(measurementUnits) { ; }
};

// Power Units Interface + Storage
class AstroPowerUnitsInterfaceStorage {
public:
    virtual void setPowerUnits(Astro_UnitsType powerUnits) = 0;
    inline Astro_UnitsType getPowerUnits() const { return _powerUnits; }

protected:
    Astro_UnitsType _powerUnits;                            // Stored power units
    inline AstroPowerUnitsInterfaceStorage(Astro_UnitsType powerUnits = Astro_UnitsType_Undefined)
        : _powerUnits(powerUnits) { ; }
};

// Temperature Units Interface + Storage
class AstroTemperatureUnitsInterfaceStorage {
public:
    virtual void setTemperatureUnits(Astro_UnitsType temperatureUnits) = 0;
    inline Astro_UnitsType getTemperatureUnits() const { return _tempUnits; }

protected:
    Astro_UnitsType _tempUnits;                             // Stored temperature units
    inline AstroTemperatureUnitsInterfaceStorage(Astro_UnitsType temperatureUnits = Astro_UnitsType_Undefined)
        : _tempUnits(temperatureUnits) { ; }
};


// Actuator Object Interface
class AstroActuatorObjectInterface {
public:
    virtual bool getCanEnable() = 0;
    virtual float getDriveIntensity() const = 0;
    virtual bool isEnabled(float tolerance = 0.0f) const = 0;

    virtual void setContinuousPowerUsage(AstroSingleMeasurement contPowerUsage) = 0;
    virtual const AstroSingleMeasurement &getContinuousPowerUsage() = 0;
    inline void setContinuousPowerUsage(float contPowerUsage, Astro_UnitsType contPowerUsageUnits = Astro_UnitsType_Undefined);

protected:
    virtual void _enableActuator(float intensity = 1.0f) = 0;
    virtual void _disableActuator() = 0;
};

// Sensor Object Interface
class AstroSensorObjectInterface {
public:
    virtual bool takeMeasurement(bool force = false) = 0;
    virtual const AstroMeasurement *getMeasurement(bool poll = false) = 0;
    virtual bool isTakingMeasurement() const = 0;
    virtual bool needsPolling(aframe_t allowance = 0) const = 0;
};

// Mount Object Interface
class AstroMountObjectInterface {
public:
    virtual void setTarget(Astro_TargetType targetType) = 0;
    virtual void park() = 0;
    virtual void unpark() = 0;
    virtual void stow() = 0;
    virtual void track() = 0;
    virtual bool isAligned(double toleranceDegrees = 0.25) const = 0;
    virtual bool isParked() const = 0;
};

// Power Rail Object Interface
class AstroRailObjectInterface {
public:
    virtual bool canActivate(AstroActuator *actuator) = 0;
    virtual float getCapacity(bool poll = false) = 0;
};

// Axis Driver Object Interface
class AstroDriverObjectInterface {
public:
    virtual void setTargetDegrees(double targetDegrees) = 0;
    virtual void stop() = 0;
    virtual double getTargetDegrees() const = 0;
};

// Trigger Object Interface
class AstroTriggerObjectInterface {
public:
    virtual Astro_TriggerState getTriggerState(bool poll = false) = 0;
    inline bool isTriggered(bool poll = false) { return getTriggerState(poll) == Astro_TriggerState_Triggered; }
};

// Focuser Object Interface
// Common absolute-position interface for telescope focus mechanisms.
class AstroFocuserObjectInterface {
public:
    virtual ~AstroFocuserObjectInterface() { ; }
    virtual void moveTo(int32_t position) = 0;
    virtual void moveBy(int32_t steps) = 0;
    virtual void halt() = 0;
    virtual int32_t getPosition() const = 0;
    virtual int32_t getTargetPosition() const = 0;
    virtual bool isMoving() const = 0;
};


// Parent Actuator Attachment Interface
class AstroParentActuatorAttachmentInterface {
public:
    virtual AstroAttachment &getParentActuatorAttachment() = 0;

    template<class U> inline void setParentActuator(U actuator);
    template<class U = AstroActuator> inline SharedPtr<U> getParentActuator();
};

// Parent Sensor Attachment Interface
class AstroParentSensorAttachmentInterface {
public:
    virtual AstroAttachment &getParentSensorAttachment() = 0;

    template<class U> inline void setParentSensor(U sensor);
    template<class U = AstroSensor> inline SharedPtr<U> getParentSensor();
};

// Parent Mount Attachment Interface
class AstroParentMountAttachmentInterface {
public:
    virtual AstroAttachment &getParentMountAttachment() = 0;

    template<class U> inline void setParentMount(U mount, aposi_t axisIndex = 0);
    template<class U = AstroMount> inline SharedPtr<U> getParentMount();
    inline aposi_t getParentMountAxisIndex();
};

// Parent Rail Attachment Interface
class AstroParentRailAttachmentInterface {
public:
    virtual AstroAttachment &getParentRailAttachment() = 0;

    template<class U> inline void setParentRail(U rail);
    template<class U = AstroRail> inline SharedPtr<U> getParentRail();
};

// Sensor Attachment Interface
class AstroSensorAttachmentInterface {
public:
    virtual AstroSensorAttachment &getSensorAttachment() = 0;

    template<class U> inline void setSensor(U sensor);
    template<class U = AstroSensor> inline SharedPtr<U> getSensor(bool poll = false);
};

// Temperature Sensor Attachment Interface
class AstroTemperatureSensorAttachmentInterface {
public:
    virtual AstroSensorAttachment &getTemperatureSensorAttachment() = 0;

    template<class U> inline void setTemperatureSensor(U sensor);
    template<class U = AstroSensor> inline SharedPtr<U> getTemperatureSensor(bool poll = false);
};

// Humidity Sensor Attachment Interface
class AstroHumiditySensorAttachmentInterface {
public:
    virtual AstroSensorAttachment &getHumiditySensorAttachment() = 0;

    template<class U> inline void setHumiditySensor(U sensor);
    template<class U = AstroSensor> inline SharedPtr<U> getHumiditySensor(bool poll = false);
};

// Wind Speed Sensor Attachment Interface
class AstroWindSpeedSensorAttachmentInterface {
public:
    virtual AstroSensorAttachment &getWindSpeedSensorAttachment() = 0;

    template<class U> inline void setWindSpeedSensor(U sensor);
    template<class U = AstroSensor> inline SharedPtr<U> getWindSpeedSensor(bool poll = false);
};

// Rain Sensor Attachment Interface
class AstroRainSensorAttachmentInterface {
public:
    virtual AstroSensorAttachment &getRainSensorAttachment() = 0;

    template<class U> inline void setRainSensor(U sensor);
    template<class U = AstroSensor> inline SharedPtr<U> getRainSensor(bool poll = false);
};

// Light Sensor Attachment Interface
class AstroLightSensorAttachmentInterface {
public:
    virtual AstroSensorAttachment &getLightSensorAttachment() = 0;

    template<class U> inline void setLightSensor(U sensor);
    template<class U = AstroSensor> inline SharedPtr<U> getLightSensor(bool poll = false);
};

// Position Sensor Attachment Interface
class AstroPositionSensorAttachmentInterface {
public:
    virtual AstroSensorAttachment &getPositionSensorAttachment() = 0;

    template<class U> inline void setPositionSensor(U sensor);
    template<class U = AstroSensor> inline SharedPtr<U> getPositionSensor(bool poll = false);
};

// Power Usage Sensor Attachment Interface
class AstroPowerUsageSensorAttachmentInterface {
public:
    virtual AstroSensorAttachment &getPowerUsageSensorAttachment() = 0;

    template<class U> inline void setPowerUsageSensor(U sensor);
    template<class U = AstroSensor> inline SharedPtr<U> getPowerUsageSensor(bool poll = false);
};

// Axis Driver Attachment Interface
class AstroAxisDriverAttachmentInterface {
public:
    virtual AstroAxisDriverAttachment &getAxisDriverAttachment() = 0;

    inline void setAxisDriver(SharedPtr<AstroAxisDriver> driver);
    inline SharedPtr<AstroAxisDriver> getAxisDriver();
};

// Trigger Attachment Interface
class AstroTriggerAttachmentInterface {
public:
    virtual AstroTriggerAttachment &getTriggerAttachment() = 0;

    inline void setTrigger(SharedPtr<AstroTrigger> trigger);
    inline SharedPtr<AstroTrigger> getTrigger(bool poll = false);
};

// Limit Trigger Attachment Interface
class AstroLimitTriggerAttachmentInterface {
public:
    virtual AstroTriggerAttachment &getLimitTriggerAttachment() = 0;

    inline void setLimitTrigger(SharedPtr<AstroTrigger> trigger);
    inline SharedPtr<AstroTrigger> getLimitTrigger(bool poll = false);
};

#endif // /ifndef AstroInterfaces_H
