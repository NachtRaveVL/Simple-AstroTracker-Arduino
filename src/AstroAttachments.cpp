/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Attachments
*/

#include "Astruino.h"

AstroDLinkObject::AstroDLinkObject()
    : _key(akey_none), _obj(nullptr), _keyString()
{ ; }

AstroDLinkObject::AstroDLinkObject(const AstroDLinkObject &object)
    : _key(object._key), _obj(object._obj), _keyString(object._keyString)
{ ; }

AstroDLinkObject &AstroDLinkObject::operator=(const AstroDLinkObject &rhs)
{
    if (this != &rhs) {
        _key = rhs._key;
        _obj = rhs._obj;
        _keyString = rhs._keyString;
    }
    return *this;
}

void AstroDLinkObject::unresolve()
{
    if (_obj && !_keyString.length()) { _keyString = _obj->getKeyString(); }
    _obj = nullptr;
}

SharedPtr<AstroObjInterface> AstroDLinkObject::resolveObject()
{
    if (_obj || !isSet()) { return _obj; }
    if (Astruino::_activeInstance) {
        _obj = static_pointer_cast<AstroObjInterface>(Astruino::_activeInstance->_objects[_key]);
    }
    if (_obj) { _keyString = AstroString(); }
    return _obj;
}

AstroIdentity AstroDLinkObject::getId() const
{
    return _obj ? _obj->getId() : (_keyString.length() ? AstroIdentity(_keyString.c_str()) : AstroIdentity(_key));
}

AstroString AstroDLinkObject::getKeyString() const
{
    return _keyString.length() ? _keyString : (_obj ? _obj->getKeyString() : AstroString());
}

AstroAttachment::AstroAttachment(AstroObjInterface *parent)
    : AstroSubObject(parent), _obj()
{ ; }

AstroAttachment::AstroAttachment(const AstroAttachment &attachment)
    : AstroSubObject(attachment._parent), _obj()
{
    initObject(attachment._obj);
}

AstroAttachment::~AstroAttachment()
{
    if (isResolved() && _obj->isObject() && _parent && _parent->isObject()) {
        _obj.get<AstroObject>()->removeLinkage(static_cast<AstroObject *>(_parent));
    }
}

void AstroAttachment::attachObject()
{
    if (resolve() && _obj->isObject() && _parent && _parent->isObject()) {
        _obj.get<AstroObject>()->addLinkage(static_cast<AstroObject *>(_parent));
    }
}

void AstroAttachment::detachObject()
{
    if (isResolved() && _obj->isObject() && _parent && _parent->isObject()) {
        _obj.get<AstroObject>()->removeLinkage(static_cast<AstroObject *>(_parent));
    }
}

void AstroAttachment::updateIfNeeded(bool poll)
{
    (void)poll;
}

void AstroAttachment::unresolve()
{
    detachObject();
    _obj.unresolve();
}

void AstroAttachment::setParent(AstroObjInterface *parent)
{
    if (_parent != parent) {
        detachObject();
        _parent = parent;
        if (isResolved()) { attachObject(); }
    }
}

SharedPtr<AstroObjInterface> AstroAttachment::getSharedPtrFor(const AstroObjInterface *object) const
{
    return object->getKey() == getKey() ? _obj._obj : AstroSubObject::getSharedPtrFor(object);
}

AstroActuatorAttachment::AstroActuatorAttachment(AstroObjInterface *parent)
    : AstroAttachment(parent), _actHandle(), _actSetup()
{ ; }

AstroActuatorAttachment::AstroActuatorAttachment(const AstroActuatorAttachment &attachment)
    : AstroAttachment(attachment), _actHandle(), _actSetup(attachment._actSetup)
{ ; }

void AstroActuatorAttachment::updateIfNeeded(bool poll)
{
    (void)poll;
    if (_actHandle.isActive()) { _actHandle.elapseTo(); }
}

void AstroActuatorAttachment::setupActivation(const AstroActivation &activation)
{
    _actSetup = activation;
    _actHandle.activation = activation;
    if (isActivated() && resolve()) { get()->setNeedsUpdate(); }
}

void AstroActuatorAttachment::setupActivation(float value, millis_t duration, bool force)
{
    setupActivation(AstroActivation(value > ASTRO_FLT_EPSILON ? Astro_DirectionMode_Forward :
                                    value < -ASTRO_FLT_EPSILON ? Astro_DirectionMode_Reverse : Astro_DirectionMode_Stop,
                                    fabsf(value), duration,
                                    force ? Astro_ActivationFlags_Forced : Astro_ActivationFlags_None));
}

void AstroActuatorAttachment::enableActivation()
{
    if (!_actHandle.actuator && _actSetup.isValid() && resolve()) {
        _actHandle.activation = _actSetup;
        _actHandle = getObject();
    }
}

AstroSensorAttachment::AstroSensorAttachment(AstroObjInterface *parent, uint8_t measurementRow)
    : AstroAttachment(parent), _measurement(), _measurementRow(measurementRow),
      _convertParam(0.0), _needsMeasurement(true)
{ ; }

AstroSensorAttachment::AstroSensorAttachment(const AstroSensorAttachment &attachment)
    : AstroAttachment(attachment), _measurement(attachment._measurement),
      _measurementRow(attachment._measurementRow), _convertParam(attachment._convertParam),
      _needsMeasurement(attachment._needsMeasurement)
{ ; }

void AstroSensorAttachment::attachObject()
{
    AstroAttachment::attachObject();
    setNeedsMeasurement();
}

void AstroSensorAttachment::updateIfNeeded(bool poll)
{
    if (!resolve()) { return; }
    if (poll || _needsMeasurement || !get()->getMeasurement().isSet()) {
        get()->poll((int64_t)unixNow(), 1);
    }
    setMeasurement(getAsSingleMeasurement(&get()->getMeasurement(), _measurementRow));
}

void AstroSensorAttachment::setMeasurement(AstroSingleMeasurement measurement)
{
    if (_measurement.units != Astro_UnitsType_Undefined && measurement.units != _measurement.units) {
        measurement.toUnits(_measurement.units, _convertParam);
    }
    _measurement = measurement;
    _needsMeasurement = false;
}

void AstroSensorAttachment::setMeasurementRow(uint8_t measurementRow)
{
    if (_measurementRow != measurementRow) {
        _measurementRow = measurementRow;
        setNeedsMeasurement();
        bumpRevisionIfNeeded();
    }
}

void AstroSensorAttachment::setMeasurementUnits(Astro_UnitsType units, double convertParam)
{
    if (_measurement.units != units || !isFPEqual(_convertParam, convertParam)) {
        _measurement.units = units;
        _convertParam = convertParam;
        setNeedsMeasurement();
        bumpRevisionIfNeeded();
    }
}

AstroAxisDriverAttachment::AstroAxisDriverAttachment(AstroObjInterface *parent, aposi_t axisIndex)
    : AstroSubObject(parent), _driver(), _axisIndex(axisIndex)
{ ; }

void AstroAxisDriverAttachment::setTargetDegrees(double targetDegrees)
{
    if (_driver) { _driver->setTargetDegrees(targetDegrees); }
}

void AstroAxisDriverAttachment::stop()
{
    if (_driver) { _driver->stop(); }
}

double AstroAxisDriverAttachment::getTargetDegrees() const
{
    return _driver ? _driver->getTargetDegrees() : 0.0;
}

AstroTriggerAttachment::AstroTriggerAttachment(AstroObjInterface *parent)
    : AstroSubObject(parent), _trigger()
{ ; }

void AstroTriggerAttachment::setObject(SharedPtr<AstroTrigger> trigger)
{
    _trigger = trigger;
    if (_trigger) { _trigger->setParent(parentForTrigger()); }
    bumpRevisionIfNeeded();
}

bool AstroTriggerAttachment::isTriggered(bool poll)
{
    if (_trigger && poll) { _trigger->update(); }
    return _trigger && _trigger->isTriggered();
}

SharedPtr<AstroCameraTrigger> AstroObservationDeviceAttachment::getObject()
{
    return AstroAttachment::getObject<AstroCameraTrigger>();
}

AstroObservationDevice *AstroObservationDeviceAttachment::get()
{
    auto object = getObject();
    return object ? static_cast<AstroObservationDevice *>(object.get()) : nullptr;
}
