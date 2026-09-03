/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Attachments
*/

#ifndef AstroAttachments_H
#define AstroAttachments_H

class AstroDLinkObject;
class AstroAttachment;
template<class ParameterType, int Slots> class AstroSignalAttachment;
class AstroActuatorAttachment;
class AstroSensorAttachment;
class AstroAxisDriverAttachment;
class AstroTriggerAttachment;

#include "Astruino.h"
#include "AstroObject.h"
#include "AstroMeasurements.h"
#include "AstroActivation.h"

// Delay/Dynamic Loaded/Linked Object Reference
// Simple class for delay loading objects that get references to others during object
// load. T should be a derived class of AstroObjInterface, with getId() method.
class AstroDLinkObject {
public:
    AstroDLinkObject();
    AstroDLinkObject(const AstroDLinkObject &obj);
    AstroDLinkObject &operator=(const AstroDLinkObject &obj);
    virtual ~AstroDLinkObject();

    inline bool isUnresolved() const { return !_obj; }
    inline bool isResolved() const { return (bool)_obj; }
    inline bool needsResolved() const { return isUnresolved() && isSet(); }
    inline bool resolve() { return isResolved() || (bool)getObject(); }
    void unresolve();
    template<class U> inline void unresolveIf(U obj) { if (operator==(obj)) { unresolve(); } }

    template<class U> inline void setObject(U obj) { operator=(obj); }
    template<class U = AstroObjInterface> inline SharedPtr<U> getObject() { return reinterpret_pointer_cast<U>(resolveObject()); }
    template<class U = AstroObjInterface> inline U *get() { return getObject<U>().get(); }

    inline AstroIdentity getId() const { return _obj ? _obj->getId() : (_keyStr ? AstroIdentity(_keyStr) : AstroIdentity(_key)); }
    inline akey_t getKey() const { return _key; }
    inline String getKeyString() const { return _keyStr ? String(_keyStr) : (_obj ? _obj->getKeyString() : addressToString((uintptr_t)_key)); }
    inline bool isSet() const { return _key != akey_none; }

    inline operator bool() const { return isResolved(); }
    inline AstroObjInterface *operator->() { return get(); }

    inline AstroDLinkObject &operator=(AstroIdentity rhs);
    inline AstroDLinkObject &operator=(const char *rhs);
    template<class U> inline AstroDLinkObject &operator=(SharedPtr<U> &rhs);
    inline AstroDLinkObject &operator=(const AstroObjInterface *rhs);
    inline AstroDLinkObject &operator=(const AstroAttachment *rhs);
    inline AstroDLinkObject &operator=(nullptr_t) { return operator=((AstroObjInterface *)nullptr); }

    inline bool operator==(const AstroIdentity &rhs) const { return _key == rhs.key; }
    inline bool operator==(const char *rhs) const { return _key == stringHash(rhs); }
    template<class U> inline bool operator==(const SharedPtr<U> &rhs) const { return _key == (rhs ? rhs->getKey() : akey_none); }
    inline bool operator==(const AstroObjInterface *rhs) const { return _key == (rhs ? rhs->getKey() : akey_none); }
    inline bool operator==(nullptr_t) const { return _key == akey_none; }

protected:
    akey_t _key;                                            // Object key
    SharedPtr<AstroObjInterface> _obj;                      // Shared pointer to object
    const char *_keyStr;                                    // Copy of id.keyString (if not resolved, or unresolved)

private:
    SharedPtr<AstroObjInterface> resolveObject();
    friend class Astruino;
    friend class AstroAttachment;
};

// Simple Attachment Point Base
// This attachment registers the parent object with the linked object's linkages upon
// dereference / unregisters the parent object at time of destruction or reassignment.
class AstroAttachment : public AstroSubObject {
public:
    AstroAttachment(AstroObjInterface *parent = nullptr, aposi_t subIndex = 0);
    AstroAttachment(const AstroAttachment &attachment);
    AstroAttachment &operator=(const AstroAttachment &attachment);
    virtual ~AstroAttachment();

    // Attaches object and any relevant signaling mechanisms. Derived classes should call base class's method first.
    virtual void attachObject();
    // Detaches object from any relevant signaling mechanism. Derived classes should call base class's method last.
    virtual void detachObject();

    // Attachment updater. Overridden by derived classes. May only update owned sub-objects (main objects are owned/updated by run system).
    virtual void updateIfNeeded(bool poll = false);

    inline bool isUnresolved() const { return !_obj; }
    inline bool isResolved() const { return (bool)_obj; }
    inline bool needsResolved() const { return _obj.needsResolved(); }
    inline bool resolve() { return isResolved() || (bool)getObject(); }
    inline void unresolve() { _obj.unresolve(); }
    template<class U> inline void unresolveIf(U obj) { _obj.unresolveIf(obj); }

    template<class U> void setObject(U obj, bool modify = true);
    template<class U> inline void initObject(U obj) { setObject(obj, false); }
    template<class U = AstroObjInterface> SharedPtr<U> getObject();
    template<class U = AstroObjInterface> inline U *get() { return getObject<U>().get(); }

    virtual void setParent(AstroObjInterface *parent) override;
    inline void setParent(AstroObjInterface *parent, aposi_t subIndex) { setParent(parent); setParentSubIndex(subIndex); }
    inline void setParentSubIndex(aposi_t subIndex) { _subIndex = subIndex; }
    inline AstroObjInterface *getParent() { return _parent; }
    inline aposi_t getParentSubIndex() { return _subIndex; }

    virtual AstroIdentity getId() const override { return _obj.getId(); }
    virtual akey_t getKey() const override { return _obj.getKey(); }
    virtual String getKeyString() const override { return _obj.getKeyString(); }
    inline bool isSet() const { return _obj.isSet(); }
    virtual SharedPtr<AstroObjInterface> getSharedPtrFor(const AstroObjInterface *obj) const override;

    inline operator bool() const { return isResolved(); }
    inline AstroObjInterface* operator->() { return get<AstroObjInterface>(); }

    inline AstroAttachment &operator=(const AstroIdentity &rhs) { setObject(rhs); return *this; }
    inline AstroAttachment &operator=(const char *rhs) { setObject(rhs); return *this; }
    template<class U> inline AstroAttachment &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline AstroAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }

    inline bool operator==(const AstroIdentity &rhs) const { return _obj == rhs; }
    inline bool operator==(const char *rhs) { return *this == AstroIdentity(rhs); }
    template<class U> inline bool operator==(const SharedPtr<U> &rhs) const { return _obj == rhs; }
    template<class U> inline bool operator==(const U *rhs) const { return _obj == rhs; }

protected:
    AstroDLinkObject _obj;                                  // Dynamic link object
    aposi_t _subIndex;                                      // Parent sub index, else 0
};


// Signal Attachment Point
// This attachment registers the parent object with a signal getter off the linked object
// upon resolvement / unregisters the parent object from the signal at time of destruction
// or reassignment.
template<class ParameterType, int Slots = 8>
class AstroSignalAttachment : public AstroAttachment {
public:
    typedef Signal<ParameterType,Slots> &(AstroObjInterface::*SignalGetterPtr)(void);

    template<class U> AstroSignalAttachment(AstroObjInterface *parent = nullptr, aposi_t subIndex = 0, Signal<ParameterType,Slots> &(U::*signalGetter)(void) = nullptr);
    AstroSignalAttachment(const AstroSignalAttachment<ParameterType,Slots> &attachment);
    AstroSignalAttachment<ParameterType,Slots> &operator=(const AstroSignalAttachment<ParameterType,Slots> &attachment);
    virtual ~AstroSignalAttachment();

    virtual void attachObject() override;
    virtual void detachObject() override;

    // Sets the signal handler getter method to use
    template<class U> void setSignalGetter(Signal<ParameterType,Slots> &(U::*signalGetter)(void));

    // Sets a handle slot to run when attached signal fires
    void setHandleSlot(const Slot<ParameterType> &handleSlot);
    inline void setHandleFunction(void (*handleFunctionPtr)(ParameterType)) { setHandleSlot(FunctionSlot<ParameterType>(handleFunctionPtr)); }
    template<class U, class V = U> inline void setHandleMethod(void (U::*handleMethodPtr)(ParameterType), V *handleClassInst = nullptr) { setHandleSlot(MethodSlot<V,ParameterType>(handleClassInst ? handleClassInst : static_cast<V *>(_parent), handleMethodPtr)); }

    inline AstroSignalAttachment<ParameterType,Slots> &operator=(const AstroIdentity &rhs) { setObject(rhs); return *this; }
    inline AstroSignalAttachment<ParameterType,Slots> &operator=(const char *rhs) { setObject(AstroIdentity(rhs)); return *this; }
    template<class U> inline AstroSignalAttachment<ParameterType,Slots> &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline AstroSignalAttachment<ParameterType,Slots> &operator=(const U *rhs) { setObject(rhs); return *this; }

protected:
    SignalGetterPtr _signalGetter;                          // Signal getter method ptr (weak)
    Slot<ParameterType> *_handleSlot;                       // Handler slot (owned)
};


// Actuator Attachment Point
// This attachment interfaces with actuator activation handles for actuator control, and
// registers the parent object with an actuator upon resolvement / unregisters the parent
// object from the actuator at time of destruction or reassignment.
class AstroActuatorAttachment : public AstroSignalAttachment<AstroActuator *, ASTRO_ACTUATOR_SIGNAL_SLOTS> {
public:
    AstroActuatorAttachment(AstroObjInterface *parent = nullptr, aposi_t subIndex = 0);
    AstroActuatorAttachment(const AstroActuatorAttachment &attachment);
    AstroActuatorAttachment &operator=(const AstroActuatorAttachment &attachment);
    virtual ~AstroActuatorAttachment();

    // Updates with actuator activation handle. Does not call actuator's update() (handled by system).
    virtual void updateIfNeeded(bool poll = false) override;

    // A rate multiplier is used to adjust either the intensity or duration of activations,
    // which depends on whenever they operate in binary mode (on/off) or variably (ranged).
    inline void setRateMultiplier(float rateMultiplier) { if (!isFPEqual(_rateMultiplier, rateMultiplier)) { _rateMultiplier = rateMultiplier; applySetup(); } }
    inline float getRateMultiplier() const { return _rateMultiplier; }

    // Activations are set up first by calling one of these methods. This configures the
    // direction, intensity, duration, and any run flags that the actuator will operate
    // upon once enabled, pending any rate offsetting. These methods are re-entrant.
    // The most recently used setup values are used for repeat activations.
    inline void setupActivation(const AstroActivation &activation) { _actSetup = activation; applySetup(); }
    inline void setupActivation(const AstroActivationHandle &handle) { setupActivation(handle.activation); }
    inline void setupActivation(Astro_DirectionMode direction, float intensity = 1.0f, millis_t duration = -1, bool force = false) { setupActivation(AstroActivation(direction, intensity, duration, (force ? Astro_ActivationFlags_Forced : Astro_ActivationFlags_None))); }
    inline void setupActivation(millis_t duration = -1, bool force = false) { setupActivation(AstroActivation(Astro_DirectionMode_Forward, 1.0f, duration, (force ? Astro_ActivationFlags_Forced : Astro_ActivationFlags_None))); }
    // These activation methods take a variable value that gets transformed by any user
    // curvature calibration data before being used, assuming units to be the same. It is
    // otherwise assumed the value is a normalized driving intensity ([0,1] or [-1,1]).
    void setupActivation(float value, millis_t duration = -1, bool force = false);
    inline void setupActivation(const AstroSingleMeasurement &measurement, millis_t duration = -1, bool force = false) { setupActivation(measurement.value, duration, force); }

    void enableActivation();
    inline void disableActivation() { _actHandle.unset(); }

    inline bool isActivated() const { return _actHandle.isActive(); }
    inline millis_t getTimeLeft() const { return _actHandle.getTimeLeft(); }
    inline millis_t getTimeActive(millis_t time = nzMillis()) const { return _actHandle.getTimeActive(time); }

    inline float getActiveDriveIntensity();
    inline float getSetupDriveIntensity() const;

    void setUpdateSlot(const Slot<AstroActuatorAttachment *> &updateSlot);
    inline void setUpdateFunction(void (*updateFunctionPtr)(AstroActuatorAttachment *)) { setUpdateSlot(FunctionSlot<AstroActuatorAttachment *>(updateFunctionPtr)); }
    template<class U> inline void setUpdateMethod(void (U::*updateMethodPtr)(AstroActivationHandle *), U *updateClassInst = nullptr) { setUpdateSlot(MethodSlot<U,AstroActuatorAttachment *>(updateClassInst ? updateClassInst : reinterpret_cast<U *>(_parent), updateMethodPtr)); }
    const Slot<AstroActuatorAttachment *> *getUpdateSlot() const { return _updateSlot; }

    inline const AstroActivationHandle &getActivationHandle() const { return _actHandle; }
    inline const AstroActivation &getActivationSetup() const { return _actSetup; }

    template<class U> inline void setObject(U obj, bool modify = false) { AstroAttachment::setObject(obj, modify); }
    inline SharedPtr<AstroActuator> getObject() { return AstroAttachment::getObject<AstroActuator>(); }
    inline AstroActuator *get() { return AstroAttachment::get<AstroActuator>(); }

    inline AstroActuator &operator*() { return *AstroAttachment::get<AstroActuator>(); }
    inline AstroActuator *operator->() { return AstroAttachment::get<AstroActuator>(); }

    inline AstroActuatorAttachment &operator=(const AstroIdentity &rhs) { setObject(rhs); return *this; }
    inline AstroActuatorAttachment &operator=(const char *rhs) { setObject(rhs); return *this; }
    template<class U> inline AstroActuatorAttachment &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline AstroActuatorAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }

protected:
    AstroActivationHandle _actHandle;                       // Actuator activation handle (double ref to object when active)
    AstroActivation _actSetup;                              // Actuator activation setup
    Slot<AstroActuatorAttachment *> *_updateSlot;           // Update slot (owned)
    float _rateMultiplier;                                  // Rate multiplier
    bool _calledLastUpdate;                                 // Last update call flag

    void applySetup();
};


// Sensor Measurement Attachment Point
// This attachment registers the parent object with a sensor's new measurement signal
// upon resolvement / unregisters the parent object from the sensor at time of destruction
// or reassignment.
// Custom handle method is responsible for calling setMeasurement() to update measurement.
class AstroSensorAttachment : public AstroSignalAttachment<const AstroMeasurement *, ASTRO_SENSOR_SIGNAL_SLOTS> {
public:
    AstroSensorAttachment(AstroObjInterface *parent = nullptr, aposi_t subIndex = 0, uint8_t measurementRow = 0);
    AstroSensorAttachment(const AstroSensorAttachment &attachment);
    virtual ~AstroSensorAttachment();

    virtual void attachObject() override;
    virtual void detachObject() override;

    // Updates measurement attachment with sensor. Does not call sensor's update() (handled by system).
    virtual void updateIfNeeded(bool poll = false) override;

    void setMeasurement(AstroSingleMeasurement measurement);
    inline void setMeasurement(float value, Astro_UnitsType units = Astro_UnitsType_Undefined) { setMeasurement(AstroSingleMeasurement(value, units)); }
    void setMeasurementRow(uint8_t measurementRow);
    void setMeasurementUnits(Astro_UnitsType units, float convertParam = FLT_UNDEF);

    inline const AstroSingleMeasurement &getMeasurement(bool poll = false) { updateIfNeeded(poll); return _measurement; }
    inline uint16_t getMeasurementFrame(bool poll = false) { updateIfNeeded(poll); return _measurement.frame; }
    inline float getMeasurementValue(bool poll = false) { updateIfNeeded(poll); return _measurement.value; }
    inline Astro_UnitsType getMeasurementUnits() const { return _measurement.units; }

    inline void setNeedsMeasurement() { _needsMeasurement = true; }
    inline bool needsMeasurement() { return _needsMeasurement; }

    inline uint8_t getMeasurementRow() const { return _measurementRow; }
    inline float getMeasurementConvertParam() const { return _convertParam; }

    inline SharedPtr<AstroSensor> getObject(bool poll = false) { updateIfNeeded(poll); return AstroAttachment::getObject<AstroSensor>(); }
    inline AstroSensor *get() { return AstroAttachment::get<AstroSensor>(); }

    inline AstroSensor &operator*() { return *AstroAttachment::get<AstroSensor>(); }
    inline AstroSensor *operator->() { return AstroAttachment::get<AstroSensor>(); }

    inline AstroSensorAttachment &operator=(const AstroIdentity &rhs) { setObject(rhs); return *this; }
    inline AstroSensorAttachment &operator=(const char *rhs) { setObject(rhs); return *this; }
    template<class U> inline AstroSensorAttachment &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline AstroSensorAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }

protected:
    AstroSingleMeasurement _measurement;                    // Local measurement (converted to measure units)
    uint8_t _measurementRow;                                // Measurement row
    float _convertParam;                                    // Convert param (default: FLT_UNDEF)
    bool _needsMeasurement;                                 // Stale measurement tracking flag

    void handleMeasurement(const AstroMeasurement *measurement);
};


// Mount Axis Driver Attachment Point
// Axis drivers are astronomy-specific sub-objects, not registered controller objects.
class AstroAxisDriverAttachment : public AstroSubObject {
public:
    AstroAxisDriverAttachment(AstroObjInterface *parent = nullptr, aposi_t axisIndex = 0);

    inline void setObject(SharedPtr<AstroAxisDriver> driver) { if (_driver != driver) { _driver = driver; bumpRevisionIfNeeded(); } }
    inline SharedPtr<AstroAxisDriver> getObject() const { return _driver; }
    inline AstroAxisDriver *get() const { return _driver.get(); }
    inline bool isSet() const { return (bool)_driver; }

    void setTargetDegrees(double targetDegrees);
    void stop();
    double getTargetDegrees() const;

    inline void setParentSubIndex(aposi_t axisIndex) { _axisIndex = axisIndex; }
    inline aposi_t getParentSubIndex() const { return _axisIndex; }

protected:
    SharedPtr<AstroAxisDriver> _driver;                      // Axis driver sub-object
    aposi_t _axisIndex;                                     // Parent axis index
};


// Trigger State Attachment Point
class AstroTriggerAttachment : public AstroSignalAttachment<Astro_TriggerState, ASTRO_TRIGGER_SIGNAL_SLOTS> {
public:
    AstroTriggerAttachment(AstroObjInterface *parent = nullptr, aposi_t subIndex = 0);
    AstroTriggerAttachment(const AstroTriggerAttachment &attachment);
    virtual ~AstroTriggerAttachment();

    virtual void updateIfNeeded(bool poll = false) override;

    inline Astro_TriggerState getTriggerState(bool poll = false);
    inline bool isTriggered(bool poll = false) { return getTriggerState(poll) == Astro_TriggerState_Triggered; }

    inline SharedPtr<AstroTrigger> getObject() { return AstroAttachment::getObject<AstroTrigger>(); }
    inline AstroTrigger *get() { return AstroAttachment::get<AstroTrigger>(); }

    inline AstroTrigger &operator*() { return *AstroAttachment::get<AstroTrigger>(); }
    inline AstroTrigger *operator->() { return AstroAttachment::get<AstroTrigger>(); }

    inline AstroTriggerAttachment &operator=(const AstroIdentity &rhs) { setObject(rhs); return *this; }
    inline AstroTriggerAttachment &operator=(const char *rhs) { setObject(rhs); return *this; }
    template<class U> inline AstroTriggerAttachment &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline AstroTriggerAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }
};

#endif // /ifndef AstroAttachments_H
