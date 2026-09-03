#include "Astruino.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void testSystemData()
{
    AstroSystemData data;
    strncpy(data.systemName, "Backyard Tracker", sizeof(data.systemName) - 1);
    data.systemName[sizeof(data.systemName) - 1] = '\0';
    data.systemMode = Astro_SystemMode_Balancing;
    data.measureMode = Astro_MeasurementMode_Scientific;
    data.latitude = 49.2827;
    data.longitude = -123.1207;
    data.altitude = 70.0;
    data.scheduler.preDuskHeatingMins = 12;
    data.scheduler.reportInterval = 3600;
    data.logger.logLevel = Astro_LogLevel_Warnings;

    StaticJsonDocument<1024> doc;
    JsonObject object = doc.to<JsonObject>();
    data.toJSONObject(object);

    AstroSystemData decoded;
    JsonObjectConst objectConst = doc.as<JsonObjectConst>();
    decoded.fromJSONObject(objectConst);
    assert(decoded.isSystemData());
    assert(strcmp(decoded.systemName, data.systemName) == 0);
    assert(decoded.systemMode == data.systemMode);
    assert(decoded.measureMode == data.measureMode);
    assert(isFPEqual(decoded.latitude, data.latitude));
    assert(isFPEqual(decoded.longitude, data.longitude));
    assert(isFPEqual(decoded.altitude, data.altitude));
    assert(decoded.scheduler.preDuskHeatingMins == data.scheduler.preDuskHeatingMins);
    assert(decoded.scheduler.reportInterval == data.scheduler.reportInterval);
    assert(decoded.logger.logLevel == data.logger.logLevel);
    assert(decoded.publisher.pubToSDCard == data.publisher.pubToSDCard);

    AstroData *allocated = newDataFromJSONObject(objectConst);
    assert(allocated && allocated->isSystemData());
    delete allocated;
}

static void testCalibrationData()
{
    AstroCalibrationData data(AstroIdentity(Astro_SensorType_Temperature, 1), Astro_UnitsType_Temperature_Celsius);
    data.setFromTwoPoints(0.1, -10.0, 0.9, 30.0);
    assert(isFPEqual(data.transform(0.5f), 10.0f));
    assert(isFPEqual(data.inverseTransform(10.0f), 0.5f));

    StaticJsonDocument<256> doc;
    JsonObject object = doc.to<JsonObject>();
    data.toJSONObject(object);
    JsonObjectConst objectConst = doc.as<JsonObjectConst>();

    AstroCalibrationData decoded;
    decoded.fromJSONObject(objectConst);
    assert(decoded.isCalibrationData());
    assert(strcmp(decoded.ownerName, data.ownerName) == 0);
    assert(decoded.calibrationUnits == data.calibrationUnits);
    assert(isFPEqual(decoded.multiplier, data.multiplier));
    assert(isFPEqual(decoded.offset, data.offset));
}

static void testActuatorData()
{
    AstroActuatorData data;
    data.id.object.idType = AstroIdentity::Actuator;
    data.id.object.objType = Astro_ActuatorType_Cover;
    data.id.object.posIndex = 2;
    data.id.object.classType = AstroActuator::RelayMotor;
    data.enableMode = Astro_EnableMode_InOrder;
    AstroDigitalPin(8, Astro_PinMode_Digital_Output, false).saveToData(&data.outputPin);
    AstroDigitalPin(9, Astro_PinMode_Digital_Output, true).saveToData(&data.outputPin2);

    StaticJsonDocument<512> doc;
    JsonObject object = doc.to<JsonObject>();
    data.toJSONObject(object);
    JsonObjectConst objectConst = doc.as<JsonObjectConst>();

    AstroData *allocated = newDataFromJSONObject(objectConst);
    assert(allocated && allocated->isObjectData());
    AstroActuatorData *decoded = static_cast<AstroActuatorData *>(allocated);
    assert(decoded->id.object.idType == AstroIdentity::Actuator);
    assert(decoded->id.object.objType == Astro_ActuatorType_Cover);
    assert(decoded->id.object.posIndex == 2);
    assert(decoded->id.object.classType == AstroActuator::RelayMotor);
    assert(decoded->enableMode == Astro_EnableMode_InOrder);
    assert(decoded->outputPin.pin == 8);
    assert(decoded->outputPin2.pin == 9);
    assert(decoded->outputPin2.dataAs.digitalPin.activeLow);
    delete allocated;
}


static void testBinarySensorData()
{
    AstroBinarySensorData data;
    data.id.object.idType = AstroIdentity::Sensor;
    data.id.object.objType = Astro_SensorType_LimitSwitch;
    data.id.object.posIndex = 1;
    data.id.object.classType = AstroSensor::Binary;
    AstroDigitalPin(14, Astro_PinMode_Digital_Input_PullUp, true).saveToData(&data.inputPin);
    data.stateStableTimeMs = 75;

    StaticJsonDocument<384> doc;
    JsonObject object = doc.to<JsonObject>();
    data.toJSONObject(object);
    JsonObjectConst objectConst = doc.as<JsonObjectConst>();

    AstroData *allocated = newDataFromJSONObject(objectConst);
    assert(allocated && allocated->isObjectData());
    AstroBinarySensorData *decoded = static_cast<AstroBinarySensorData *>(allocated);
    assert(decoded->id.object.classType == AstroSensor::Binary);
    assert(decoded->inputPin.pin == 14);
    assert(decoded->inputPin.dataAs.digitalPin.activeLow);
    assert(decoded->stateStableTimeMs == 75);

    AstroSensor *sensor = newSensorObjectFromData(decoded);
    assert(sensor && sensor->isBinaryClass() && !sensor->isDigitalClass());
    delete sensor;
    delete allocated;
}

static void testTriggerSubData()
{
    AstroTriggerSubData data;
    data.type = AstroTrigger::MeasureValue;
    strncpy(data.sensorName, "Temperature #1", sizeof(data.sensorName) - 1);
    data.measurementRow = 0;
    data.measurementUnits = Astro_UnitsType_Temperature_Celsius;
    data.detriggerTol = 0.5;
    data.detriggerDelay = 1000;
    data.dataAs.measureValue.tolerance = 5.0;
    data.dataAs.measureValue.triggerBelow = true;

    StaticJsonDocument<384> doc;
    JsonObject object = doc.to<JsonObject>();
    data.toJSONObject(object);
    JsonObjectConst objectConst = doc.as<JsonObjectConst>();

    AstroTriggerSubData decoded;
    decoded.fromJSONObject(objectConst);
    assert(decoded.type == data.type);
    assert(strcmp(decoded.sensorName, data.sensorName) == 0);
    assert(decoded.measurementUnits == data.measurementUnits);
    assert(isFPEqual(decoded.detriggerTol, data.detriggerTol));
    assert(decoded.detriggerDelay == data.detriggerDelay);
    assert(isFPEqual(decoded.dataAs.measureValue.tolerance, data.dataAs.measureValue.tolerance));
    assert(decoded.dataAs.measureValue.triggerBelow == data.dataAs.measureValue.triggerBelow);
}

int main()
{
    testSystemData();
    testCalibrationData();
    testActuatorData();
    testBinarySensorData();
    testTriggerSubData();
    puts("PASS serialization");
    return 0;
}
