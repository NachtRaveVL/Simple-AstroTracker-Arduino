/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Power Rails
*/

#include "Astruino.h"

AstroRail *newRailObjectFromData(const AstroRailData *dataIn)
{
    if (dataIn && !isValidType(dataIn->id.object.idType)) return nullptr;
    ASTRO_SOFT_ASSERT(dataIn && dataIn->isObjectData(), SFP(AStr_Err_InvalidParameter));

    if (dataIn && dataIn->isObjectData()) {
        switch (dataIn->id.object.classType) {
            case (aid_t)AstroRail::Simple:
                return new AstroSimpleRail((const AstroSimpleRailData *)dataIn);
            case (aid_t)AstroRail::Regulated:
                return new AstroRegulatedRail((const AstroRegulatedRailData *)dataIn);
            default: break;
        }
    }

    return nullptr;
}


AstroRail::AstroRail(Astro_RailType railType, aposi_t railIndex, int classTypeIn)
    : AstroObject(AstroIdentity(railType, railIndex)), AstroPowerUnitsInterfaceStorage(defaultPowerUnits()),
      classType(static_cast<decltype(Simple)>(classTypeIn)), _limitState(Astro_TriggerState_Undefined)
{
    allocateLinkages(ASTRO_RAILS_LINKS_BASESIZE);
}

AstroRail::AstroRail(const AstroRailData *dataIn)
    : AstroObject(dataIn), AstroPowerUnitsInterfaceStorage(definedUnitsElse(dataIn->powerUnits, defaultPowerUnits())),
      classType(static_cast<decltype(Simple)>(dataIn->id.object.classType)), _limitState(Astro_TriggerState_Undefined)
{
    allocateLinkages(ASTRO_RAILS_LINKS_BASESIZE);
}

AstroRail::~AstroRail()
{
    if (_links) {
        auto actuators = linksFilterActuators(getLinkages());
        for (auto iter = actuators.begin(); iter != actuators.end(); ++iter) { removeLinkage(*iter); }
    }
}

void AstroRail::update()
{
    AstroObject::update();

    handleLimit(triggerStateFromBool(getCapacity(true) >= 1.0f - FLT_EPSILON));
}

bool AstroRail::addLinkage(AstroObject *object)
{
    if (AstroObject::addLinkage(object)) {
        if (object->isActuatorType()) {
            ASTRO_HARD_ASSERT(isSimpleClass() || isRegulatedClass(), SFP(AStr_Err_OperationFailure));
            if (isSimpleClass()) {
                auto methodSlot = MethodSlot<AstroSimpleRail, AstroActuator *>((AstroSimpleRail *)this, &AstroSimpleRail::handleActivation);
                ((AstroActuator *)object)->getActivationSignal().attach(methodSlot);
            } else if (isRegulatedClass()) {
                auto methodSlot = MethodSlot<AstroRegulatedRail, AstroActuator *>((AstroRegulatedRail *)this, &AstroRegulatedRail::handleActivation);
                ((AstroActuator *)object)->getActivationSignal().attach(methodSlot);
            }
        }
        return true;
    }
    return false;
}

bool AstroRail::removeLinkage(AstroObject *object)
{
    if (AstroObject::removeLinkage(object)) {
        if (((AstroObject *)object)->isActuatorType()) {
            ASTRO_HARD_ASSERT(isSimpleClass() || isRegulatedClass(), SFP(AStr_Err_OperationFailure));
            if (isSimpleClass()) {
                auto methodSlot = MethodSlot<AstroSimpleRail, AstroActuator *>((AstroSimpleRail *)this, &AstroSimpleRail::handleActivation);
                ((AstroActuator *)object)->getActivationSignal().detach(methodSlot);
            } else if (isRegulatedClass()) {
                auto methodSlot = MethodSlot<AstroRegulatedRail, AstroActuator *>((AstroRegulatedRail *)this, &AstroRegulatedRail::handleActivation);
                ((AstroActuator *)object)->getActivationSignal().detach(methodSlot);
            }
        }
        return true;
    }
    return false;
}

Signal<AstroRail *, ASTRO_RAIL_SIGNAL_SLOTS> &AstroRail::getCapacitySignal()
{
    return _capacitySignal;
}

AstroData *AstroRail::allocateData() const
{
    return _allocateDataForObjType((int8_t)_id.type, (int8_t)classType);
}

void AstroRail::saveToData(AstroData *dataOut)
{
    AstroObject::saveToData(dataOut);

    dataOut->id.object.classType = (int8_t)classType;

    ((AstroRailData *)dataOut)->powerUnits = _powerUnits;
}

void AstroRail::handleLimit(Astro_TriggerState limitState)
{
    if (limitState == Astro_TriggerState_Disabled || limitState == Astro_TriggerState_Undefined) { return; }

    if (_limitState != limitState) {
        _limitState = limitState;

        if (_limitState == Astro_TriggerState_NotTriggered) {
            #ifdef ASTRO_USE_MULTITASKING
                scheduleSignalFireOnce<AstroRail *>(getSharedPtr(), _capacitySignal, this);
            #else
                _capacitySignal.fire(this);
            #endif
        }
    }
}


AstroSimpleRail::AstroSimpleRail(Astro_RailType railType, aposi_t railIndex, int maxActiveAtOnce, int classType)
    : AstroRail(railType, railIndex, classType), _activeCount(0), _maxActiveAtOnce(maxActiveAtOnce)
{ ; }

AstroSimpleRail::AstroSimpleRail(const AstroSimpleRailData *dataIn)
    : AstroRail(dataIn), _activeCount(0), _maxActiveAtOnce(dataIn->maxActiveAtOnce)
{ ; }

bool AstroSimpleRail::canActivate(AstroActuator *actuator)
{
    (void)actuator;
    return _activeCount < _maxActiveAtOnce;
}

float AstroSimpleRail::getCapacity(bool poll)
{
    (void)poll;
    return _activeCount / (float)_maxActiveAtOnce;
}

void AstroSimpleRail::setPowerUnits(Astro_UnitsType powerUnits)
{
    if (_powerUnits != powerUnits) {
        _powerUnits = powerUnits;
    }
}

void AstroSimpleRail::saveToData(AstroData *dataOut)
{
    AstroRail::saveToData(dataOut);

    ((AstroSimpleRailData *)dataOut)->maxActiveAtOnce = _maxActiveAtOnce;
}

void AstroSimpleRail::handleActivation(AstroActuator *actuator)
{
    bool activeCountBefore = _activeCount;

    if (actuator->isEnabled()) {
        _activeCount++;
    } else {
        _activeCount--;
    }

    if (_activeCount < activeCountBefore) {
        #ifdef ASTRO_USE_MULTITASKING
            scheduleSignalFireOnce<AstroRail *>(getSharedPtr(), _capacitySignal, this);
        #else
            _capacitySignal.fire(this);
        #endif
    }
}


AstroRegulatedRail::AstroRegulatedRail(Astro_RailType railType, aposi_t railIndex, float maxPower, int classType)
    : AstroRail(railType, railIndex, classType), _maxPower(maxPower), _powerUsage(this), _limitTrigger(this)
{
    _powerUsage.setMeasurementUnits(getPowerUnits(), getRailVoltage());
    _powerUsage.setHandleMethod(&AstroRegulatedRail::handlePower, this);

    _limitTrigger.setHandleMethod(&AstroRail::handleLimit, this);
}

AstroRegulatedRail::AstroRegulatedRail(const AstroRegulatedRailData *dataIn)
    : AstroRail(dataIn),
      _maxPower(dataIn->maxPower),
      _powerUsage(this), _limitTrigger(this)
{
    _powerUsage.setMeasurementUnits(AstroRail::getPowerUnits(), getRailVoltage());
    _powerUsage.setHandleMethod(&AstroRegulatedRail::handlePower, this);
    _powerUsage.initObject(dataIn->powerUsageSensor);

    _limitTrigger.setHandleMethod(&AstroRail::handleLimit, this);
    _limitTrigger.setObject(newTriggerObjectFromSubData(&(dataIn->limitTrigger)));
    ASTRO_SOFT_ASSERT(_limitTrigger, SFP(AStr_Err_AllocationFailure));
}

void AstroRegulatedRail::update()
{
    AstroRail::update();

    _powerUsage.updateIfNeeded(true);

    _limitTrigger.updateIfNeeded();
}

SharedPtr<AstroObjInterface> AstroRegulatedRail::getSharedPtrFor(const AstroObjInterface *obj) const
{
    return obj->getKey() == _limitTrigger.getKey() ? _limitTrigger.getSharedPtrFor(obj) :
           AstroObject::getSharedPtrFor(obj);
}

bool AstroRegulatedRail::canActivate(AstroActuator *actuator)
{
    if (_limitTrigger.isTriggered()) { return false; }
    AstroSingleMeasurement powerReq = actuator->getContinuousPowerUsage().asUnits(getPowerUnits(), getRailVoltage());
    return _powerUsage.getMeasurementValue(true) + powerReq.value < (ASTRO_RAILS_FRACTION_SATURATED * _maxPower) - FLT_EPSILON;
}

float AstroRegulatedRail::getCapacity(bool poll)
{
    if (_limitTrigger.isTriggered(poll)) { return 1.0f; }
    return _powerUsage.getMeasurementValue(poll) / (ASTRO_RAILS_FRACTION_SATURATED * _maxPower);
}

void AstroRegulatedRail::setPowerUnits(Astro_UnitsType powerUnits)
{
    if (_powerUnits != powerUnits) {
        _powerUnits = powerUnits;

        _powerUsage.setMeasurementUnits(getPowerUnits(), getRailVoltage());
        bumpRevisionIfNeeded();
    }
}

AstroSensorAttachment &AstroRegulatedRail::getPowerUsageSensorAttachment()
{
    return _powerUsage;
}

AstroTriggerAttachment &AstroRegulatedRail::getLimitTriggerAttachment()
{
    return _limitTrigger;
}

void AstroRegulatedRail::saveToData(AstroData *dataOut)
{
    AstroRail::saveToData(dataOut);

    ((AstroRegulatedRailData *)dataOut)->maxPower = roundForExport(_maxPower, 1);
    if (_powerUsage.isSet()) {
        strncpy(((AstroRegulatedRailData *)dataOut)->powerUsageSensor, _powerUsage.getKeyString().c_str(), ASTRO_NAME_MAXSIZE);
    }
    if (_limitTrigger.isSet()) {
        _limitTrigger->saveToData(&(((AstroRegulatedRailData *)dataOut)->limitTrigger));
    }
}

void AstroRegulatedRail::handleActivation(AstroActuator *actuator)
{
    if (!getPowerUsageSensor(true) && actuator) {
        auto powerReq = actuator->getContinuousPowerUsage().asUnits(getPowerUnits(), getRailVoltage());
        auto powerUsage = getPowerUsageSensorAttachment().getMeasurement(true);
        bool enabled = actuator->isEnabled();

        if (enabled) {
            powerUsage.value += powerReq.value;
        } else {
            powerUsage.value -= powerReq.value;
        }

        getPowerUsageSensorAttachment().setMeasurement(powerUsage);

        if (!enabled) {
            #ifdef ASTRO_USE_MULTITASKING
                scheduleSignalFireOnce<AstroRail *>(getSharedPtr(), _capacitySignal, this);
            #else
                _capacitySignal.fire(this);
            #endif
        }
    }
}

void AstroRegulatedRail::handlePower(const AstroMeasurement *measurement)
{
    if (measurement && measurement->frame) {
        float capacityBefore = getCapacity();

        getPowerUsageSensorAttachment().setMeasurement(getAsSingleMeasurement(measurement, _powerUsage.getMeasurementRow(), _maxPower, getPowerUnits()));

        if (getCapacity() < capacityBefore - FLT_EPSILON) {
            #ifdef ASTRO_USE_MULTITASKING
                scheduleSignalFireOnce<AstroRail *>(getSharedPtr(), _capacitySignal, this);
            #else
                _capacitySignal.fire(this);
            #endif
        }
    }
}


AstroRailData::AstroRailData()
    : AstroObjectData(), powerUnits(Astro_UnitsType_Undefined)
{
    _size = sizeof(*this);
}

void AstroRailData::toJSONObject(JsonObject &objectOut) const
{
    AstroObjectData::toJSONObject(objectOut);

    if (powerUnits != Astro_UnitsType_Undefined) { objectOut[SFP(AStr_Key_PowerUnits)] = unitsTypeToSymbol(powerUnits); }
}

void AstroRailData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroObjectData::fromJSONObject(objectIn);

    powerUnits = unitsTypeFromSymbol(objectIn[SFP(AStr_Key_PowerUnits)]);
}

AstroSimpleRailData::AstroSimpleRailData()
    : AstroRailData(), maxActiveAtOnce(2)
{
    _size = sizeof(*this);
}

void AstroSimpleRailData::toJSONObject(JsonObject &objectOut) const
{
    AstroRailData::toJSONObject(objectOut);

    if (maxActiveAtOnce != 2) { objectOut[SFP(AStr_Key_MaxActiveAtOnce)] = maxActiveAtOnce; }
}

void AstroSimpleRailData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroRailData::fromJSONObject(objectIn);

    maxActiveAtOnce = objectIn[SFP(AStr_Key_MaxActiveAtOnce)] | maxActiveAtOnce;
}

AstroRegulatedRailData::AstroRegulatedRailData()
    : AstroRailData(), maxPower(0), powerUsageSensor{0}, limitTrigger()
{
    _size = sizeof(*this);
}

void AstroRegulatedRailData::toJSONObject(JsonObject &objectOut) const
{
    AstroRailData::toJSONObject(objectOut);

    objectOut[SFP(AStr_Key_MaxPower)] = maxPower;
    if (powerUsageSensor[0]) { objectOut[SFP(AStr_Key_PowerUsageSensor)] = charsToString(powerUsageSensor, ASTRO_NAME_MAXSIZE); }
    if (isValidType(limitTrigger.type)) {
        JsonObject limitTriggerObj = objectOut.createNestedObject(SFP(AStr_Key_LimitTrigger));
        limitTrigger.toJSONObject(limitTriggerObj);
    }
}

void AstroRegulatedRailData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroRailData::fromJSONObject(objectIn);

    maxPower = objectIn[SFP(AStr_Key_MaxPower)] | maxPower;
    const char *powerUsageSensorStr = objectIn[SFP(AStr_Key_PowerUsageSensor)];
    if (powerUsageSensorStr && powerUsageSensorStr[0]) { strncpy(powerUsageSensor, powerUsageSensorStr, ASTRO_NAME_MAXSIZE); }
    JsonObjectConst limitTriggerObj = objectIn[SFP(AStr_Key_LimitTrigger)];
    if (!limitTriggerObj.isNull()) { limitTrigger.fromJSONObject(limitTriggerObj); }
}
