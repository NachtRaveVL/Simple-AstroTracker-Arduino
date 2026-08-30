/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Datas
*/

#include "Astruino.h"

AstroData *_allocateDataFromBaseDecode(const AstroData &baseDecode)
{
    AstroData *retVal = nullptr;

    if (baseDecode.isStandardData()) {
        if (baseDecode.isSystemData()) {
            retVal = new AstroSystemData();
        } else if (baseDecode.isCalibrationData()) {
            retVal = new AstroCalibrationData();
        } else if (baseDecode.isUIData()) {
            retVal = new AstroUIData();
        }
    } else if (baseDecode.isObjectData()) {
        retVal = _allocateDataForObjType(baseDecode.id.object.idType, baseDecode.id.object.classType);
    }

    ASTRO_SOFT_ASSERT(retVal, SFP(AStr_Err_UnknownDataDecode));
    if (retVal) {
        retVal->id = baseDecode.id;
        ASTRO_SOFT_ASSERT(retVal->_version >= baseDecode._version, SFP(AStr_Err_DataVersionMismatch));
        retVal->_revision = baseDecode._revision;
        return retVal;
    }
    return new AstroData(baseDecode);
}

AstroData *_allocateDataForObjType(int8_t idType, int8_t classType)
{
    switch (idType) {
        case (aid_t)AstroIdentity::Actuator:
            switch (classType) {
                case (aid_t)AstroActuator::Base:
                    return new AstroActuatorData();
                case (aid_t)AstroActuator::Callback:
                    return new AstroActuatorData();
                case (aid_t)AstroActuator::Digital:
                    return new AstroActuatorData();
                case (aid_t)AstroActuator::RelayMotor:
                    return new AstroActuatorData();
                case (aid_t)AstroActuator::Analog:
                    return new AstroActuatorData();
                case (aid_t)AstroActuator::Focuser:
                    return new AstroActuatorData();
                default: break;
            }
            break;

        case (aid_t)AstroIdentity::Sensor:
            switch (classType) {
                case (aid_t)AstroSensor::Value:
                    return new AstroSensorData();
                case (aid_t)AstroSensor::Callback:
                    return new AstroSensorData();
                case (aid_t)AstroSensor::Digital:
                    return new AstroSensorData();
                case (aid_t)AstroSensor::Analog:
                    return new AstroSensorData();
                default: break;
            }
            break;

        case (aid_t)AstroIdentity::Target:
            switch (classType) {
                case (aid_t)AstroTarget::Static:
                    return new AstroStaticTargetData();
                case (aid_t)AstroTarget::Dynamic:
                    return new AstroDynamicTargetData();
                default: break;
            }
            break;

        case (aid_t)AstroIdentity::Mount:
            switch (classType) {
                case (aid_t)AstroMount::Mount:
                    return new AstroMountData();
                default: break;
            }
            break;

        case (aid_t)AstroIdentity::Rail:
            switch (classType) {
                case (aid_t)AstroRail::Simple:
                    return new AstroSimpleRailData();
                case (aid_t)AstroRail::Regulated:
                    return new AstroRegulatedRailData();
                default: break;
            }
            break;

        case (aid_t)AstroIdentity::ObservationDevice:
            switch (classType) {
                case (aid_t)AstroObservationDevice::CameraTrigger:
                    return new AstroObservationDeviceData();
                default: break;
            }
            break;

        default: break;
    }

    return nullptr;
}

AstroSystemData::AstroSystemData()
    : AstroData('A','S','Y','S', 1),
      systemMode(Astro_SystemMode_Undefined), measureMode(Astro_MeasurementMode_Undefined),
      dispOutMode(Astro_DisplayOutputMode_Undefined), ctrlInMode(Astro_ControlInputMode_Undefined),
      systemName{0}, timeZoneOffset(0), pollingInterval(ASTRO_DATA_LOOP_INTERVAL),
      autosaveEnabled(Astro_Autosave_Disabled), autosaveFallback(Astro_Autosave_Disabled), autosaveInterval(ASTRO_SYS_AUTOSAVE_INTERVAL),
      wifiSSID{0}, wifiPassword{0}, wifiPasswordSeed(0),
      macAddress{0},
      latitude(DBL_UNDEF), longitude(DBL_UNDEF), altitude(DBL_UNDEF)
{
    _size = sizeof(*this);
    ASTRO_HARD_ASSERT(isSystemData(), SFP(AStr_Err_OperationFailure));
    strncpy(systemName, SFP(AStr_Default_SystemName).c_str(), ASTRO_NAME_MAXSIZE);
}

void AstroSystemData::toJSONObject(JsonObject &objectOut) const
{
    AstroData::toJSONObject(objectOut);

    objectOut[SFP(AStr_Key_SystemMode)] = systemModeToString(systemMode);
    objectOut[SFP(AStr_Key_MeasureMode)] = measurementModeToString(measureMode);
    #ifdef ASTRO_USE_GUI
        objectOut[SFP(AStr_Key_DispOutMode)] = displayOutputModeToString(dispOutMode);
        objectOut[SFP(AStr_Key_CtrlInMode)] = controlInputModeToString(ctrlInMode);
    #else
        objectOut[SFP(AStr_Key_DispOutMode)] = displayOutputModeToString(Astro_DisplayOutputMode_Disabled);
        objectOut[SFP(AStr_Key_CtrlInMode)] = controlInputModeToString(Astro_ControlInputMode_Disabled);
    #endif
    if (systemName[0]) { objectOut[SFP(AStr_Key_SystemName)] = charsToString(systemName, ASTRO_NAME_MAXSIZE); }
    if (timeZoneOffset != 0) { objectOut[SFP(AStr_Key_TimeZoneOffset)] = timeZoneOffset; }
    if (pollingInterval && pollingInterval != ASTRO_DATA_LOOP_INTERVAL) { objectOut[SFP(AStr_Key_PollingInterval)] = pollingInterval; }
    if (autosaveEnabled != Astro_Autosave_Disabled) { objectOut[SFP(AStr_Key_AutosaveEnabled)] = autosaveEnabled; }
    if (autosaveFallback != Astro_Autosave_Disabled) { objectOut[SFP(AStr_Key_AutosaveFallback)] = autosaveFallback; }
    if (autosaveInterval && autosaveInterval != ASTRO_SYS_AUTOSAVE_INTERVAL) { objectOut[SFP(AStr_Key_AutosaveInterval)] = autosaveInterval; }
    if (wifiSSID[0]) { objectOut[SFP(AStr_Key_WiFiSSID)] = charsToString(wifiSSID, ASTRO_NAME_MAXSIZE); }
    if (wifiPasswordSeed) {
        objectOut[SFP(AStr_Key_WiFiPassword)] = hexStringFromBytes(wifiPassword, ASTRO_NAME_MAXSIZE);
        objectOut[SFP(AStr_Key_WiFiPasswordSeed)] = wifiPasswordSeed;
    } else if (wifiPassword[0]) {
        objectOut[SFP(AStr_Key_WiFiPassword)] = charsToString((const char *)wifiPassword, ASTRO_NAME_MAXSIZE);
    }
    if (!arrayElementsEqual<uint8_t>(macAddress, 6, 0)) {
        objectOut[SFP(AStr_Key_MACAddress)] = commaStringFromArray(macAddress, 6);
    }
    if (latitude != DBL_UNDEF && longitude != DBL_UNDEF) {
        if (altitude != DBL_UNDEF) {
            double loc[3] = {latitude,longitude,altitude};
            objectOut[SFP(AStr_Key_Location)] = commaStringFromArray(loc, 3);
        } else {
            double loc[2] = {latitude,longitude};
            objectOut[SFP(AStr_Key_Location)] = commaStringFromArray(loc, 2);
        }
    }

    JsonObject schedulerObj = objectOut.createNestedObject(SFP(AStr_Key_Scheduler));
    scheduler.toJSONObject(schedulerObj); if (!schedulerObj.size()) { objectOut.remove(SFP(AStr_Key_Scheduler)); }
    JsonObject loggerObj = objectOut.createNestedObject(SFP(AStr_Key_Logger));
    logger.toJSONObject(loggerObj); if (!loggerObj.size()) { objectOut.remove(SFP(AStr_Key_Logger)); }
    JsonObject publisherObj = objectOut.createNestedObject(SFP(AStr_Key_Publisher));
    publisher.toJSONObject(publisherObj); if (!publisherObj.size()) { objectOut.remove(SFP(AStr_Key_Publisher)); }
}

void AstroSystemData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroData::fromJSONObject(objectIn);

    systemMode = systemModeFromString(objectIn[SFP(AStr_Key_SystemMode)]);
    measureMode = measurementModeFromString(objectIn[SFP(AStr_Key_MeasureMode)]);
    #ifdef ASTRO_USE_GUI
        dispOutMode = displayOutputModeFromString(objectIn[SFP(AStr_Key_DispOutMode)]);
        ctrlInMode = controlInputModeFromString(objectIn[SFP(AStr_Key_CtrlInMode)]);
    #else
        dispOutMode = Astro_DisplayOutputMode_Disabled;
        ctrlInMode = Astro_ControlInputMode_Disabled;
    #endif
    const char *systemNameStr = objectIn[SFP(AStr_Key_SystemName)];
    if (systemNameStr && systemNameStr[0]) { strncpy(systemName, systemNameStr, ASTRO_NAME_MAXSIZE); }
    timeZoneOffset = objectIn[SFP(AStr_Key_TimeZoneOffset)] | timeZoneOffset;
    pollingInterval = objectIn[SFP(AStr_Key_PollingInterval)] | pollingInterval;
    autosaveEnabled = objectIn[SFP(AStr_Key_AutosaveEnabled)] | autosaveEnabled;
    autosaveFallback = objectIn[SFP(AStr_Key_AutosaveFallback)] | autosaveFallback;
    autosaveInterval = objectIn[SFP(AStr_Key_AutosaveInterval)] | autosaveInterval;
    const char *wifiSSIDStr = objectIn[SFP(AStr_Key_WiFiSSID)];
    if (wifiSSIDStr && wifiSSIDStr[0]) { strncpy(wifiSSID, wifiSSIDStr, ASTRO_NAME_MAXSIZE); }
    const char *wifiPasswordStr = objectIn[SFP(AStr_Key_WiFiPassword)];
    wifiPasswordSeed = objectIn[SFP(AStr_Key_WiFiPasswordSeed)] | wifiPasswordSeed;
    if (wifiPasswordStr && wifiPasswordSeed) { hexStringToBytes(String(wifiPasswordStr), wifiPassword, ASTRO_NAME_MAXSIZE); }
    else if (wifiPasswordStr && wifiPasswordStr[0]) { strncpy((char *)wifiPassword, wifiPasswordStr, ASTRO_NAME_MAXSIZE); wifiPasswordSeed = 0; }
    JsonVariantConst macAddressVar = objectIn[SFP(AStr_Key_MACAddress)];
    commaStringToArray(macAddressVar, macAddress, 6);
    JsonVariantConst locationVar = objectIn[SFP(AStr_Key_Location)];
    if (!locationVar.isNull()) {
        auto commaCount = occurrencesInString(locationVar.as<String>(), String(','));
        if (commaCount == 2) {
            double loc[3]; commaStringToArray(locationVar, loc, 3);
            latitude = loc[0]; longitude = loc[1]; altitude = loc[2];
        } else if (commaCount == 1) {
            double loc[2]; commaStringToArray(locationVar, loc, 2);
            latitude = loc[0]; longitude = loc[1];
        }
    }

    JsonObjectConst schedulerObj = objectIn[SFP(AStr_Key_Scheduler)];
    if (!schedulerObj.isNull()) { scheduler.fromJSONObject(schedulerObj); }
    JsonObjectConst loggerObj = objectIn[SFP(AStr_Key_Logger)];
    if (!loggerObj.isNull()) { logger.fromJSONObject(loggerObj); }
    JsonObjectConst publisherObj = objectIn[SFP(AStr_Key_Publisher)];
    if (!publisherObj.isNull()) { publisher.fromJSONObject(publisherObj); }
}


AstroCalibrationData::AstroCalibrationData()
    : AstroData('A','C','A','L', 1),
      ownerName{0}, calibrationUnits(Astro_UnitsType_Undefined),
      multiplier(1.0f), offset(0.0f)
{
    _size = sizeof(*this);
    ASTRO_HARD_ASSERT(isCalibrationData(), SFP(AStr_Err_OperationFailure));
}

AstroCalibrationData::AstroCalibrationData(AstroIdentity ownerId, Astro_UnitsType calibrationUnitsIn)
    : AstroData('A','C','A','L', 1),
      ownerName{0}, calibrationUnits(calibrationUnitsIn),
      multiplier(1.0f), offset(0.0f)
{
    _size = sizeof(*this);
    ASTRO_HARD_ASSERT(isCalibrationData(), SFP(AStr_Err_OperationFailure));
    if (ownerId) {
        strncpy(ownerName, ownerId.keyString.c_str(), ASTRO_NAME_MAXSIZE);
    }
}

void AstroCalibrationData::toJSONObject(JsonObject &objectOut) const
{
    AstroData::toJSONObject(objectOut);

    if (ownerName[0]) { objectOut[SFP(AStr_Key_SensorName)] = charsToString(ownerName, ASTRO_NAME_MAXSIZE); }
    if (calibrationUnits != Astro_UnitsType_Undefined) { objectOut[SFP(AStr_Key_CalibrationUnits)] = unitsTypeToSymbol(calibrationUnits); }
    objectOut[SFP(AStr_Key_Multiplier)] = multiplier;
    objectOut[SFP(AStr_Key_Offset)] = offset;
}

void AstroCalibrationData::fromJSONObject(JsonObjectConst &objectIn)
{
    AstroData::fromJSONObject(objectIn);

    const char *ownerNameStr = objectIn[SFP(AStr_Key_SensorName)];
    if (ownerNameStr && ownerNameStr[0]) { strncpy(ownerName, ownerNameStr, ASTRO_NAME_MAXSIZE); }
    calibrationUnits = unitsTypeFromSymbol(objectIn[SFP(AStr_Key_CalibrationUnits)]);
    multiplier = objectIn[SFP(AStr_Key_Multiplier)] | multiplier;
    offset = objectIn[SFP(AStr_Key_Offset)] | offset;
}

void AstroCalibrationData::setFromTwoPoints(float point1MeasuredAt, float point1CalibratedTo,
                                            float point2MeasuredAt, float point2CalibratedTo)
{
    float aTerm = point2CalibratedTo - point1CalibratedTo;
    float bTerm = point2MeasuredAt - point1MeasuredAt;
    ASTRO_SOFT_ASSERT(!isFPEqual(bTerm, 0.0f), SFP(AStr_Err_InvalidParameter));
    if (!isFPEqual(bTerm, 0.0f)) {
        multiplier = aTerm / bTerm;
        offset = ((aTerm * point2MeasuredAt) + (bTerm * point1CalibratedTo)) / bTerm;
        bumpRevisionIfNeeded();
    }
}
