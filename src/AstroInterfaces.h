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
class AstroCameraTrigger;
class AstroObservationDevice;
class AstroFocuser;
class AstroRTCInterface;

class AstroAttachment;
class AstroActuatorAttachment;
class AstroSensorAttachment;
class AstroAxisDriverAttachment;
class AstroTriggerAttachment;
class AstroObservationDeviceAttachment;

struct AstroIdentity;
struct AstroMeasurement;
struct AstroSingleMeasurement;

#include "Astruino.h"

// JSON Serializable Interface
class AstroJSONSerializableInterface {
public:
    virtual ~AstroJSONSerializableInterface() { ; }
    virtual void toJSONObject(JsonObject &objectOut) const = 0;
    virtual void fromJSONObject(JsonObjectConst &objectIn) = 0;
};

// Object Interface
// Common interface shared by system objects and sub-objects so that attachments and
// controller code can refer to them without depending on a concrete implementation.
class AstroObjInterface {
public:
    virtual ~AstroObjInterface() { ; }

    // Releases references to a system object before it is removed from object storage.
    virtual void unresolveAny(AstroObject *object) = 0;

    // Returns the object's identity, hashed key, and human-readable key string.
    virtual AstroIdentity getId() const = 0;
    virtual akey_t getKey() const = 0;
    virtual AstroString getKeyString() const = 0;
    virtual SharedPtr<AstroObjInterface> getSharedPtr() const = 0;
    virtual SharedPtr<AstroObjInterface> getSharedPtrFor(const AstroObjInterface *object) const = 0;

    // Returns true for main system objects and false for embedded sub-objects.
    virtual bool isObject() const = 0;
    inline bool isSubObject() const { return !isObject(); }
};

#ifdef ARDUINO
// RTC Module Interface
class AstroRTCInterface {
public:
    virtual bool begin(TwoWire *wireInstance) = 0;
    virtual void adjust(const DateTime &dt) = 0;
    virtual bool lostPower(void) = 0;
    virtual DateTime now() = 0;
};
#endif


// Digital Input Pin Interface
struct AstroDigitalInputPinInterface {
    virtual ~AstroDigitalInputPinInterface() { ; }
    virtual int digitalRead() = 0;
    inline int get() { return digitalRead(); }
};

// Digital Output Pin Interface
struct AstroDigitalOutputPinInterface {
    virtual ~AstroDigitalOutputPinInterface() { ; }
    virtual void digitalWrite(int status) = 0;
    inline void set(int status) { digitalWrite(status); }
};

// Analog Input Pin Interface
struct AstroAnalogInputPinInterface {
    virtual ~AstroAnalogInputPinInterface() { ; }
    virtual float analogRead() = 0;
    virtual int analogRead_raw() = 0;
    inline float get() { return analogRead(); }
    inline int get_raw() { return analogRead_raw(); }
};

// Analog Output Pin Interface
struct AstroAnalogOutputPinInterface {
    virtual ~AstroAnalogOutputPinInterface() { ; }
    virtual void analogWrite(float amount) = 0;
    virtual void analogWrite_raw(int amount) = 0;
    inline void set(float amount) { analogWrite(amount); }
    inline void set_raw(int amount) { analogWrite_raw(amount); }
};


// Angle Units Interface + Storage
class AstroAngleUnitsInterfaceStorage {
public:
    virtual ~AstroAngleUnitsInterfaceStorage() { ; }
    virtual void setAngleUnits(Astro_UnitsType angleUnits) { _angleUnits = angleUnits; }
    inline Astro_UnitsType getAngleUnits() const { return _angleUnits; }

protected:
    Astro_UnitsType _angleUnits;                            // Stored angle units
    inline AstroAngleUnitsInterfaceStorage(Astro_UnitsType angleUnits = Astro_UnitsType_Undefined)
        : _angleUnits(angleUnits) { ; }
};

// Distance Units Interface + Storage
class AstroDistanceUnitsInterfaceStorage {
public:
    virtual ~AstroDistanceUnitsInterfaceStorage() { ; }
    virtual void setDistanceUnits(Astro_UnitsType distanceUnits) { _distanceUnits = distanceUnits; }
    inline Astro_UnitsType getDistanceUnits() const { return _distanceUnits; }
    inline void setSpeedUnits(Astro_UnitsType speedUnits);
    inline Astro_UnitsType getSpeedUnits() const;

protected:
    Astro_UnitsType _distanceUnits;                         // Stored distance units
    inline AstroDistanceUnitsInterfaceStorage(Astro_UnitsType distanceUnits = Astro_UnitsType_Undefined)
        : _distanceUnits(distanceUnits) { ; }
};

// Measurement Units Interface
class AstroMeasurementUnitsInterface {
public:
    virtual ~AstroMeasurementUnitsInterface() { ; }
    virtual void setMeasurementUnits(Astro_UnitsType measurementUnits, uint8_t measurementRow = 0) = 0;
    virtual Astro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const = 0;

    inline Astro_UnitsType getRateUnits(uint8_t measurementRow = 0) const;
    inline Astro_UnitsType getBaseUnits(uint8_t measurementRow = 0) const;
};

template <size_t N>
class AstroMeasurementUnitsStorage {
protected:
    Astro_UnitsType _measurementUnits[N];                   // Stored measurement units by row

    inline AstroMeasurementUnitsStorage(Astro_UnitsType measurementUnits = Astro_UnitsType_Undefined)
    {
        for (size_t index = 0; index < N; ++index) { _measurementUnits[index] = measurementUnits; }
    }
};

class AstroMeasurementUnitsInterfaceStorageSingle : public AstroMeasurementUnitsInterface,
                                                    public AstroMeasurementUnitsStorage<1> {
public:
    virtual void setMeasurementUnits(Astro_UnitsType measurementUnits, uint8_t measurementRow = 0) override;
    virtual Astro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const override;

protected:
    inline AstroMeasurementUnitsInterfaceStorageSingle(Astro_UnitsType measurementUnits = Astro_UnitsType_Undefined)
        : AstroMeasurementUnitsStorage<1>(measurementUnits) { ; }
};

class AstroMeasurementUnitsInterfaceStorageDouble : public AstroMeasurementUnitsInterface,
                                                    public AstroMeasurementUnitsStorage<2> {
public:
    virtual void setMeasurementUnits(Astro_UnitsType measurementUnits, uint8_t measurementRow = 0) override;
    virtual Astro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const override;

protected:
    inline AstroMeasurementUnitsInterfaceStorageDouble(Astro_UnitsType measurementUnits = Astro_UnitsType_Undefined)
        : AstroMeasurementUnitsStorage<2>(measurementUnits) { ; }
};

class AstroMeasurementUnitsInterfaceStorageTriple : public AstroMeasurementUnitsInterface,
                                                    public AstroMeasurementUnitsStorage<3> {
public:
    virtual void setMeasurementUnits(Astro_UnitsType measurementUnits, uint8_t measurementRow = 0) override;
    virtual Astro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const override;

protected:
    inline AstroMeasurementUnitsInterfaceStorageTriple(Astro_UnitsType measurementUnits = Astro_UnitsType_Undefined)
        : AstroMeasurementUnitsStorage<3>(measurementUnits) { ; }
};

// Power Units Interface + Storage
class AstroPowerUnitsInterfaceStorage {
public:
    virtual ~AstroPowerUnitsInterfaceStorage() { ; }
    virtual void setPowerUnits(Astro_UnitsType powerUnits) { _powerUnits = powerUnits; }
    inline Astro_UnitsType getPowerUnits() const { return _powerUnits; }

protected:
    Astro_UnitsType _powerUnits;                            // Stored power units
    inline AstroPowerUnitsInterfaceStorage(Astro_UnitsType powerUnits = Astro_UnitsType_Undefined)
        : _powerUnits(powerUnits) { ; }
};

// Temperature Units Interface + Storage
class AstroTemperatureUnitsInterfaceStorage {
public:
    virtual ~AstroTemperatureUnitsInterfaceStorage() { ; }
    virtual void setTemperatureUnits(Astro_UnitsType temperatureUnits) { _temperatureUnits = temperatureUnits; }
    inline Astro_UnitsType getTemperatureUnits() const { return _temperatureUnits; }

protected:
    Astro_UnitsType _temperatureUnits;                      // Stored temperature units
    inline AstroTemperatureUnitsInterfaceStorage(Astro_UnitsType temperatureUnits = Astro_UnitsType_Undefined)
        : _temperatureUnits(temperatureUnits) { ; }
};


// Actuator Object Interface
class AstroActuatorObjectInterface {
public:
    virtual ~AstroActuatorObjectInterface() { ; }
    virtual void setPower(float power) = 0;
    virtual float getPower() const = 0;
};

// Sensor Object Interface
class AstroSensorObjectInterface {
public:
    virtual ~AstroSensorObjectInterface() { ; }
    virtual bool poll(int64_t timestamp = 0, aframe_t frame = 1) = 0;
    virtual const AstroSingleMeasurement &getMeasurement() const = 0;
};

// Mount Object Interface
class AstroMountObjectInterface {
public:
    virtual ~AstroMountObjectInterface() { ; }
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
    virtual ~AstroRailObjectInterface() { ; }
    virtual bool canActivate(AstroActuator *actuator) = 0;
    virtual float getCapacity(bool poll = false) = 0;
};

// Axis Driver Object Interface
class AstroDriverObjectInterface {
public:
    virtual ~AstroDriverObjectInterface() { ; }
    virtual void setTargetDegrees(double targetDegrees) = 0;
    virtual void stop() = 0;
    virtual double getTargetDegrees() const = 0;
};

// Trigger Object Interface
class AstroTriggerObjectInterface {
public:
    virtual ~AstroTriggerObjectInterface() { ; }
    virtual bool isTriggered() const = 0;
};

// Observation Device Interface
class AstroObservationDeviceInterface {
public:
    virtual ~AstroObservationDeviceInterface() { ; }
    virtual bool ready() const = 0;
    virtual void startObservation() = 0;
    virtual void stopObservation() = 0;
};

// Focuser Object Interface
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

    template<class U> inline void setParentMount(U mount);
    template<class U = AstroMount> inline SharedPtr<U> getParentMount();
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

// Observation Device Attachment Interface
class AstroObservationDeviceAttachmentInterface {
public:
    virtual AstroObservationDeviceAttachment &getObservationDeviceAttachment() = 0;

    template<class U> inline void setObservationDevice(SharedPtr<U> device);
    template<class U = AstroCameraTrigger> inline SharedPtr<U> getObservationDevice();
};

#endif // /ifndef AstroInterfaces_H
