#include "Astruino.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void testObjectData()
{
    AstroMount mount(Astro_MountType_AltAz, 3);
    mount.unsetModified();
    mount.bumpRevisionIfNeeded();

    AstroObjectData *saveData = mount.newSaveData();
    assert(saveData);
    assert(saveData->idType == AstroIdentity::Mount);
    assert(saveData->objType == Astro_MountType_AltAz);
    assert(saveData->posIndex == 3);

    char json[256];
    assert(saveData->toJSON(json, sizeof(json)));

    AstroObjectData decoded;
    assert(decoded.fromJSON(json));
    assert(decoded.idType == saveData->idType);
    assert(decoded.objType == saveData->objType);
    assert(decoded.posIndex == saveData->posIndex);
    assert(decoded.revision == saveData->revision);
    assert(strcmp(decoded.name, saveData->name) == 0);

    AstroObject *restored = AstroFactory::newObjectFromData(&decoded);
    assert(restored);
    assert(restored->getId().isMountType());
    assert(restored->getId().objTypeAs.mountType == Astro_MountType_AltAz);
    assert(restored->getId().posIndex == 3);
    assert(restored->getRevision() == decoded.revision);
    assert(restored->getKeyString() == AstroString(decoded.name));

    delete restored;
    delete saveData;
}

static void testFactoryObjects()
{
    AstroObjectData data;
    data.revision = 7;
    data.posIndex = 2;

    data.idType = AstroIdentity::Actuator;
    data.objType = Astro_ActuatorType_DewHeater;
    snprintf(data.name, sizeof(data.name), "DewHeater #2");
    AstroObject *object = AstroFactory::newObjectFromData(&data);
    assert(object && object->getId().isActuatorType());
    assert(object->getRevision() == 7);
    delete object;

    data.idType = AstroIdentity::Sensor;
    data.objType = Astro_SensorType_Humidity;
    snprintf(data.name, sizeof(data.name), "Humidity #2");
    object = AstroFactory::newObjectFromData(&data);
    assert(object && object->getId().isSensorType());
    assert(object->getRevision() == 7);
    delete object;

    data.idType = AstroIdentity::Rail;
    data.objType = Astro_RailType_DC12V;
    snprintf(data.name, sizeof(data.name), "DC12V #2");
    object = AstroFactory::newObjectFromData(&data);
    assert(object && object->getId().isRailType());
    assert(isFPEqual(static_cast<AstroRail *>(object)->getVoltage(), 12.0));
    delete object;

    data.idType = AstroIdentity::Cover;
    data.objType = 0;
    snprintf(data.name, sizeof(data.name), "Cover #2");
    object = AstroFactory::newObjectFromData(&data);
    assert(object && object->getId().isCoverType());
    delete object;

    data.idType = AstroIdentity::ObservationDevice;
    snprintf(data.name, sizeof(data.name), "Observation #2");
    object = AstroFactory::newObjectFromData(&data);
    assert(object && object->getId().isObservationDeviceType());
    delete object;
}

static void testMeasurementData()
{
    AstroMeasurementData data;
    data.measurementRow = 2;
    data.value = 12.345678;
    data.units = Astro_UnitsType_Temperature_Celsius;
    data.timestamp = 1787101200LL;
    data.frame = 42;

    char json[256];
    assert(data.toJSON(json, sizeof(json)));

    AstroMeasurementData decoded;
    assert(decoded.fromJSON(json));
    assert(decoded.measurementRow == data.measurementRow);
    assert(isFPEqual(decoded.value, data.value));
    assert(decoded.units == data.units);
    assert(decoded.timestamp == data.timestamp);
    assert(decoded.frame == data.frame);
}

static void testSystemData()
{
    AstroSystemData data;
    snprintf(data.systemName, sizeof(data.systemName), "Backyard Tracker");
    data.systemMode = Astro_SystemMode_Balancing;
    data.measurementMode = Astro_MeasurementMode_Scientific;
    data.observer = AstroObserver(49.2827, -123.1207, 70.0);
    data.scheduler.deploySunAltitudeDegrees = -12.0;
    data.scheduler.stowSunAltitudeDegrees = -6.0;
    data.scheduler.alignmentToleranceDegrees = 0.35;
    data.scheduler.settleSeconds = 12;
    data.scheduler.reportIntervalSeconds = 90;
    data.logger.logLevel = Astro_LogLevel_Warnings;
    snprintf(data.logger.logFilePrefix, sizeof(data.logger.logFilePrefix), "logs/test");
    data.logger.logToSDCard = true;
    snprintf(data.publisher.dataFilePrefix, sizeof(data.publisher.dataFilePrefix), "data/test");
    data.publisher.pubToMQTT = true;

    char json[512];
    assert(data.toJSON(json, sizeof(json)));

    AstroSystemData decoded;
    assert(decoded.fromJSON(json));
    assert(strcmp(decoded.systemName, data.systemName) == 0);
    assert(decoded.systemMode == data.systemMode);
    assert(decoded.measurementMode == data.measurementMode);
    assert(isFPEqual(decoded.observer.latitudeDegrees, data.observer.latitudeDegrees));
    assert(isFPEqual(decoded.observer.longitudeDegrees, data.observer.longitudeDegrees));
    assert(decoded.scheduler.settleSeconds == data.scheduler.settleSeconds);
    assert(decoded.scheduler.reportIntervalSeconds == data.scheduler.reportIntervalSeconds);
    assert(decoded.logger.logLevel == data.logger.logLevel);
    assert(strcmp(decoded.logger.logFilePrefix, data.logger.logFilePrefix) == 0);
    assert(decoded.logger.logToSDCard == data.logger.logToSDCard);
    assert(strcmp(decoded.publisher.dataFilePrefix, data.publisher.dataFilePrefix) == 0);
    assert(decoded.publisher.pubToMQTT == data.publisher.pubToMQTT);
}

static void testCalibrationData()
{
    AstroCalibrationData data(AstroIdentity(Astro_SensorType_Temperature, 1), Astro_UnitsType_Temperature_Celsius);
    data.setFromTwoPoints(0.1, -10.0, 0.9, 30.0);
    assert(isFPEqual(data.transform(0.5), 10.0));
    assert(isFPEqual(data.inverseTransform(10.0), 0.5));

    char json[256];
    assert(data.toJSON(json, sizeof(json)));
    AstroCalibrationData decoded;
    assert(decoded.fromJSON(json));
    assert(decoded.calibrationUnits == data.calibrationUnits);
    assert(isFPEqual(decoded.multiplier, data.multiplier));
    assert(isFPEqual(decoded.offset, data.offset));
}

int main()
{
    testObjectData();
    testFactoryObjects();
    testMeasurementData();
    testSystemData();
    testCalibrationData();
    puts("PASS serialization");
    return 0;
}
