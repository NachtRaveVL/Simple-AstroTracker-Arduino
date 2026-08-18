/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Controller
*/

#include "Astruino.h"

Astruino *Astruino::_activeInstance = nullptr;

Astruino *getController()
{
    return Astruino::getActiveInstance();
}

AstroLogger *getLogger()
{
    return getController() ? &getController()->logger : nullptr;
}

AstroPublisher *getPublisher()
{
    return getController() ? &getController()->publisher : nullptr;
}

AstroScheduler *getScheduler()
{
    return getController() ? &getController()->scheduler : nullptr;
}


Astruino::Astruino(Astro_MountType mountType)
    : scheduler(), logger(), publisher(), _systemData(), _mount(mountType), _cover(),
      _camera(), _thermal(), _timeProvider(nullptr), _thermalReadings(), _safeToObserve(true),
      _lastUpdate(0), _initialized(false), _suspended(true)
{
    if (!_activeInstance) { _activeInstance = this; }

    scheduler.setMount(&_mount);
    scheduler.setCover(&_cover);
    scheduler.setObservationDevice(&_camera);
    scheduler.setThermalBalancer(&_thermal);
    scheduler.setLogger(&logger);
}

Astruino::~Astruino()
{
    if (_activeInstance == this) { _activeInstance = nullptr; }
}

void Astruino::init(Astro_SystemMode systemMode, Astro_MeasurementMode measurementMode)
{
    _systemData.systemMode = systemMode;
    _systemData.measurementMode = measurementMode;
    applySystemData();
    _initialized = true;
    _suspended = true;
}

void Astruino::setObserver(const AstroObserver &observer)
{
    _systemData.observer = observer;
    _mount.setObserver(observer);
}

void Astruino::applySystemData()
{
    _mount.setObserver(_systemData.observer);
    scheduler.setConfig(_systemData.scheduler);
    logger.setSubData(&_systemData.logger);
    publisher.setSubData(&_systemData.publisher);
}

void Astruino::launch()
{
    if (!_initialized) { init(); }
    applySystemData();
    _suspended = false;
}

void Astruino::suspend()
{
    _suspended = true;
    _camera.stopObservation();
    _mount.stow();
}


void Astruino::update()
{
    if (!_initialized || _suspended) { return; }

    int64_t unixTime = 0;
    if (_timeProvider) {
        if (!_timeProvider->getUnixTime(&unixTime)) { return; }
    } else {
        unixTime = (int64_t)time(nullptr);
        if (unixTime <= 0) { return; }
    }

    millis_t now = astroNZMillis();
    double elapsedSeconds = _lastUpdate ? (double)(now - _lastUpdate) / 1000.0 : 0.0;
    _lastUpdate = now;

    AstroEquatorialCoordinates sunCoordinates;
    double sunAltitudeDegrees = 90.0;
    if (astroResolveSolarSystemTarget(Astro_Target_Sun, unixTime, &sunCoordinates)) {
        sunAltitudeDegrees = astroEquatorialToHorizontal(sunCoordinates, _systemData.observer, unixTime).altitudeDegrees;
    }

    update(unixTime, elapsedSeconds, sunAltitudeDegrees, _safeToObserve, _thermalReadings);
}

void Astruino::update(int64_t unixTime, double elapsedSeconds, double sunAltitudeDegrees,
                      bool safeToObserve, const AstroThermalReadings &thermalReadings)
{
    if (!_initialized || _suspended) { return; }
    scheduler.update(unixTime, elapsedSeconds, sunAltitudeDegrees, safeToObserve, thermalReadings);
}
