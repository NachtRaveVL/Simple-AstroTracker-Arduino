/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Thermal Balancing
*/

#include "Astruino.h"
#include <math.h>

AstroThermalReadings::AstroThermalReadings()
    : ambientTemperatureC(10.0), humidityPercent(50.0), opticsTemperatureC(10.0),
      cameraSensorTemperatureC(10.0), cameraBodyTemperatureC(10.0)
{ ; }

AstroThermalOutputs::AstroThermalOutputs()
    : dewHeaterPower(0.0f), electronicsHeaterPower(0.0f), cameraCoolingPower(0.0f), cameraFanPower(0.0f),
      dewPointC(0.0), opticsTargetC(0.0), cameraTargetC(0.0)
{ ; }

AstroThermalBalancer::AstroThermalBalancer(AstroObjInterface *parent)
    : AstroSubObject(parent), _mode(Astro_ThermalMode_DayStorage), _dewMarginC(ASTRO_THERMAL_DEW_MARGIN_C),
      _maxOpticsAboveAmbientC(ASTRO_THERMAL_OPTICS_MAX_C), _cameraRequestedTargetC(ASTRO_THERMAL_CAMERA_TARGET_C),
      _cameraCommandTargetC(10.0), _cameraCoolingRampCPerMinute(ASTRO_THERMAL_CAMERA_RAMP_CPM),
      _electronicsMinimumC(ASTRO_THERMAL_ELECTRONICS_MIN_C), _lastUpdate(0),
      _ambientTemperature(this), _humidity(this), _opticsTemperature(this),
      _cameraSensorTemperature(this), _cameraBodyTemperature(this),
      _dewHeater(this), _electronicsHeater(this), _cameraCooler(this), _cameraFan(this),
      _readings(), _outputs()
{ ; }

void AstroThermalBalancer::setMode(Astro_ThermalMode mode)
{
    if (_mode != mode) {
        _mode = mode;
        bumpRevisionIfNeeded();
    }
}
void AstroThermalBalancer::setDewMargin(double dewMarginC)
{
    if (!isFPEqual(_dewMarginC, dewMarginC)) {
        _dewMarginC = dewMarginC;
        bumpRevisionIfNeeded();
    }
}
void AstroThermalBalancer::setMaxOpticsAboveAmbient(double degreesC)
{
    if (!isFPEqual(_maxOpticsAboveAmbientC, degreesC)) {
        _maxOpticsAboveAmbientC = degreesC;
        bumpRevisionIfNeeded();
    }
}
void AstroThermalBalancer::setCameraTarget(double targetC)
{
    if (!isFPEqual(_cameraRequestedTargetC, targetC)) {
        _cameraRequestedTargetC = targetC;
        bumpRevisionIfNeeded();
    }
}
void AstroThermalBalancer::setCameraCoolingRamp(double degreesPerMinute)
{
    double next = degreesPerMinute > 0.0 ? degreesPerMinute : 0.0;
    if (!isFPEqual(_cameraCoolingRampCPerMinute, next)) {
        _cameraCoolingRampCPerMinute = next;
        bumpRevisionIfNeeded();
    }
}
void AstroThermalBalancer::setElectronicsMinimum(double minimumC)
{
    if (!isFPEqual(_electronicsMinimumC, minimumC)) {
        _electronicsMinimumC = minimumC;
        bumpRevisionIfNeeded();
    }
}

float AstroThermalBalancer::clampPower(double value)
{
    if (value <= 0.0) { return 0.0f; }
    if (value >= 1.0) { return 1.0f; }
    return (float)value;
}

double AstroThermalBalancer::calculateDewPoint(double temperatureC, double humidityPercent)
{
    double humidity = humidityPercent < 1.0 ? 1.0 : (humidityPercent > 100.0 ? 100.0 : humidityPercent);
    const double a = 17.62;
    const double b = 243.12;
    double gamma = log(humidity / 100.0) + (a * temperatureC) / (b + temperatureC);
    return (b * gamma) / (a - gamma);
}

static bool astroReadAttached(AstroSensorAttachment &attachment, Astro_UnitsType units, double *valueOut)
{
    if (!valueOut || !attachment.isSet()) { return false; }
    attachment.setMeasurementUnits(units);
    attachment.updateIfNeeded(true);
    const AstroSingleMeasurement &measurement = attachment.getMeasurement();
    if (!measurement.isSet()) { return false; }
    *valueOut = measurement.value;
    return true;
}

bool AstroThermalBalancer::updateReadings()
{
    bool any = false;
    any = astroReadAttached(_ambientTemperature, Astro_UnitsType_Temperature_Celsius, &_readings.ambientTemperatureC) || any;
    any = astroReadAttached(_humidity, Astro_UnitsType_Humidity_RH, &_readings.humidityPercent) || any;
    any = astroReadAttached(_opticsTemperature, Astro_UnitsType_Temperature_Celsius, &_readings.opticsTemperatureC) || any;
    any = astroReadAttached(_cameraSensorTemperature, Astro_UnitsType_Temperature_Celsius, &_readings.cameraSensorTemperatureC) || any;
    any = astroReadAttached(_cameraBodyTemperature, Astro_UnitsType_Temperature_Celsius, &_readings.cameraBodyTemperatureC) || any;
    return any;
}

void AstroThermalBalancer::applyOutput(AstroActuatorAttachment &attachment, float power)
{
    if (!attachment.isSet()) { return; }
    attachment.setupActivation(power);
    if (fabsf(power) > ASTRO_FLT_EPSILON) { attachment.enableActivation(); }
    else { attachment.disableActivation(); }
}

void AstroThermalBalancer::update()
{
    millis_t now = astroNZMillis();
    bool firstUpdate = !_lastUpdate;
    double elapsedSeconds = firstUpdate ? 0.0 : (double)(now - _lastUpdate) / 1000.0;
    _lastUpdate = now;

    if (!updateReadings()) {
        applyOutput(_dewHeater, 0.0f);
        applyOutput(_electronicsHeater, 0.0f);
        applyOutput(_cameraCooler, 0.0f);
        applyOutput(_cameraFan, 0.0f);
        return;
    }

    bool haveAmbient = _ambientTemperature.getMeasurement().isSet();
    bool haveHumidity = _humidity.getMeasurement().isSet();
    bool haveOptics = _opticsTemperature.getMeasurement().isSet();
    bool haveCameraSensor = _cameraSensorTemperature.getMeasurement().isSet();
    bool haveCameraBody = _cameraBodyTemperature.getMeasurement().isSet();

    if (haveAmbient && haveHumidity && haveOptics) {
        _outputs.dewPointC = calculateDewPoint(_readings.ambientTemperatureC, _readings.humidityPercent);
        _outputs.opticsTargetC = _outputs.dewPointC + _dewMarginC;
        double opticsMax = _readings.ambientTemperatureC + _maxOpticsAboveAmbientC;
        if (_outputs.opticsTargetC > opticsMax) { _outputs.opticsTargetC = opticsMax; }
        _outputs.dewHeaterPower = clampPower((_outputs.opticsTargetC - _readings.opticsTemperatureC) / 5.0);
    } else {
        _outputs.dewHeaterPower = 0.0f;
    }

    _outputs.electronicsHeaterPower = haveCameraBody
        ? clampPower((_electronicsMinimumC - _readings.cameraBodyTemperatureC) / 8.0) : 0.0f;

    if (haveAmbient && haveCameraSensor) {
        if (firstUpdate) { _cameraCommandTargetC = _readings.ambientTemperatureC; }
        double desiredCamera = _mode == Astro_ThermalMode_NightObserving ? _cameraRequestedTargetC : _readings.ambientTemperatureC;
        double rampStep = _cameraCoolingRampCPerMinute * elapsedSeconds / 60.0;
        if (_cameraCommandTargetC < desiredCamera) {
            _cameraCommandTargetC = _cameraCommandTargetC + rampStep > desiredCamera ? desiredCamera : _cameraCommandTargetC + rampStep;
        } else if (_cameraCommandTargetC > desiredCamera) {
            _cameraCommandTargetC = _cameraCommandTargetC - rampStep < desiredCamera ? desiredCamera : _cameraCommandTargetC - rampStep;
        }
        _outputs.cameraTargetC = _cameraCommandTargetC;
        _outputs.cameraCoolingPower = _mode == Astro_ThermalMode_NightObserving
            ? clampPower((_readings.cameraSensorTemperatureC - _outputs.cameraTargetC) / 20.0) : 0.0f;
    } else {
        _outputs.cameraCoolingPower = 0.0f;
    }

    if (haveAmbient && haveCameraBody) {
        _outputs.cameraFanPower = _mode == Astro_ThermalMode_NightObserving
            ? clampPower(_outputs.cameraCoolingPower * 0.7 +
                         (_readings.cameraBodyTemperatureC - _readings.ambientTemperatureC) / 20.0)
            : clampPower((_readings.cameraBodyTemperatureC - _readings.ambientTemperatureC) / 10.0);
    } else {
        _outputs.cameraFanPower = 0.0f;
    }

    applyOutput(_dewHeater, _outputs.dewHeaterPower);
    applyOutput(_electronicsHeater, _outputs.electronicsHeaterPower);
    applyOutput(_cameraCooler, _outputs.cameraCoolingPower);
    applyOutput(_cameraFan, _outputs.cameraFanPower);
}

bool AstroThermalBalancer::cameraStable(double toleranceC) const
{
    return !_cameraSensorTemperature.isSet() ||
           fabs(_readings.cameraSensorTemperatureC - _cameraCommandTargetC) <= toleranceC;
}

bool AstroThermalBalancer::cameraSafeToStow() const
{
    return !_cameraSensorTemperature.isSet() || !_ambientTemperature.isSet() ||
           _readings.cameraSensorTemperatureC >= _readings.ambientTemperatureC - 3.0;
}

void AstroThermalBalancer::unresolveAny(AstroObject *object)
{
    _ambientTemperature.unresolveAny(object);
    _humidity.unresolveAny(object);
    _opticsTemperature.unresolveAny(object);
    _cameraSensorTemperature.unresolveAny(object);
    _cameraBodyTemperature.unresolveAny(object);
    _dewHeater.unresolveAny(object);
    _electronicsHeater.unresolveAny(object);
    _cameraCooler.unresolveAny(object);
    _cameraFan.unresolveAny(object);
}
