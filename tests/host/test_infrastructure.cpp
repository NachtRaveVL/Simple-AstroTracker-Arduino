#include "Astruino.h"
#include <cmath>
#include <cstring>
#include <iostream>

#define CHECK(cond) do { if (!(cond)) { std::cerr << "FAIL line " << __LINE__ << ": " #cond << '\n'; return 1; } } while (0)

static bool nearly(double a, double b, double tolerance = 1.0e-6) { return std::fabs(a - b) <= tolerance; }

struct CallbackState {
    double value;
    double target;
    float power;
    bool stopped;
};

static bool readValue(void *context, double *valueOut)
{
    CallbackState *state = (CallbackState *)context;
    *valueOut = state->value;
    return true;
}

static void writePower(void *context, float power) { ((CallbackState *)context)->power = power; }
static void writeTarget(void *context, double target) { ((CallbackState *)context)->target = target; }
static void stopTarget(void *context) { ((CallbackState *)context)->stopped = true; }
static bool readAxisPosition(void *context, double *positionOut) { *positionOut = ((CallbackState *)context)->value; return true; }

struct FocuserState { int32_t target = 0; int32_t position = 0; bool stopped = false; };
static void moveFocuser(void *context, int32_t target) { ((FocuserState *)context)->target = target; }
static void stopFocuser(void *context) { ((FocuserState *)context)->stopped = true; }
static bool readFocuser(void *context, int32_t *positionOut) { *positionOut = ((FocuserState *)context)->position; return true; }

static bool getTime(void *, int64_t *timeOut) { *timeOut = 123456789; return true; }
static bool getLocation(void *, AstroObserver *observerOut) { *observerOut = AstroObserver(49.0, -123.0, 10.0); return true; }

int main()
{

    CHECK(SFP(AStr_Tracking) == AstroString("Tracking"));
    CHECK(std::strcmp(CFP(AStr_Tracking), "Tracking") == 0);
    AstroBitResolution bits(10);
    CHECK(std::abs(bits.inverseTransform(0.5f) - 512) <= 1);
    CHECK(nearly(bits.transform(512), 512.0 / 1023.0, 1.0e-5));
    CHECK(baseUnits(Astro_UnitsType_Speed_MetersPerSec) == Astro_UnitsType_Distance_Meters);
    CHECK(rateUnits(Astro_UnitsType_Distance_Feet) == Astro_UnitsType_Speed_FeetPerSec);

    AstroIdentity sensorId(Astro_SensorType_Temperature, 2);
    AstroIdentity sensorIdAgain(Astro_SensorType_Temperature, 2);
    AstroIdentity otherSensor(Astro_SensorType_Humidity, 2);
    CHECK(sensorId == sensorIdAgain);
    CHECK(sensorId != otherSensor);
    CHECK(sensorId.keyString == AstroString("Temperature #2"));

    AstroObject objectA(sensorId);
    AstroObject objectB(otherSensor);
    CHECK(objectA.addLinkage(&objectB));
    CHECK(objectA.hasLinkage(&objectB));
    CHECK(objectA.addLinkage(&objectB));
    CHECK(objectA.removeLinkage(&objectB));
    CHECK(objectA.hasLinkage(&objectB));
    CHECK(objectA.removeLinkage(&objectB));
    CHECK(!objectA.hasLinkage(&objectB));

    AstroDigitalPin digital(7, Astro_PinMode_Digital_Output, false);
    digital.init();
    digital.activate();
    CHECK(astroGetHostDigitalPin(7) == HIGH);
    digital.deactivate();
    CHECK(astroGetHostDigitalPin(7) == LOW);

    AstroAnalogPin analog(3, Astro_PinMode_Analog_Output, 10);
    analog.analogWrite(0.5f);
    CHECK(std::abs(astroGetHostAnalogPin(3) - 512) <= 1);
    astroSetHostAnalogPin(4, 256);
    AstroAnalogPin analogIn(4, Astro_PinMode_Analog_Input, 10);
    CHECK(nearly(analogIn.analogRead(), 256.0 / 1023.0, 1.0e-5));

    AstroPinData pinData;
    pinData.type = AstroPin::Digital;
    pinData.pin = 8;
    pinData.mode = Astro_PinMode_Digital_Input_PullUp;
    pinData.channel = -3;
    pinData.activeLow = true;
    char pinJson[192];
    CHECK(pinData.toJSON(pinJson, sizeof(pinJson)));
    AstroPinData pinRoundTrip;
    CHECK(pinRoundTrip.fromJSON(pinJson));
    CHECK(pinRoundTrip.pin == pinData.pin);
    CHECK(pinRoundTrip.mode == pinData.mode);
    CHECK(pinRoundTrip.channel == pinData.channel);
    CHECK(pinRoundTrip.activeLow == pinData.activeLow);
    AstroPin *pinFromData = newPinObjectFromSubData(&pinRoundTrip);
    CHECK(pinFromData != nullptr);
    CHECK(pinFromData->type == AstroPin::Digital);
    CHECK(pinFromData->pin == 8);
    delete pinFromData;
    CHECK(expanderPosForPinChannel(17) == 1);
    CHECK(pinChannelForExpanderChannel(17) == 17);

    pintype_t muxPins[2] = {10, 11};
    AstroPinMuxer mux(AstroPin(AstroPin::Digital, 12, Astro_PinMode_Digital_Input), muxPins, 2);
    mux.init();
    mux.selectChannel(3);
    CHECK(mux.getSelectedChannel() == 3);
    CHECK(astroGetHostDigitalPin(10) == HIGH);
    CHECK(astroGetHostDigitalPin(11) == HIGH);

    AstroSingleMeasurement temperature(0.0, Astro_UnitsType_Temperature_Celsius, 100, 4);
    CHECK(nearly(temperature.asUnits(Astro_UnitsType_Temperature_Fahrenheit).value, 32.0));
    AstroSingleMeasurement angle(370.0, Astro_UnitsType_Angle_Degrees_360, 100, 4);
    CHECK(nearly(angle.wrappedBy(360.0).value, 10.0));

    AstroThresholdTrigger stableTrigger(10.0, false, 0.0, 1000);
    CHECK(!stableTrigger.update(11.0, 100));
    CHECK(!stableTrigger.update(11.0, 1099));
    CHECK(stableTrigger.update(11.0, 1100));
    CHECK(stableTrigger.update(9.0, 1200));
    CHECK(!stableTrigger.update(9.0, 2200));

    CallbackState state{42.5, 0.0, 0.0f, false};
    AstroCallbackSensor sensor(readValue, &state, Astro_SensorType_Temperature, Astro_UnitsType_Temperature_Celsius, 0);
    CHECK(sensor.poll(123, 7));
    CHECK(nearly(sensor.getMeasurement().value, 42.5));
    CHECK(sensor.getMeasurement().frame == 7);

    AstroCallbackActuator actuator(writePower, &state, Astro_ActuatorType_DewHeater, 0);
    {
        AstroActivationHandle handle(&actuator, Astro_DirectionMode_Forward, 0.6f, (millis_t)-1);
        actuator.resolveActivations();
        CHECK(nearly(actuator.getPower(), 0.6f, 1.0e-5));
        CHECK(nearly(state.power, 0.6f, 1.0e-5));
    }
    actuator.resolveActivations();
    CHECK(nearly(actuator.getPower(), 0.0f));

    AstroDigitalActuator relay(AstroDigitalPin(13, Astro_PinMode_Digital_Output, true), Astro_ActuatorType_Cover);
    relay.setPower(1.0f);
    CHECK(astroGetHostDigitalPin(13) == LOW);
    relay.setPower(0.0f);
    CHECK(astroGetHostDigitalPin(13) == HIGH);

    AstroAnalogActuator pwm(AstroAnalogPin(6, Astro_PinMode_Analog_Output, 10), Astro_ActuatorType_DewHeater);
    pwm.setPower(0.25f);
    CHECK(std::abs(astroGetHostAnalogPin(6) - 256) <= 1);

    astroSetHostDigitalPin(14, LOW);
    AstroDigitalSensor limitSensor(AstroDigitalPin(14, Astro_PinMode_Digital_Input_PullUp, true), Astro_SensorType_LimitSwitch);
    CHECK(limitSensor.poll(500, 8));
    CHECK(nearly(limitSensor.getMeasurement().value, 1.0));

    astroSetHostAnalogPin(15, 768);
    AstroAnalogSensor analogSensor(AstroAnalogPin(15, Astro_PinMode_Analog_Input, 10), Astro_SensorType_Light, Astro_UnitsType_Raw_1);
    CHECK(analogSensor.poll(500, 8));
    CHECK(nearly(analogSensor.getMeasurement().value, 768.0 / 1023.0, 1.0e-5));

    AstroRail rail(Astro_RailType_DC12V, 12.0, 50.0, 0);
    CHECK(rail.requestPower(30.0));
    CHECK(!rail.requestPower(25.0));
    rail.releasePower(10.0);
    CHECK(nearly(rail.getPowerUsage(), 20.0));

    const uint8_t bytes[] = {1, 2, 3, 4};
    AstroMemoryStream stream(bytes, sizeof(bytes));
    uint8_t readBack[4] = {0};
    CHECK(stream.readBytes(readBack, sizeof(readBack)) == sizeof(readBack));
    CHECK(std::memcmp(bytes, readBack, sizeof(bytes)) == 0);
    stream.reset();
    CHECK(stream.available() == sizeof(bytes));

    AstroCallbackAxisDriver axis(writeTarget, stopTarget, &state);
    axis.setPositionCallback(readAxisPosition);
    axis.setTargetDegrees(123.25);
    CHECK(nearly(state.target, 123.25));
    double axisPosition = 0.0;
    CHECK(axis.getPositionDegrees(&axisPosition));
    CHECK(nearly(axisPosition, state.value));
    axis.stop();
    CHECK(state.stopped);

    FocuserState focuserState;
    AstroFocuser focuser(20000);
    focuser.setMoveCallback(moveFocuser, &focuserState);
    focuser.setStopCallback(stopFocuser);
    focuser.setPositionCallback(readFocuser);
    focuser.moveTo(12000);
    CHECK(focuser.isMoving());
    CHECK(focuserState.target == 12000);
    focuserState.position = 12000; focuser.update();
    CHECK(!focuser.isMoving());
    CHECK(focuser.getPosition() == 12000);
    focuser.moveBy(20000);
    CHECK(focuser.getTargetPosition() == 20000);
    focuser.halt();
    CHECK(focuserState.stopped);

    AstroManualTimeProvider manualTime(123456789);
    AstroFixedLocationProvider fixedLocation(AstroObserver(49.0, -123.0, 10.0));
    int64_t fixedUnixTime = 0;
    AstroObserver fixedObserver;
    CHECK(manualTime.getUnixTime(&fixedUnixTime));
    CHECK(fixedUnixTime == 123456789);
    CHECK(fixedLocation.getObserver(&fixedObserver));
    CHECK(nearly(fixedObserver.longitudeDegrees, -123.0));

    AstroCallbackTimeProvider timeProvider(getTime);
    AstroCallbackLocationProvider locationProvider(getLocation);
    int64_t unixTime = 0;
    AstroObserver observer;
    CHECK(timeProvider.getUnixTime(&unixTime));
    CHECK(unixTime == 123456789);
    CHECK(locationProvider.getObserver(&observer));
    CHECK(nearly(observer.latitudeDegrees, 49.0));

    AstroMount *mount = AstroFactory::newMount(Astro_MountType_Equatorial);
    CHECK(mount != nullptr);
    CHECK(mount->getMountType() == Astro_MountType_Equatorial);
    delete mount;

    AstroDigitalActuator *factoryRelay = AstroFactory::newDigitalActuator(
        Astro_ActuatorType_Cover, AstroDigitalPin(16, Astro_PinMode_Digital_Output));
    CHECK(factoryRelay != nullptr);
    factoryRelay->setPower(1.0f);
    CHECK(astroGetHostDigitalPin(16) == HIGH);
    delete factoryRelay;

    AstroDigitalActuator *factoryHeater = AstroFactory::newDewHeaterRelay(17, true);
    CHECK(factoryHeater != nullptr);
    factoryHeater->setPower(1.0f);
    CHECK(astroGetHostDigitalPin(17) == LOW);
    delete factoryHeater;

    AstroAnalogSensor *factoryPosition = AstroFactory::newPositionSensor(18, 10);
    CHECK(factoryPosition != nullptr);
    CHECK(factoryPosition->getSensorType() == Astro_SensorType_Position);
    delete factoryPosition;

    AstroThresholdTrigger *factoryTrigger = AstroFactory::newThresholdTrigger(5.0, false, 0.25, 100);
    CHECK(factoryTrigger != nullptr);
    CHECK(!factoryTrigger->update(6.0, 1));
    CHECK(factoryTrigger->update(6.0, 101));
    delete factoryTrigger;

    AstroFocuser *factoryFocuser = AstroFactory::newFocuser(15000);
    CHECK(factoryFocuser != nullptr);
    CHECK(factoryFocuser->getMaximumPosition() == 15000);
    delete factoryFocuser;

    Astruino controller;
    controller.init(Astro_SystemMode_Tracking, Astro_MeasurementMode_Metric);
    controller.setObserver(AstroObserver(49.0, -123.0, 10.0));
    AstroManualTimeProvider controllerTime(1767225600);
    controller.setTimeProvider(&controllerTime);
    controller.setSafeToObserve(true);
    controller.setThermalReadings(AstroThermalReadings());
    controller.launch();
    controller.update();
    CHECK(controller.isInitialized());
    CHECK(!controller.isSuspended());
    CHECK(nearly(controller.getObserver().latitudeDegrees, 49.0));
    controller.suspend();
    CHECK(controller.isSuspended());

    std::cout << "Infrastructure tests passed\n";
    return 0;
}
