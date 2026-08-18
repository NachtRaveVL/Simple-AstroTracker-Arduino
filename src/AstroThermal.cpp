/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Thermal Balancing
*/

#include "AstroThermal.h"
#include <math.h>

static float clampPower(double value)
{
    if (value <= 0.0) { return 0.0f; }
    if (value >= 1.0) { return 1.0f; }
    return (float)value;
}

AstroThermalReadings::AstroThermalReadings()
    : ambientTemperatureC(10.0), humidityPercent(50.0), opticsTemperatureC(10.0),
      cameraSensorTemperatureC(10.0), cameraBodyTemperatureC(10.0)
{ ; }

AstroThermalOutputs::AstroThermalOutputs()
    : dewHeaterPower(0.0f), electronicsHeaterPower(0.0f), cameraCoolingPower(0.0f), cameraFanPower(0.0f),
      dewPointC(0.0), opticsTargetC(0.0), cameraTargetC(0.0)
{ ; }

AstroThermalBalancer::AstroThermalBalancer()
    : _mode(Astro_ThermalMode_DayStorage), _dewMarginC(ASTRO_THERMAL_DEW_MARGIN_C), _maxOpticsAboveAmbientC(ASTRO_THERMAL_OPTICS_MAX_C),
      _cameraRequestedTargetC(ASTRO_THERMAL_CAMERA_TARGET_C), _cameraCommandTargetC(10.0), _cameraCoolingRampCPerMinute(ASTRO_THERMAL_CAMERA_RAMP_CPM),
      _electronicsMinimumC(ASTRO_THERMAL_ELECTRONICS_MIN_C), _dewHeater(nullptr), _electronicsHeater(nullptr), _cameraCooler(nullptr), _cameraFan(nullptr)
{ ; }

void AstroThermalBalancer::setMode(Astro_ThermalMode mode) { _mode = mode; }
void AstroThermalBalancer::setDewMargin(double dewMarginC) { _dewMarginC = dewMarginC; }
void AstroThermalBalancer::setMaxOpticsAboveAmbient(double degreesC) { _maxOpticsAboveAmbientC = degreesC; }
void AstroThermalBalancer::setCameraTarget(double targetC) { _cameraRequestedTargetC = targetC; }
void AstroThermalBalancer::setCameraCoolingRamp(double degreesPerMinute) { _cameraCoolingRampCPerMinute = degreesPerMinute > 0.0 ? degreesPerMinute : 0.0; }
void AstroThermalBalancer::setElectronicsMinimum(double minimumC) { _electronicsMinimumC = minimumC; }
void AstroThermalBalancer::setDewHeater(AstroActuator *actuator) { _dewHeater = actuator; }
void AstroThermalBalancer::setElectronicsHeater(AstroActuator *actuator) { _electronicsHeater = actuator; }
void AstroThermalBalancer::setCameraCooler(AstroActuator *actuator) { _cameraCooler = actuator; }
void AstroThermalBalancer::setCameraFan(AstroActuator *actuator) { _cameraFan = actuator; }

double AstroThermalBalancer::calculateDewPoint(double temperatureC, double humidityPercent)
{
    double humidity = humidityPercent < 1.0 ? 1.0 : (humidityPercent > 100.0 ? 100.0 : humidityPercent);
    const double a = 17.62;
    const double b = 243.12;
    double gamma = log(humidity / 100.0) + (a * temperatureC) / (b + temperatureC);
    return (b * gamma) / (a - gamma);
}

AstroThermalOutputs AstroThermalBalancer::update(const AstroThermalReadings &readings, double elapsedSeconds)
{
    AstroThermalOutputs output;
    output.dewPointC = calculateDewPoint(readings.ambientTemperatureC, readings.humidityPercent);
    output.opticsTargetC = output.dewPointC + _dewMarginC;
    double opticsMax = readings.ambientTemperatureC + _maxOpticsAboveAmbientC;
    if (output.opticsTargetC > opticsMax) { output.opticsTargetC = opticsMax; }

    double rampStep = _cameraCoolingRampCPerMinute * elapsedSeconds / 60.0;
    double desiredCamera = _mode == Astro_ThermalMode_NightObserving ? _cameraRequestedTargetC : readings.ambientTemperatureC;
    if (_cameraCommandTargetC < desiredCamera) {
        _cameraCommandTargetC = _cameraCommandTargetC + rampStep > desiredCamera ? desiredCamera : _cameraCommandTargetC + rampStep;
    } else if (_cameraCommandTargetC > desiredCamera) {
        _cameraCommandTargetC = _cameraCommandTargetC - rampStep < desiredCamera ? desiredCamera : _cameraCommandTargetC - rampStep;
    }
    output.cameraTargetC = _cameraCommandTargetC;

    output.dewHeaterPower = clampPower((output.opticsTargetC - readings.opticsTemperatureC) / 5.0);
    output.electronicsHeaterPower = clampPower((_electronicsMinimumC - readings.cameraBodyTemperatureC) / 8.0);

    if (_mode == Astro_ThermalMode_NightObserving) {
        output.cameraCoolingPower = clampPower((readings.cameraSensorTemperatureC - output.cameraTargetC) / 20.0);
        output.cameraFanPower = clampPower(output.cameraCoolingPower * 0.7 + (readings.cameraBodyTemperatureC - readings.ambientTemperatureC) / 20.0);
    } else {
        output.cameraCoolingPower = 0.0f;
        output.cameraFanPower = clampPower((readings.cameraBodyTemperatureC - readings.ambientTemperatureC) / 10.0);
    }

    if (_dewHeater) { _dewHeater->setPower(output.dewHeaterPower); }
    if (_electronicsHeater) { _electronicsHeater->setPower(output.electronicsHeaterPower); }
    if (_cameraCooler) { _cameraCooler->setPower(output.cameraCoolingPower); }
    if (_cameraFan) { _cameraFan->setPower(output.cameraFanPower); }

    return output;
}

bool AstroThermalBalancer::cameraStable(const AstroThermalReadings &readings, double toleranceC) const
{
    return fabs(readings.cameraSensorTemperatureC - _cameraCommandTargetC) <= toleranceC;
}

bool AstroThermalBalancer::cameraSafeToStow(const AstroThermalReadings &readings) const
{
    return readings.cameraSensorTemperatureC >= readings.ambientTemperatureC - 3.0;
}
