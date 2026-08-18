// Simple-AstroTracker-Arduino Equatorial Tracking Example
//
// A very small equatorial setup using two user supplied axis drivers. The callbacks here
// only show where the actual stepper/servo calls would be made. This lets the example
// work with whatever motor hardware and gear reduction the tracker was built around.

#include <Astruino.h>

// Observer Settings
#define SETUP_LATITUDE                  49.2827             // Latitude, degrees (+N / -S)
#define SETUP_LONGITUDE                -123.1207            // Longitude, degrees (+E / -W)
#define SETUP_ELEVATION                70.0                 // Elevation, meters
#define SETUP_START_UNIX               1787101200LL         // Replace with RTC/GPS/NTP time in a real setup

Astruino astroController(Astro_MountType_Equatorial);

static double axisTargets[2] = {0.0, 0.0};
static millis_t lastUpdate = 0;

void setAxisTarget(void *context, uint8_t axisIndex, double targetDegrees)
{
    (void)context;
    if (axisIndex >= 2) { return; }
    axisTargets[axisIndex] = targetDegrees;

    // Send targetDegrees to the stepper/servo controller here.
    // The mount does not care what kind of motor is attached to each axis.
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }

    AstroObserver observer(SETUP_LATITUDE, SETUP_LONGITUDE, SETUP_ELEVATION);
    astroController.init();
    astroController.setObserver(observer);
    astroController.launch();

    auto &mount = astroController.getMount();
    mount.setTarget(Astro_Target_M42);
    mount.setAxisRates(6.0, 6.0);
    mount.setStowPosition(0.0, 0.0);
    mount.setAxisTargetCallback(setAxisTarget);
    mount.track();

    lastUpdate = millis();
    Serial.println(F("Astruino equatorial tracker started"));
}

void loop()
{
    millis_t now = millis();
    double elapsedSeconds = (now - lastUpdate) / 1000.0;
    lastUpdate = now;

    int64_t unixTime = SETUP_START_UNIX + (now / 1000);
    astroController.getMount().update(unixTime, elapsedSeconds);

    static millis_t lastReport = 0;
    if (now - lastReport >= 5000) {
        lastReport = now;

        Serial.print(F("RA axis target: "));
        Serial.print(axisTargets[0], 4);
        Serial.print(F(" deg, DEC axis target: "));
        Serial.print(axisTargets[1], 4);
        Serial.println(F(" deg"));
    }
}
