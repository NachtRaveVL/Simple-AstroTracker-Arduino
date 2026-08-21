/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Attachments
*/

#ifndef AstroAttachments_H
#define AstroAttachments_H

class AstroDLinkObject;
class AstroAttachment;
class AstroActuatorAttachment;
class AstroSensorAttachment;
class AstroAxisDriverAttachment;
class AstroTriggerAttachment;
class AstroObservationDeviceAttachment;

class AstroObject;
class AstroActuator;
class AstroSensor;
class AstroAxisDriver;
class AstroTrigger;
class AstroObservationDevice;
class AstroCameraTrigger;

#include "AstroObject.h"
#include "AstroMeasurements.h"
#include "AstroActivation.h"

extern akey_t astroStringHash(const char *stringIn);

// Delay/Dynamic Loaded/Linked Object Reference
// Simple class for delay loading objects that get references to others during object
// load. T should be a derived class of AstroObjInterface, with getId() method.
class AstroDLinkObject {
public:
    AstroDLinkObject();
    AstroDLinkObject(const AstroDLinkObject &object);
    virtual ~AstroDLinkObject() { ; }

    inline bool isUnresolved() const { return !_obj; }
    inline bool isResolved() const { return (bool)_obj; }
    inline bool needsResolved() const { return isUnresolved() && isSet(); }
    inline bool resolve() { return isResolved() || (bool)getObject(); }
    void unresolve();
    template<class U> inline void unresolveIf(U object) { if (operator==(object)) { unresolve(); } }

    template<class U> inline void setObject(U object) { operator=(object); }
    template<class U = AstroObjInterface> inline SharedPtr<U> getObject() { return astroReinterpretPointerCast<U>(resolveObject()); }
    template<class U = AstroObjInterface> inline U *get() { return getObject<U>().get(); }

    AstroIdentity getId() const;
    inline akey_t getKey() const { return _key; }
    AstroString getKeyString() const;
    inline bool isSet() const { return _key != akey_none; }

    inline operator bool() const { return isResolved(); }
    inline AstroObjInterface *operator->() { return get(); }

    AstroDLinkObject &operator=(const AstroDLinkObject &rhs);
    inline AstroDLinkObject &operator=(AstroIdentity rhs);
    inline AstroDLinkObject &operator=(const char *rhs);
    template<class U> inline AstroDLinkObject &operator=(SharedPtr<U> rhs);
    inline AstroDLinkObject &operator=(const AstroObjInterface *rhs);
    inline AstroDLinkObject &operator=(const AstroAttachment *rhs);
    inline AstroDLinkObject &operator=(nullptr_t) { return operator=((AstroObjInterface *)nullptr); }

    inline bool operator==(const AstroIdentity &rhs) const { return _key == rhs.key; }
    inline bool operator==(const char *rhs) const { return _key == (rhs ? astroStringHash(rhs) : akey_none); }
    template<class U> inline bool operator==(const SharedPtr<U> &rhs) const { return _key == (rhs ? rhs->getKey() : akey_none); }
    inline bool operator==(const AstroObjInterface *rhs) const { return _key == (rhs ? rhs->getKey() : akey_none); }
    inline bool operator==(nullptr_t) const { return _key == akey_none; }

protected:
    akey_t _key;                                            // Object key
    SharedPtr<AstroObjInterface> _obj;                      // Shared pointer to object
    AstroString _keyString;                                 // Object key string while unresolved

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
    AstroAttachment(AstroObjInterface *parent = nullptr);
    AstroAttachment(const AstroAttachment &attachment);
    virtual ~AstroAttachment();

    // Attaches object and any relevant mechanisms. Derived classes should call base class's method first.
    virtual void attachObject();
    // Detaches object from any relevant mechanisms. Derived classes should call base class's method last.
    virtual void detachObject();

    // Attachment updater. Overridden by derived classes. May only update owned sub-objects.
    virtual void updateIfNeeded(bool poll = false);

    inline bool isUnresolved() const { return !_obj; }
    inline bool isResolved() const { return (bool)_obj; }
    inline bool needsResolved() const { return _obj.needsResolved(); }
    inline bool resolve() { return isResolved() || (bool)getObject(); }
    void unresolve();
    virtual void unresolveAny(AstroObject *object) override { unresolveIf(object); }
    template<class U> inline void unresolveIf(U object) { if (_obj == object) { unresolve(); } }

    template<class U> void setObject(U object, bool modify = true);
    template<class U> inline void initObject(U object) { setObject(object, false); }
    template<class U = AstroObjInterface> SharedPtr<U> getObject();
    template<class U = AstroObjInterface> inline U *get() { return getObject<U>().get(); }

    virtual void setParent(AstroObjInterface *parent) override;

    virtual AstroIdentity getId() const override { return _obj.getId(); }
    virtual akey_t getKey() const override { return _obj.getKey(); }
    virtual AstroString getKeyString() const override { return _obj.getKeyString(); }
    inline bool isSet() const { return _obj.isSet(); }
    virtual SharedPtr<AstroObjInterface> getSharedPtrFor(const AstroObjInterface *object) const override;

    inline operator bool() const { return isResolved(); }
    inline AstroObjInterface *operator->() { return get<AstroObjInterface>(); }

    inline AstroAttachment &operator=(const AstroIdentity &rhs) { setObject(rhs); return *this; }
    inline AstroAttachment &operator=(const char *rhs) { setObject(rhs); return *this; }
    template<class U> inline AstroAttachment &operator=(SharedPtr<U> rhs) { setObject(rhs); return *this; }
    template<class U> inline AstroAttachment &operator=(const U *rhs) { setObject(rhs); return *this; }

protected:
    AstroDLinkObject _obj;                                  // Dynamic link object
};

// Actuator Attachment Point
// This attachment interfaces with actuator activation handles for actuator control, and
// registers the parent object with an actuator upon resolvement / unregisters the parent
// object from the actuator at time of destruction or reassignment.
class AstroActuatorAttachment : public AstroAttachment {
public:
    AstroActuatorAttachment(AstroObjInterface *parent = nullptr);
    AstroActuatorAttachment(const AstroActuatorAttachment &attachment);
    virtual ~AstroActuatorAttachment() { ; }

    virtual void updateIfNeeded(bool poll = false) override;

    void setupActivation(const AstroActivation &activation);
    inline void setupActivation(Astro_DirectionMode direction, float intensity = 1.0f, millis_t duration = (millis_t)-1, bool force = false) { setupActivation(AstroActivation(direction, intensity, duration, force ? Astro_ActivationFlags_Forced : Astro_ActivationFlags_None)); }
    inline void setupActivation(millis_t duration = (millis_t)-1, bool force = false) { setupActivation(AstroActivation(Astro_DirectionMode_Forward, 1.0f, duration, force ? Astro_ActivationFlags_Forced : Astro_ActivationFlags_None)); }
    void setupActivation(float value, millis_t duration = (millis_t)-1, bool force = false);

    void enableActivation();
    inline void disableActivation() { _actHandle.unset(); }

    inline bool isActivated() const { return _actHandle.isActive(); }
    inline millis_t getTimeLeft() const { return _actHandle.getTimeLeft(); }
    inline millis_t getTimeActive(millis_t time = astroNZMillis()) const { return _actHandle.getTimeActive(time); }
    inline float getActiveDriveIntensity() const { return _actHandle.getDriveIntensity(); }
    inline float getSetupDriveIntensity() const { return _actSetup.getDriveIntensity(); }

    template<class U> inline void setObject(U object, bool modify = false) { AstroAttachment::setObject(object, modify); }
    inline SharedPtr<AstroActuator> getObject() { return AstroAttachment::getObject<AstroActuator>(); }
    inline AstroActuator *get() { return AstroAttachment::get<AstroActuator>(); }

    inline AstroActuator *operator->() { return get(); }

protected:
    AstroActivationHandle _actHandle;                       // Actuator activation handle
    AstroActivation _actSetup;                              // Actuator activation setup
};

// Sensor Measurement Attachment Point
// This attachment polls the selected measurement row from the attached sensor and stores
// the latest converted measurement locally for the parent process.
class AstroSensorAttachment : public AstroAttachment {
public:
    AstroSensorAttachment(AstroObjInterface *parent = nullptr, uint8_t measurementRow = 0);
    AstroSensorAttachment(const AstroSensorAttachment &attachment);
    virtual ~AstroSensorAttachment() { ; }

    virtual void attachObject() override;
    virtual void updateIfNeeded(bool poll = false) override;

    void setMeasurement(AstroSingleMeasurement measurement);
    void setMeasurementRow(uint8_t measurementRow);
    void setMeasurementUnits(Astro_UnitsType units, double convertParam = 0.0);

    inline const AstroSingleMeasurement &getMeasurement(bool poll = false) { updateIfNeeded(poll); return _measurement; }
    inline double getMeasurementValue(bool poll = false) { updateIfNeeded(poll); return _measurement.value; }
    inline Astro_UnitsType getMeasurementUnits() const { return _measurement.units; }
    inline uint8_t getMeasurementRow() const { return _measurementRow; }
    inline double getMeasurementConvertParam() const { return _convertParam; }
    inline void setNeedsMeasurement() { _needsMeasurement = true; }

    template<class U> inline void setObject(U object, bool modify = false) { AstroAttachment::setObject(object, modify); }
    inline SharedPtr<AstroSensor> getObject() { return AstroAttachment::getObject<AstroSensor>(); }
    inline AstroSensor *get() { return AstroAttachment::get<AstroSensor>(); }

    inline AstroSensor *operator->() { return get(); }

protected:
    AstroSingleMeasurement _measurement;                    // Current measurement
    uint8_t _measurementRow;                                // Measurement row
    double _convertParam;                                   // Measurement conversion parameter
    bool _needsMeasurement;                                 // Measurement update flag
};

// Axis Driver Attachment Point
// Axis drivers are owned sub-objects rather than registered main objects.
class AstroAxisDriverAttachment : public AstroSubObject {
public:
    AstroAxisDriverAttachment(AstroObjInterface *parent = nullptr, aposi_t axisIndex = 0);

    inline void setObject(SharedPtr<AstroAxisDriver> driver) { _driver = driver; bumpRevisionIfNeeded(); }
    inline SharedPtr<AstroAxisDriver> getObject() const { return _driver; }
    inline AstroAxisDriver *get() const { return _driver.get(); }
    inline bool isSet() const { return (bool)_driver; }

    void setTargetDegrees(double targetDegrees);
    void stop();
    double getTargetDegrees() const;

    inline void setParentSubIndex(aposi_t axisIndex) { _axisIndex = axisIndex; }
    inline aposi_t getParentSubIndex() const { return _axisIndex; }

protected:
    SharedPtr<AstroAxisDriver> _driver;                     // Axis driver sub-object
    aposi_t _axisIndex;                                     // Parent axis index
};

// Trigger Attachment Point
// Triggers are owned sub-objects that poll their own attached sensors.
class AstroTriggerAttachment : public AstroSubObject {
public:
    AstroTriggerAttachment(AstroObjInterface *parent = nullptr);

    void setObject(SharedPtr<AstroTrigger> trigger);
    inline SharedPtr<AstroTrigger> getObject() const { return _trigger; }
    inline AstroTrigger *get() const { return _trigger.get(); }
    inline bool isSet() const { return (bool)_trigger; }
    bool isTriggered(bool poll = false);

protected:
    SharedPtr<AstroTrigger> _trigger;                       // Trigger sub-object
    AstroObjInterface *parentForTrigger() const { return _parent; }
};

// Observation Device Attachment Point
// Observation devices are registered objects; the current concrete device is the camera trigger.
class AstroObservationDeviceAttachment : public AstroAttachment {
public:
    AstroObservationDeviceAttachment(AstroObjInterface *parent = nullptr) : AstroAttachment(parent) { ; }

    template<class U> inline void setObject(SharedPtr<U> object, bool modify = false) { AstroAttachment::setObject(object, modify); }
    SharedPtr<AstroCameraTrigger> getObject();
    AstroObservationDevice *get();
    inline AstroObservationDevice *operator->() { return get(); }
};

#include "AstroAttachments.hpp"

#endif // /ifndef AstroAttachments_H
