/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Thermal Balancing
*/

#ifndef AstroThermal_H
#define AstroThermal_H

#include "AstroDefines.h"
#include "AstroActuators.h"

// Thermal Readings
// Current environmental and equipment temperatures used by thermal balancing.
struct AstroThermalReadings {
    double ambientTemperatureC;                              // Ambient temperature c
    double humidityPercent;                                  // Humidity percent
    double opticsTemperatureC;                               // Optics temperature c
    double cameraSensorTemperatureC;                         // Camera sensor temperature c
    double cameraBodyTemperatureC;                           // Camera body temperature c

    AstroThermalReadings();
};

// Thermal Outputs
// Normalized heater, cooler, and fan demands calculated by the thermal balancer.
struct AstroThermalOutputs {
    float dewHeaterPower;                                    // Dew heater power
    float electronicsHeaterPower;                            // Electronics heater power
    float cameraCoolingPower;                                // Camera cooling power
    float cameraFanPower;                                    // Camera fan power
    double dewPointC;                                        // Dew point c
    double opticsTargetC;                                    // Optics target c
    double cameraTargetC;                                    // Camera target c

    AstroThermalOutputs();
};

// Thermal Balancer
// Balances dew protection, equipment heating, and optional camera cooling from sensor readings.
class AstroThermalBalancer {
public:
    AstroThermalBalancer();

    void setMode(Astro_ThermalMode mode);
    void setDewMargin(double dewMarginC);
    void setMaxOpticsAboveAmbient(double degreesC);
    void setCameraTarget(double targetC);
    void setCameraCoolingRamp(double degreesPerMinute);
    void setElectronicsMinimum(double minimumC);
    void setDewHeater(AstroActuator *actuator);
    void setElectronicsHeater(AstroActuator *actuator);
    void setCameraCooler(AstroActuator *actuator);
    void setCameraFan(AstroActuator *actuator);

    AstroThermalOutputs update(const AstroThermalReadings &readings, double elapsedSeconds);
    bool cameraStable(const AstroThermalReadings &readings, double toleranceC = 1.0) const;
    bool cameraSafeToStow(const AstroThermalReadings &readings) const;

    static double calculateDewPoint(double temperatureC, double humidityPercent);

protected:
    Astro_ThermalMode _mode;                                 // Mode
    double _dewMarginC;                                      // Dew margin c
    double _maxOpticsAboveAmbientC;                          // Max optics above ambient c
    double _cameraRequestedTargetC;                          // Camera requested target c
    double _cameraCommandTargetC;                            // Camera command target c
    double _cameraCoolingRampCPerMinute;                     // Camera cooling ramp cper minute
    double _electronicsMinimumC;                             // Electronics minimum c
    AstroActuator *_dewHeater;                               // Dew heater, not owned
    AstroActuator *_electronicsHeater;                       // Electronics heater, not owned
    AstroActuator *_cameraCooler;                            // Camera cooler, not owned
    AstroActuator *_cameraFan;                               // Camera fan, not owned
};

#endif // /ifndef AstroThermal_H
