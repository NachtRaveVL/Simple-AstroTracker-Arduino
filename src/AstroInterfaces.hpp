/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Interface Inlines
*/

#ifndef AstroInterfaces_HPP
#define AstroInterfaces_HPP

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

class AstroMotorObjectInterface;

class AstroParentActuatorAttachmentInterface;
class AstroParentSensorAttachmentInterface;
class AstroParentMountAttachmentInterface;
class AstroParentRailAttachmentInterface;

class AstroSensorAttachmentInterface;
class AstroAngleSensorAttachmentInterface;
class AstroPositionSensorAttachmentInterface;
class AstroPowerProductionSensorAttachmentInterface;
class AstroPowerUsageSensorAttachmentInterface;
class AstroSpeedSensorAttachmentInterface;
class AstroTemperatureSensorAttachmentInterface;
class AstroWindSpeedSensorAttachmentInterface;

class AstroTriggerAttachmentInterface;
class AstroMinimumTriggerAttachmentInterface;
class AstroMaximumTriggerAttachmentInterface;
class AstroLimitTriggerAttachmentInterface;

#include "Astruino.h"

// JSON Serializable Interface
struct AstroJSONSerializableInterface {
    // Given a JSON element to fill in, writes self to JSON format.
    virtual void toJSONObject(JsonObject &objectOut) const = 0;

    // Given a JSON element to read from, reads overtop self from JSON format.
    virtual void fromJSONObject(JsonObjectConst &objectIn) = 0;
};


// Object Interface
class AstroObjInterface {
public:
    virtual void unresolveAny(AstroObject *obj) = 0;

    virtual AstroIdentity getId() const = 0;
    virtual akey_t getKey() const = 0;
    virtual String getKeyString() const = 0;
    virtual SharedPtr<AstroObjInterface> getSharedPtr() const = 0;
    virtual SharedPtr<AstroObjInterface> getSharedPtrFor(const AstroObjInterface *obj) const = 0;

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
    Astro_UnitsType _angleUnits;
    inline AstroAngleUnitsInterfaceStorage(Astro_UnitsType angleUnits = Astro_UnitsType_Undefined) : _angleUnits(angleUnits) { ; }
};

// Distance Units Interface + Storage
class AstroDistanceUnitsInterfaceStorage {
public:
    virtual void setDistanceUnits(Astro_UnitsType distanceUnits) = 0;
    inline Astro_UnitsType getDistanceUnits() const { return _distUnits; }
    inline void setSpeedUnits(Astro_UnitsType speedUnits);
    inline Astro_UnitsType getSpeedUnits() const;

protected:
    Astro_UnitsType _distUnits;
    inline AstroDistanceUnitsInterfaceStorage(Astro_UnitsType distanceUnits = Astro_UnitsType_Undefined) : _distUnits(distanceUnits) { ; }
};

// Measure Units Interface
// Uses virtual getMeasurementUnits() so that units can be local or shadowed
class AstroMeasurementUnitsInterface {
public:
    virtual void setMeasurementUnits(Astro_UnitsType measurementUnits, uint8_t measurementRow = 0) = 0;
    virtual Astro_UnitsType getMeasurementUnits(uint8_t measurementRow = 0) const = 0;

    inline Astro_UnitsType getRateUnits(uint8_t measurementRow = 0) const;
    inline Astro_UnitsType getBaseUnits(uint8_t measurementRow = 0) const;
};

// Measure Units Storage
template <size_t N> class AstroMeasurementUnitsStorage {
protected:
    Astro_UnitsType _measurementUnits[N];
    inline AstroMeasurementUnitsStorage(Astro_UnitsType measurementUnits = Astro_UnitsType_Undefined) { for (aposi_t i = 0; i < N; ++i) { _measurementUnits[i] = measurementUnits; } }
};

// Single Measure Units Interface + Storage
class AstroMeasurementUnitsInterfaceStorageSingle : public AstroMeasurementUnitsInterface, public AstroMeasurementUnitsStorage<1> {
protected:
    inline AstroMeasurementUnitsInterfaceStorageSingle(Astro_UnitsType measurementUnits = Astro_UnitsType_Undefined) : AstroMeasurementUnitsStorage<1>(measurementUnits) { ; }
};

// Double Measure Units Interface + Storage
class AstroMeasurementUnitsInterfaceStorageDouble : public AstroMeasurementUnitsInterface, public AstroMeasurementUnitsStorage<2> {
protected:
    inline AstroMeasurementUnitsInterfaceStorageDouble(Astro_UnitsType measurementUnits = Astro_UnitsType_Undefined) : AstroMeasurementUnitsStorage<2>(measurementUnits) { ; }
};

// Triple Measure Units Interface + Storage
class AstroMeasurementUnitsInterfaceStorageTriple : public AstroMeasurementUnitsInterface, public AstroMeasurementUnitsStorage<3> {
protected:
    inline AstroMeasurementUnitsInterfaceStorageTriple(Astro_UnitsType measurementUnits = Astro_UnitsType_Undefined) : AstroMeasurementUnitsStorage<3>(measurementUnits) { ; }
};

// Power Units Interface + Storage
class AstroPowerUnitsInterfaceStorage {
public:
    virtual void setPowerUnits(Astro_UnitsType powerUnits) = 0;
    inline Astro_UnitsType getPowerUnits() const { return _powerUnits; }

protected:
    Astro_UnitsType _powerUnits;
    inline AstroPowerUnitsInterfaceStorage(Astro_UnitsType powerUnits = Astro_UnitsType_Undefined) : _powerUnits(powerUnits) { ; }
};

// Temperature Units Interface + Storage
class AstroTemperatureUnitsInterfaceStorage {
public:
    virtual void setTemperatureUnits(Astro_UnitsType temperatureUnits) = 0;
    inline Astro_UnitsType getTemperatureUnits() const { return _tempUnits; }

protected:
    Astro_UnitsType _tempUnits;
    inline AstroTemperatureUnitsInterfaceStorage(Astro_UnitsType temperatureUnits = Astro_UnitsType_Undefined) : _tempUnits(temperatureUnits) { ; }
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
    virtual void _enableActuator(float intensity = 1.0) = 0;
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
    virtual bool canActivate(AstroActuator *actuator) = 0;
    virtual bool isDaylight(bool poll = false) = 0;
    virtual bool isAligned(bool poll = false) = 0;
};

// Rail Object Interface
class AstroRailObjectInterface {
public:
    virtual bool canActivate(AstroActuator *actuator) = 0;
    virtual float getCapacity(bool poll = false) = 0;
};

// Driver Object Interface
class AstroDriverObjectInterface {
public:
    virtual void setTargetSetpoint(float targetSetpoint) = 0;
    virtual float getMaxTargetOffset(bool poll = false) = 0;
    virtual Astro_DrivingState getDrivingState(bool poll = false) = 0;
    inline bool isAligned(bool poll = false) { return getDrivingState(poll) == Astro_DrivingState_AlignedTarget; }
};

// Trigger Object Interface
class AstroTriggerObjectInterface {
public:
    virtual Astro_TriggerState getTriggerState(bool poll = false) = 0;
    inline bool isTriggered(bool poll = false) { return getTriggerState(poll) == Astro_TriggerState_Triggered; }
};


// Motor Object Interface
class AstroMotorObjectInterface {
public:
    virtual bool canTravel(Astro_DirectionMode direction, float distance, Astro_UnitsType distanceUnits = Astro_UnitsType_Undefined) = 0;
    virtual AstroActivationHandle travel(Astro_DirectionMode direction, float distance, Astro_UnitsType distanceUnits = Astro_UnitsType_Undefined) = 0;
    virtual bool canTravel(Astro_DirectionMode direction, millis_t time) = 0;
    virtual AstroActivationHandle travel(Astro_DirectionMode direction, millis_t time) = 0;

    virtual void setContinuousSpeed(AstroSingleMeasurement contSpeed) = 0;
    virtual const AstroSingleMeasurement &getContinuousSpeed() = 0;
    inline void setContinuousSpeed(float contSpeed, Astro_UnitsType contSpeedUnits = Astro_UnitsType_Undefined);

    virtual bool isMinTravel(bool poll = false) = 0;
    virtual bool isMaxTravel(bool poll = false) = 0;

protected:
    virtual void handleTravelTime(millis_t time) = 0;
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


// Abstract Sensor Attachment Interface
class AstroSensorAttachmentInterface {
    virtual AstroSensorAttachment &getSensorAttachment() = 0;

    template<class U> inline void setSensor(U sensor);
    template<class U = AstroSensor> inline SharedPtr<U> getSensor(bool poll = false);
};

// Angle Sensor Attachment Interface
class AstroAngleSensorAttachmentInterface {
public:
    virtual AstroSensorAttachment &getAngleSensorAttachment() = 0;

    template<class U> inline void setAngleSensor(U sensor);
    template<class U = AstroSensor> inline SharedPtr<U> getAngleSensor(bool poll = false);
};

// Position Sensor Attachment Interface
class AstroPositionSensorAttachmentInterface {
public:
    virtual AstroSensorAttachment &getPositionSensorAttachment() = 0;

    template<class U> inline void setPositionSensor(U sensor);
    template<class U = AstroSensor> inline SharedPtr<U> getPositionSensor(bool poll = false);
};

// Power Production Sensor Attachment Interface
class AstroPowerProductionSensorAttachmentInterface {
public:
    virtual AstroSensorAttachment &getPowerProductionSensorAttachment() = 0;

    template<class U> inline void setPowerProductionSensor(U sensor);
    template<class U = AstroSensor> inline SharedPtr<U> getPowerProductionSensor(bool poll = false);
};

// Power Usage Sensor Attachment Interface
class AstroPowerUsageSensorAttachmentInterface {
public:
    virtual AstroSensorAttachment &getPowerUsageSensorAttachment() = 0;

    template<class U> inline void setPowerUsageSensor(U sensor);
    template<class U = AstroSensor> inline SharedPtr<U> getPowerUsageSensor(bool poll = false);
};

// Speed Sensor Attachment Interface
class AstroSpeedSensorAttachmentInterface {
public:
    virtual AstroSensorAttachment &getSpeedSensorAttachment() = 0;

    template<class U> inline void setSpeedSensor(U sensor);
    template<class U = AstroSensor> inline SharedPtr<U> getSpeedSensor(bool poll = false);
};

// Temperature Sensor Attachment Interface
class AstroTemperatureSensorAttachmentInterface {
public:
    virtual AstroSensorAttachment &getTemperatureSensorAttachment() = 0;

    template<class U> inline void setTemperatureSensor(U sensor);
    template<class U = AstroSensor> inline SharedPtr<U> getTemperatureSensor(bool poll = false);
};

// Wind Speed Sensor Attachment Interface
class AstroWindSpeedSensorAttachmentInterface {
public:
    virtual AstroSensorAttachment &getWindSpeedSensorAttachment() = 0;

    template<class U> inline void setWindSpeedSensor(U sensor);
    template<class U = AstroSensor> inline SharedPtr<U> getWindSpeedSensor(bool poll = false);
};


// Abstract Trigger Attachment Interface
class AstroTriggerAttachmentInterface {
    virtual AstroTriggerAttachment &getTriggerAttachment() = 0;

    template<class U> inline void setTrigger(U trigger);
    template<class U = AstroTrigger> inline SharedPtr<U> getTrigger(bool poll = false);
};

// Minimum Trigger Attachment Interface
class AstroMinimumTriggerAttachmentInterface {
public:
    virtual AstroTriggerAttachment &getMinimumTriggerAttachment() = 0;

    template<class U> inline void setMinimumTrigger(U trigger);
    template<class U = AstroTrigger> inline SharedPtr<U> getMinimumTrigger(bool poll = false);
};

// Maximum Trigger Attachment Interface
class AstroMaximumTriggerAttachmentInterface {
public:
    virtual AstroTriggerAttachment &getMaximumTriggerAttachment() = 0;

    template<class U> inline void setMaximumTrigger(U trigger);
    template<class U = AstroTrigger> inline SharedPtr<U> getMaximumTrigger(bool poll = false);
};

// Limit Trigger Attachment Interface
class AstroLimitTriggerAttachmentInterface {
public:
    virtual AstroTriggerAttachment &getLimitTriggerAttachment() = 0;

    template<class U> inline void setLimitTrigger(U trigger);
    template<class U = AstroTrigger> inline SharedPtr<U> getLimitTrigger(bool poll = false);
};

#endif // /ifndef AstroInterfaces_HPP
