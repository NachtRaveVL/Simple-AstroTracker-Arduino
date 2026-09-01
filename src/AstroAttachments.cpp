/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Attachments
*/

#include "Astruino.h"
#include "AstroCoreLogic.h"

AstroDLinkObject::AstroDLinkObject()
    : _key(akey_none), _obj(nullptr), _keyStr(nullptr)
{ ; }

AstroDLinkObject::AstroDLinkObject(const AstroDLinkObject &obj)
    : _key(obj._key), _obj(obj._obj), _keyStr(nullptr)
{
    if (obj._keyStr) {
        auto len = strnlen(obj._keyStr, ASTRO_NAME_MAXSIZE);
        if (len) {
            _keyStr = (const char *)malloc(len + 1);
            strncpy((char *)_keyStr, obj._keyStr, len + 1);
        }
    }
}

AstroDLinkObject::~AstroDLinkObject()
{
    if (_keyStr) { free((void *)_keyStr); }
}

void AstroDLinkObject::unresolve()
{
    if (_obj && !_keyStr) {
        auto id = _obj->getId();
        auto len = id.keyString.length();
        if (len) {
            _keyStr = (const char *)malloc(len + 1);
            strncpy((char *)_keyStr, id.keyString.c_str(), len + 1);
        }
    }
    ASTRO_HARD_ASSERT(!_obj || _key == _obj->getKey(), SFP(AStr_Err_OperationFailure));
    _obj = nullptr;
}

SharedPtr<AstroObjInterface> AstroDLinkObject::resolveObject()
{
    if (_obj || !isSet()) { return _obj; }
    if (Astruino::_activeInstance) {
        _obj = static_pointer_cast<AstroObjInterface>(Astruino::_activeInstance->_objects[_key]);
    }
    if (_obj && _keyStr) {
        free((void *)_keyStr); _keyStr = nullptr;
    }
    return _obj;
}


AstroAttachment::AstroAttachment(AstroObjInterface *parent, aposi_t subIndex)
    : AstroSubObject(parent), _obj(), _subIndex(subIndex)
{ ; }

AstroAttachment::AstroAttachment(const AstroAttachment &attachment)
    : AstroSubObject(attachment._parent), _obj(), _subIndex(attachment._subIndex)
{
    initObject(attachment._obj);
}

AstroAttachment::~AstroAttachment()
{
    if (isResolved() && _obj->isObject() && _parent && _parent->isObject()) {
        _obj.get<AstroObject>()->removeLinkage((AstroObject *)_parent);
    }
}

void AstroAttachment::attachObject()
{
    if (resolve() && _obj->isObject() && _parent && _parent->isObject()) { // purposeful resolve in front
        _obj.get<AstroObject>()->addLinkage((AstroObject *)_parent);
    }
}

void AstroAttachment::detachObject()
{
    if (isResolved() && _obj->isObject() && _parent && _parent->isObject()) {
        _obj.get<AstroObject>()->removeLinkage((AstroObject *)_parent);
    }
    // note: used to set _obj to nullptr here, but found that it's best not to -> avoids additional operator= calls during typical detach scenarios
}

void AstroAttachment::updateIfNeeded(bool poll)
{
    // intended to be overridden by derived classes, but not an error if left not implemented
}

void AstroAttachment::setParent(AstroObjInterface *parent)
{
    if (_parent != parent) {
        if (isResolved() && _obj->isObject() && _parent && _parent->isObject()) { _obj.get<AstroObject>()->removeLinkage((AstroObject *)_parent); }

        _parent = parent;

        if (isResolved() && _obj->isObject() && _parent && _parent->isObject()) { _obj.get<AstroObject>()->addLinkage((AstroObject *)_parent); }
    }
}

SharedPtr<AstroObjInterface> AstroAttachment::getSharedPtrFor(const AstroObjInterface *obj) const
{
    return obj->getKey() == getKey() ? _obj._obj : AstroSubObject::getSharedPtrFor(obj);
}


AstroActuatorAttachment::AstroActuatorAttachment(AstroObjInterface *parent, aposi_t subIndex)
    : AstroSignalAttachment<AstroActuator *, ASTRO_ACTUATOR_SIGNAL_SLOTS>(parent, subIndex, &AstroActuator::getActivationSignal),
       _actHandle(), _actSetup(), _updateSlot(nullptr), _rateMultiplier(1.0f), _calledLastUpdate(false)
{ ; }

AstroActuatorAttachment::AstroActuatorAttachment(const AstroActuatorAttachment &attachment)
    : AstroSignalAttachment<AstroActuator *, ASTRO_ACTUATOR_SIGNAL_SLOTS>(attachment),
      _actHandle(attachment._actHandle), _actSetup(attachment._actSetup),
      _updateSlot(attachment._updateSlot ? attachment._updateSlot->clone() : nullptr),
      _rateMultiplier(attachment._rateMultiplier), _calledLastUpdate(false)
{ ; }

AstroActuatorAttachment::~AstroActuatorAttachment()
{
    if (_updateSlot) { delete _updateSlot; _updateSlot = nullptr; }
}

void AstroActuatorAttachment::updateIfNeeded(bool poll)
{
    if (_actHandle.isValid()) {
        if (isActivated()) {
            _actHandle.elapseTo();
            if (_updateSlot) { (*_updateSlot)(this); }
            _calledLastUpdate = _actHandle.isDone();
        } else if (_actHandle.isDone() && !_calledLastUpdate) {
            if (_updateSlot) { (*_updateSlot)(this); }
            _calledLastUpdate = true;
        }
    }
}

void AstroActuatorAttachment::setupActivation(float value, millis_t duration, bool force)
{
    int direction = astroDirectionForOffset(value, FLT_EPSILON);
    setupActivation(AstroActivation(direction > 0 ? Astro_DirectionMode_Forward : direction < 0 ? Astro_DirectionMode_Reverse : Astro_DirectionMode_Stop,
                                    fabsf(value), duration, (force ? Astro_ActivationFlags_Forced : Astro_ActivationFlags_None)));
}

void AstroActuatorAttachment::enableActivation()
{
    if (!_actHandle.actuator && _actSetup.isValid() && resolve()) {
        if (_actHandle.isDone()) { applySetup(); } // repeats existing setup
        _calledLastUpdate = false;
        _actHandle = getObject();
    }
}

void AstroActuatorAttachment::setUpdateSlot(const Slot<AstroActuatorAttachment *> &updateSlot)
{
    if (!_updateSlot || !_updateSlot->operator==(&updateSlot)) {
        if (_updateSlot) { delete _updateSlot; _updateSlot = nullptr; }
        _updateSlot = updateSlot.clone();
    }
}

void AstroActuatorAttachment::applySetup()
{
    if (_actSetup.isValid()) {
        _actHandle.activation.direction = _actSetup.direction;
        _actHandle.activation.flags = _actSetup.flags;

        if (resolve() && (get()->classType == AstroActuator::Digital || get()->classType == AstroActuator::RelayMotor)) {
            _actHandle.activation.intensity = _actSetup.intensity;
            if (!_actSetup.isUntimed()) {
                _actHandle.activation.duration = _actSetup.duration * _rateMultiplier;
            } else {
                _actHandle.activation.duration = _actSetup.duration;
            }
        } else {
            _actHandle.activation.intensity = _actSetup.intensity * _rateMultiplier;
            _actHandle.activation.duration = _actSetup.duration;
        }

        if (isActivated() && resolve()) { get()->setNeedsUpdate(); }
    }
}


AstroSensorAttachment::AstroSensorAttachment(AstroObjInterface *parent, aposi_t subIndex, uint8_t measurementRow)
    : AstroSignalAttachment<const AstroMeasurement *, ASTRO_SENSOR_SIGNAL_SLOTS>(parent, subIndex, &AstroSensor::getMeasurementSignal),
      _measurementRow(measurementRow), _convertParam(FLT_UNDEF), _needsMeasurement(true)
{
    setHandleMethod(&AstroSensorAttachment::handleMeasurement, this);
}

AstroSensorAttachment::AstroSensorAttachment(const AstroSensorAttachment &attachment)
    : AstroSignalAttachment<const AstroMeasurement *, ASTRO_SENSOR_SIGNAL_SLOTS>(attachment),
      _measurement(attachment._measurement), _measurementRow(attachment._measurementRow),
      _convertParam(attachment._convertParam), _needsMeasurement(attachment._needsMeasurement)
{
    setHandleSlot(*attachment._handleSlot);
}

AstroSensorAttachment::~AstroSensorAttachment()
{ ; }

void AstroSensorAttachment::attachObject()
{
    AstroSignalAttachment<const AstroMeasurement *, ASTRO_SENSOR_SIGNAL_SLOTS>::attachObject();

    if (_handleSlot) { (*_handleSlot)(get()->getMeasurement()); }
    else { handleMeasurement(get()->getMeasurement()); }
}

void AstroSensorAttachment::detachObject()
{
    AstroSignalAttachment<const AstroMeasurement *, ASTRO_SENSOR_SIGNAL_SLOTS>::detachObject();

    setNeedsMeasurement();
}

void AstroSensorAttachment::updateIfNeeded(bool poll)
{
    if ((poll || _needsMeasurement) && resolve()) {
        if (_handleSlot) { (*_handleSlot)(get()->getMeasurement()); }
        else { handleMeasurement(get()->getMeasurement()); }

        get()->takeMeasurement((poll || _needsMeasurement)); // purposeful recheck
    }
}

void AstroSensorAttachment::setMeasurement(AstroSingleMeasurement measurement)
{
    auto outUnits = definedUnitsElse(getMeasurementUnits(), measurement.units);
    _measurement = measurement;
    _measurement.setMinFrame(1);

    convertUnits(&_measurement, outUnits, _convertParam);
    _needsMeasurement = false;
}

void AstroSensorAttachment::setMeasurementRow(uint8_t measurementRow)
{
    if (_measurementRow != measurementRow) {
        _measurementRow = measurementRow;

        setNeedsMeasurement();
    }
}

void AstroSensorAttachment::setMeasurementUnits(Astro_UnitsType units, float convertParam)
{
    if (_measurement.units != units || !isFPEqual(_convertParam, convertParam)) {
        _convertParam = convertParam;
        convertUnits(&_measurement, units, _convertParam);

        setNeedsMeasurement();
    }
}

void AstroSensorAttachment::handleMeasurement(const AstroMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        setMeasurement(getAsSingleMeasurement(measurement, _measurementRow));
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


AstroTriggerAttachment::AstroTriggerAttachment(AstroObjInterface *parent, aposi_t subIndex)
    : AstroSignalAttachment<Astro_TriggerState, ASTRO_TRIGGER_SIGNAL_SLOTS>(parent, subIndex, &AstroTrigger::getTriggerSignal)
{ ; }

AstroTriggerAttachment::AstroTriggerAttachment(const AstroTriggerAttachment &attachment)
    : AstroSignalAttachment<Astro_TriggerState, ASTRO_TRIGGER_SIGNAL_SLOTS>(attachment)
{ ; }

AstroTriggerAttachment::~AstroTriggerAttachment()
{ ; }

void AstroTriggerAttachment::updateIfNeeded(bool poll)
{
    if (poll && resolve()) { get()->update(); }
}
