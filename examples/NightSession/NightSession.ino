// Simple-AstroTracker-Arduino Night Session Example
//
// Demonstrates the scheduler driving a simple homemade setup with a mount, powered
// cover, camera trigger, weather safety input, and basic environmental readings.

#include <Astruino.h>

#define SETUP_LATITUDE                  49.2827
#define SETUP_LONGITUDE                -123.1207
#define SETUP_ELEVATION                70.0
#define SETUP_START_UNIX               1787101200LL

Astruino astroController(Astro_MountType_Equatorial);

static float coverPower = 0.0f;
static bool cameraActive = false;
static double ambientTemperatureC = 8.0;
static double humidityPercent = 78.0;
static double opticsTemperatureC = 8.5;
static double cameraSensorTemperatureC = -9.5;
static double cameraBodyTemperatureC = 7.0;
static double rainState = 0.0;

void driveCover(void *context, float power)
{
    (void)context;
    coverPower = power;
    // Drive a relay/H-bridge/servo here. Positive opens, negative closes.
}

void triggerCamera(void *context, bool active)
{
    (void)context;
    cameraActive = active;
    digitalWrite(LED_BUILTIN, active ? HIGH : LOW); // Replace with shutter/record pin if desired.
}

bool readValue(void *context, double *valueOut)
{
    if (!context || !valueOut) { return false; }
    *valueOut = *(double *)context;
    return true;
}

void logEvent(void *context, const AstroLogEvent &event)
{
    (void)context;
    Serial.print(event.prefix);
    Serial.println(event.message);
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }
    pinMode(LED_BUILTIN, OUTPUT);

    AstroObserver observer(SETUP_LATITUDE, SETUP_LONGITUDE, SETUP_ELEVATION);
    astroController.init();
    astroController.setObserver(observer);
    setTime((time_t)SETUP_START_UNIX);

    auto coverActuator = astroController.addCallbackActuator(driveCover, Astro_ActuatorType_Cover);
    astroController.getCover().setActuator(coverActuator);
    astroController.getCover().setTravelRate(0.15f);

    auto ambientSensor = astroController.addCallbackSensor(readValue, Astro_SensorType_Temperature,
                                                            Astro_UnitsType_Temperature_Celsius, &ambientTemperatureC);
    auto humiditySensor = astroController.addCallbackSensor(readValue, Astro_SensorType_Humidity,
                                                             Astro_UnitsType_Humidity_RH, &humidityPercent);
    auto opticsSensor = astroController.addCallbackSensor(readValue, Astro_SensorType_Temperature,
                                                           Astro_UnitsType_Temperature_Celsius, &opticsTemperatureC);
    auto cameraSensor = astroController.addCallbackSensor(readValue, Astro_SensorType_CameraTemperature,
                                                           Astro_UnitsType_Temperature_Celsius, &cameraSensorTemperatureC);
    auto cameraBodySensor = astroController.addCallbackSensor(readValue, Astro_SensorType_Temperature,
                                                               Astro_UnitsType_Temperature_Celsius, &cameraBodyTemperatureC);
    auto rainSensor = astroController.addCallbackSensor(readValue, Astro_SensorType_Rain,
                                                         Astro_UnitsType_Raw_1, &rainState);

    astroController.getThermalBalancer().setAmbientTemperatureSensor(ambientSensor);
    astroController.getThermalBalancer().setHumiditySensor(humiditySensor);
    astroController.getThermalBalancer().setOpticsTemperatureSensor(opticsSensor);
    astroController.getThermalBalancer().setCameraSensorTemperatureSensor(cameraSensor);
    astroController.getThermalBalancer().setCameraBodyTemperatureSensor(cameraBodySensor);

    auto rainTrigger = astroController.addThresholdTrigger(rainSensor, 0.5, false, 0.0, 1000);
    astroController.getScheduler().setSafetyTrigger(rainTrigger);

    astroController.getCamera().setTriggerCallback(triggerCamera);
    astroController.getCamera().setReady(true);
    astroController.getLogger().setSink(logEvent);
    astroController.getScheduler().setTarget(Astro_Target_M31);

    astroController.launch();
}

void loop()
{
    // Replace these example values with installed sensors.
    ambientTemperatureC -= 0.0002;

    astroController.update();

    static millis_t lastReport = 0;
    millis_t now = millis();
    if (now - lastReport >= 5000) {
        lastReport = now;
        Serial.print(F("Cover: "));
        Serial.print(astroController.getCover().getPosition(), 2);
        Serial.print(F(" power: "));
        Serial.print(coverPower, 2);
        Serial.print(F(" camera: "));
        Serial.println(cameraActive ? F("recording") : F("idle"));
    }
}
