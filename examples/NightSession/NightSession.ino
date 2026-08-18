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

static millis_t lastUpdate = 0;
static float coverPower = 0.0f;
static bool cameraActive = false;

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
    astroController.launch();

    static AstroCallbackActuator coverActuator(driveCover, nullptr, Astro_ActuatorType_Cover);
    astroController.getCover().setActuator(&coverActuator);
    astroController.getCover().setTravelRate(0.15f);

    astroController.getCamera().setTriggerCallback(triggerCamera);
    astroController.getCamera().setReady(true);
    astroController.getLogger().setSink(logEvent);

    astroController.getScheduler().setTarget(Astro_Target_M31);

    lastUpdate = millis();
}

void loop()
{
    millis_t now = millis();
    double elapsedSeconds = (now - lastUpdate) / 1000.0;
    lastUpdate = now;
    int64_t unixTime = SETUP_START_UNIX + (now / 1000);

    // Replace these example values with real light/weather/environment sensors.
    double seconds = now / 1000.0;
    double sunAltitude = -15.0;                            // Below twilight threshold in this example
    bool safeToObserve = true;                             // Rain/wind/limit interlocks feed this value

    AstroThermalReadings readings;
    readings.ambientTemperatureC = 8.0 - seconds * 0.0002;
    readings.humidityPercent = 78.0;
    readings.opticsTemperatureC = 8.5;
    readings.cameraSensorTemperatureC = -9.5;
    readings.cameraBodyTemperatureC = 7.0;

    astroController.update(unixTime, elapsedSeconds, sunAltitude, safeToObserve, readings);

    static millis_t lastReport = 0;
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
