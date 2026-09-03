// Simple-AstroTracker-Arduino Equatorial Tracking Example
//
// A very small equatorial setup using two common STEP/DIR stepper motor drivers.
// Adjust the pin assignments, steps-per-degree values, and maximum step rates to match
// the motors, microstepping configuration, and mechanical reduction in the tracker.

#include <Astruino.h>

// Observer Settings
#define SETUP_LATITUDE                  49.2827             // Latitude, degrees (+N / -S)
#define SETUP_LONGITUDE                -123.1207            // Longitude, degrees (+E / -W)
#define SETUP_ELEVATION                70.0                 // Elevation, meters
#define SETUP_START_UNIX               1787101200LL         // Replace with RTC/GPS/NTP time in a real setup

// Mount Hardware
#define SETUP_RA_STEP_PIN              2                    // Right-ascension STEP pin
#define SETUP_RA_DIR_PIN               3                    // Right-ascension DIR pin
#define SETUP_RA_ENABLE_PIN            4                    // Right-ascension ENABLE pin, else -1
#define SETUP_DEC_STEP_PIN             5                    // Declination STEP pin
#define SETUP_DEC_DIR_PIN              6                    // Declination DIR pin
#define SETUP_DEC_ENABLE_PIN           7                    // Declination ENABLE pin, else -1
#define SETUP_RA_STEPS_PER_DEG         1280.0               // Motor steps per axis degree after microstepping/gearing
#define SETUP_DEC_STEPS_PER_DEG        1280.0               // Motor steps per axis degree after microstepping/gearing
#define SETUP_RA_MAX_STEPS_SEC         4000.0               // Maximum right-ascension step rate
#define SETUP_DEC_MAX_STEPS_SEC        4000.0               // Maximum declination step rate

Astruino astroController(Astro_MountType_Equatorial);

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }

    AstroObserver observer(SETUP_LATITUDE, SETUP_LONGITUDE, SETUP_ELEVATION);
    astroController.init();
    astroController.setObserver(observer);
    setTime((time_t)SETUP_START_UNIX);

    auto primaryDriver = astroController.addMountAxisStepper(SETUP_RA_STEP_PIN, SETUP_RA_DIR_PIN,
                                                              SETUP_RA_ENABLE_PIN, SETUP_RA_STEPS_PER_DEG,
                                                              SETUP_RA_MAX_STEPS_SEC);
    auto secondaryDriver = astroController.addMountAxisStepper(SETUP_DEC_STEP_PIN, SETUP_DEC_DIR_PIN,
                                                                SETUP_DEC_ENABLE_PIN, SETUP_DEC_STEPS_PER_DEG,
                                                                SETUP_DEC_MAX_STEPS_SEC);

    auto &mount = astroController.getMount();
    mount.setAxisDriver(0, primaryDriver);
    mount.setAxisDriver(1, secondaryDriver);
    mount.setTarget(Astro_Target_M42);
    mount.setStowPosition(0.0, 0.0);
    mount.unpark();
    mount.track();

    astroController.launch();
    Serial.println(F("Astruino equatorial tracker started"));
}

void loop()
{
    astroController.update();

    static millis_t lastReport = 0;
    millis_t now = millis();
    if (now - lastReport >= 5000) {
        lastReport = now;

        auto &mount = astroController.getMount();
        Serial.print(F("RA axis: "));
        Serial.print(mount.getPrimaryAxis().positionDegrees, 4);
        Serial.print(F(" / "));
        Serial.print(mount.getPrimaryAxis().targetDegrees, 4);
        Serial.print(F(" deg, DEC axis: "));
        Serial.print(mount.getSecondaryAxis().positionDegrees, 4);
        Serial.print(F(" / "));
        Serial.print(mount.getSecondaryAxis().targetDegrees, 4);
        Serial.println(F(" deg"));
    }
}
