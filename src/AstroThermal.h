/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Thermal Balancing
*/

#ifndef AstroThermal_H
#define AstroThermal_H

struct AstroThermalReadings;
struct AstroThermalOutputs;
class AstroThermalBalancer;

#include "Astruino.h"

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
// Balances dew protection, equipment heating, and optional camera cooling from attached
// sensors. Actuator requests remain resident through attachment activation handles.
class AstroThermalBalancer : public AstroSubObject {
public:
    AstroThermalBalancer(AstroObjInterface *parent = nullptr);

    void setMode(Astro_ThermalMode mode);
    void setDewMargin(double dewMarginC);
    void setMaxOpticsAboveAmbient(double degreesC);
    void setCameraTarget(double targetC);
    void setCameraCoolingRamp(double degreesPerMinute);
    void setElectronicsMinimum(double minimumC);

    template<class U> inline void setAmbientTemperatureSensor(U sensor) { _ambientTemperature.setObject(sensor); }
    template<class U> inline void setHumiditySensor(U sensor) { _humidity.setObject(sensor); }
    template<class U> inline void setOpticsTemperatureSensor(U sensor) { _opticsTemperature.setObject(sensor); }
    template<class U> inline void setCameraSensorTemperatureSensor(U sensor) { _cameraSensorTemperature.setObject(sensor); }
    template<class U> inline void setCameraBodyTemperatureSensor(U sensor) { _cameraBodyTemperature.setObject(sensor); }

    template<class U> inline void setDewHeater(U actuator) { _dewHeater.setObject(actuator); }
    template<class U> inline void setElectronicsHeater(U actuator) { _electronicsHeater.setObject(actuator); }
    template<class U> inline void setCameraCooler(U actuator) { _cameraCooler.setObject(actuator); }
    template<class U> inline void setCameraFan(U actuator) { _cameraFan.setObject(actuator); }

    void update();
    virtual void unresolveAny(AstroObject *object) override;
    bool cameraStable(double toleranceC = 1.0) const;
    bool cameraSafeToStow() const;

    inline const AstroThermalReadings &getReadings() const { return _readings; }
    inline const AstroThermalOutputs &getOutputs() const { return _outputs; }

    inline AstroSensorAttachment &getAmbientTemperatureSensorAttachment() { return _ambientTemperature; }
    inline AstroSensorAttachment &getHumiditySensorAttachment() { return _humidity; }
    inline AstroSensorAttachment &getOpticsTemperatureSensorAttachment() { return _opticsTemperature; }
    inline AstroSensorAttachment &getCameraSensorTemperatureAttachment() { return _cameraSensorTemperature; }
    inline AstroSensorAttachment &getCameraBodyTemperatureAttachment() { return _cameraBodyTemperature; }

    static double calculateDewPoint(double temperatureC, double humidityPercent);

protected:
    Astro_ThermalMode _mode;                                 // Mode
    double _dewMarginC;                                      // Dew margin c
    double _maxOpticsAboveAmbientC;                          // Max optics above ambient c
    double _cameraRequestedTargetC;                          // Camera requested target c
    double _cameraCommandTargetC;                            // Camera command target c
    double _cameraCoolingRampCPerMinute;                     // Camera cooling ramp cper minute
    double _electronicsMinimumC;                             // Electronics minimum c
    millis_t _lastUpdate;                                    // Last update time, in milliseconds

    AstroSensorAttachment _ambientTemperature;               // Ambient temperature sensor attachment
    AstroSensorAttachment _humidity;                         // Humidity sensor attachment
    AstroSensorAttachment _opticsTemperature;                // Optics temperature sensor attachment
    AstroSensorAttachment _cameraSensorTemperature;          // Camera sensor temperature attachment
    AstroSensorAttachment _cameraBodyTemperature;            // Camera body temperature attachment

    AstroActuatorAttachment _dewHeater;                      // Dew heater actuator attachment
    AstroActuatorAttachment _electronicsHeater;              // Electronics heater actuator attachment
    AstroActuatorAttachment _cameraCooler;                   // Camera cooler actuator attachment
    AstroActuatorAttachment _cameraFan;                      // Camera fan actuator attachment

    AstroThermalReadings _readings;                          // Latest attached readings
    AstroThermalOutputs _outputs;                            // Latest calculated outputs

    bool updateReadings();
    static float clampPower(double value);
    static void applyOutput(AstroActuatorAttachment &attachment, float power);
};

#endif // /ifndef AstroThermal_H
