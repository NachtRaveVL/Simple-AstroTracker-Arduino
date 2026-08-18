// Simple-AstroTracker-Arduino Camera Thermal Example
//
// Experimental thermal balancing example. Camera cooling is intentionally kept generic
// so a TEC, fan, heater, or commercial controller can be attached through an actuator.

#include <Astruino.h>

static float coolerPower = 0.0f;
static float fanPower = 0.0f;
static float dewHeaterPower = 0.0f;
static millis_t lastUpdate = 0;

Astruino astroController;

void driveCooler(void *context, float power)
{
    (void)context;
    coolerPower = power;
    // analogWrite() or send the requested power to a TEC controller here.
}

void driveFan(void *context, float power)
{
    (void)context;
    fanPower = power;
}

void driveDewHeater(void *context, float power)
{
    (void)context;
    dewHeaterPower = power;
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }

    static AstroCallbackActuator cooler(driveCooler, nullptr, Astro_ActuatorType_CameraCooler);
    static AstroCallbackActuator fan(driveFan, nullptr, Astro_ActuatorType_Fan);
    static AstroCallbackActuator dewHeater(driveDewHeater, nullptr, Astro_ActuatorType_DewHeater);

    auto &thermal = astroController.getThermalBalancer();
    thermal.setCameraCooler(&cooler);
    thermal.setCameraFan(&fan);
    thermal.setDewHeater(&dewHeater);
    thermal.setMode(Astro_ThermalMode_NightObserving);
    thermal.setCameraTarget(-10.0);
    thermal.setCameraCoolingRamp(2.0);
    thermal.setDewMargin(3.0);

    lastUpdate = millis();
}

void loop()
{
    millis_t now = millis();
    double elapsedSeconds = (now - lastUpdate) / 1000.0;
    lastUpdate = now;

    // Replace these with real sensors attached to the camera/optics.
    AstroThermalReadings readings;
    readings.ambientTemperatureC = 7.0;
    readings.humidityPercent = 86.0;
    readings.opticsTemperatureC = 7.3;
    readings.cameraSensorTemperatureC = -8.5;
    readings.cameraBodyTemperatureC = 6.5;

    AstroThermalOutputs outputs = astroController.getThermalBalancer().update(readings, elapsedSeconds);

    static millis_t lastReport = 0;
    if (now - lastReport >= 3000) {
        lastReport = now;
        Serial.print(F("Dew point: "));
        Serial.print(outputs.dewPointC, 1);
        Serial.print(F(" C, dew heater: "));
        Serial.print(dewHeaterPower * 100.0f, 0);
        Serial.print(F("%, cooler: "));
        Serial.print(coolerPower * 100.0f, 0);
        Serial.print(F("%, fan: "));
        Serial.print(fanPower * 100.0f, 0);
        Serial.println('%');
    }
}
