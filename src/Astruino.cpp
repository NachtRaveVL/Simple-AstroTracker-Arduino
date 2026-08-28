/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Controller
*/

#include "Astruino.h"

Astruino *Astruino::_activeInstance = nullptr;

#ifdef ARDUINO
static AstroRTCInterface *_rtcSyncProvider = nullptr;
static time_t rtcNow()
{
    return _rtcSyncProvider ? _rtcSyncProvider->now().unixtime() : 0;
}
#endif

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


AstroCover::AstroCover()
    : AstroSubObject(), _position(0.0f), _target(0.0f), _travelRate(ASTRO_COVER_TRAVEL_RATE),
      _travelTimeout(ASTRO_COVER_TRAVEL_TIMEOUT_SECS), _travelElapsed(0.0),
      _openLimitActive(false), _closedLimitActive(false), _faulted(false),
      _actuator(this), _openSensor(this), _closedSensor(this), _lastUpdate(nzMillis())
{ ; }

void AstroCover::open()
{
    if (_faulted) { return; }
    if (!isFPEqual(_target, 1.0f)) { _travelElapsed = 0.0; }
    _target = 1.0f;
}

void AstroCover::close()
{
    if (_faulted) { return; }
    if (!isFPEqual(_target, 0.0f)) { _travelElapsed = 0.0; }
    _target = 0.0f;
}

void AstroCover::stop()
{
    _target = _position;
    _travelElapsed = 0.0;
    applyActuatorPower(0.0f);
}

void AstroCover::setTravelRate(float fractionPerSecond)
{
    _travelRate = fractionPerSecond > 0.0f ? fractionPerSecond : 0.0f;
}

void AstroCover::setTravelTimeout(double seconds)
{
    _travelTimeout = seconds > 0.0 ? seconds : 0.0;
}

void AstroCover::setPosition(float position)
{
    _position = constrain(position, 0.0f, 1.0f);
    _target = _position;
    _travelElapsed = 0.0;
}

void AstroCover::clearFault()
{
    _faulted = false;
    _travelElapsed = 0.0;
    _lastUpdate = nzMillis();
}

bool AstroCover::isOpen() const
{
    if (_openSensor.isSet()) { return _openLimitActive; }
    return _position >= 1.0f - FLT_EPSILON;
}

bool AstroCover::isClosed() const
{
    if (_closedSensor.isSet()) { return _closedLimitActive; }
    return _position <= FLT_EPSILON;
}

bool AstroCover::isMoving() const
{
    return !_faulted && !isFPEqual(_target, _position);
}

bool AstroCover::pollLimitSensor(AstroSensorAttachment &sensor, bool *activeOut)
{
    if (!activeOut || !sensor.isSet()) { return false; }
    sensor.updateIfNeeded(true);
    const AstroSingleMeasurement &measurement = sensor.getMeasurement();
    if (!measurement.isSet()) { return false; }
    *activeOut = measurement.value > 0.5;
    return true;
}

void AstroCover::applyActuatorPower(float power)
{
    if (!_actuator.isSet()) { return; }
    if (isFPEqual(power, 0.0f)) {
        _actuator.disableActivation();
        return;
    }
    _actuator.setupActivation(power, (millis_t)-1, false);
    _actuator.enableActivation();
}

void AstroCover::update()
{
    const millis_t now = nzMillis();
    const double elapsedSeconds = _lastUpdate ? (double)(now - _lastUpdate) / 1000.0 : 0.0;
    _lastUpdate = now;

    bool openLimit = false;
    bool closedLimit = false;
    const bool hasOpenLimit = pollLimitSensor(_openSensor, &openLimit);
    const bool hasClosedLimit = pollLimitSensor(_closedSensor, &closedLimit);
    if (hasOpenLimit) { _openLimitActive = openLimit; }
    if (hasClosedLimit) { _closedLimitActive = closedLimit; }

    if (_openLimitActive && _closedLimitActive) {
        _faulted = true;
        applyActuatorPower(0.0f);
        return;
    }
    if (_openLimitActive) { _position = 1.0f; }
    else if (_closedLimitActive) { _position = 0.0f; }

    if (_faulted || isFPEqual(_target, _position)) {
        _travelElapsed = 0.0;
        applyActuatorPower(0.0f);
        return;
    }

    const float direction = _target > _position ? 1.0f : -1.0f;
    if ((direction > 0.0f && _openLimitActive) || (direction < 0.0f && _closedLimitActive)) {
        _position = direction > 0.0f ? 1.0f : 0.0f;
        _target = _position;
        _travelElapsed = 0.0;
        applyActuatorPower(0.0f);
        return;
    }

    _travelElapsed += elapsedSeconds;
    if (_travelTimeout > 0.0 && _travelElapsed >= _travelTimeout) {
        _faulted = true;
        applyActuatorPower(0.0f);
        return;
    }

    applyActuatorPower(direction);
    if (_travelRate <= 0.0f || elapsedSeconds <= 0.0) { return; }

    const float step = _travelRate * elapsedSeconds;
    if (direction > 0.0f) { _position = min(_target, _position + step); }
    else { _position = max(_target, _position - step); }

    if (isFPEqual(_target, _position)) {
        _position = _target;
        _travelElapsed = 0.0;
        applyActuatorPower(0.0f);
    }
}

void AstroCover::unresolveAny(AstroObject *object)
{
    _actuator.unresolveAny(object);
    _openSensor.unresolveAny(object);
    _closedSensor.unresolveAny(object);
    AstroSubObject::unresolveAny(object);
}


Astruino::Astruino(Astro_MountType mountType, Astro_RTCType rtcType, AstroDeviceSetup rtcSetup)
    : scheduler(), logger(), publisher(), _systemData(),
      _mount(new AstroMount(mountType, 0)), _cover(), _camera(new AstroCameraTrigger(nullptr, nullptr, 0)),
      _thermal(), _rtcType(rtcType), _rtcSetup(rtcSetup)
#ifdef ARDUINO
      , _rtc(nullptr)
#endif
      , _rtcBegan(false), _rtcBattFail(false), _initialized(false), _suspended(true)
{
    if (!_activeInstance) { _activeInstance = this; }

    registerObject(_mount);
    registerObject(_camera);

    scheduler.setMount(_mount);
    scheduler.setCover(&_cover);
    scheduler.setObservationDevice(_camera);
    scheduler.setThermalBalancer(&_thermal);
    scheduler.setLogger(&logger);
}

Astruino::~Astruino()
{
    suspend();
    while (_objects.size()) { _objects.erase(_objects.begin()); }
    deallocateRTC();
    if (_activeInstance == this) { _activeInstance = nullptr; }
}

void Astruino::init(Astro_SystemMode systemMode, Astro_MeasurementMode measurementMode)
{
    _systemData.systemMode = systemMode;
    _systemData.measurementMode = measurementMode;
    applySystemData();
#ifdef ARDUINO
    if ((_rtcSyncProvider = getRTC())) { setSyncProvider(rtcNow); }
#endif
    _initialized = true;
    _suspended = true;
}

void Astruino::setObserver(const AstroObserver &observer)
{
    _systemData.latitude = observer.latitudeDegrees;
    _systemData.longitude = observer.longitudeDegrees;
    _systemData.altitude = observer.elevationMeters;
    _mount->setObserver(observer);
}

void Astruino::applySystemData()
{
    _mount->setObserver(AstroObserver(_systemData.latitude, _systemData.longitude, _systemData.altitude));
    scheduler.setConfig(_systemData.scheduler);
    logger.setSubData(&_systemData.logger);
    publisher.setSubData(&_systemData.publisher);
}

bool Astruino::unregisterObject(SharedPtr<AstroObject> object)
{
    if (!object) { return false; }
    _cover.unresolveAny(object.get());
    _thermal.unresolveAny(object.get());
    scheduler.unresolveAny(object.get());
    return AstroObjectRegistration::unregisterObject(object);
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
    if (_camera) { _camera->stopObservation(); }
    if (_mount) { _mount->stow(); }
}


void Astruino::update()
{
    if (!_initialized || _suspended) { return; }

    updateObjects();
    _cover.update();
    scheduler.update();
}

void Astruino::setTimeZoneOffset(int8_t hoursOffset)
{
    if (_systemData.timeZoneOffset != hoursOffset) { _systemData.timeZoneOffset = hoursOffset; }
}

time_t Astruino::getTimeZoneOffset() const
{
    return (time_t)_systemData.timeZoneOffset * 3600L;
}

void Astruino::allocateRTC()
{
#ifdef ARDUINO
    if (!_rtc && _rtcType != Astro_RTCType_None && _rtcSetup.cfgType == AstroDeviceSetup::I2CSetup) {
        switch (_rtcType) {
            case Astro_RTCType_DS1307:
                _rtc = new AstroRTCWrapper<RTC_DS1307>();
                break;
            case Astro_RTCType_DS3231:
                _rtc = new AstroRTCWrapper<RTC_DS3231>();
                break;
            case Astro_RTCType_PCF8523:
                _rtc = new AstroRTCWrapper<RTC_PCF8523>();
                break;
            case Astro_RTCType_PCF8563:
                _rtc = new AstroRTCWrapper<RTC_PCF8563>();
                break;
            default:
                break;
        }
        _rtcBegan = false;
        ASTRO_SOFT_ASSERT(_rtc, SFP(AStr_AllocationFailure));
        ASTRO_HARD_ASSERT(_rtcSetup.i2c.address == 0, SFP(AStr_Err_RTCDefaultAddressOnly));
    }
#endif
}

void Astruino::deallocateRTC()
{
#ifdef ARDUINO
    if (_rtc) {
        if (_rtcSyncProvider == _rtc) {
            setSyncProvider(nullptr);
            _rtcSyncProvider = nullptr;
        }
        delete _rtc;
        _rtc = nullptr;
        _rtcBegan = false;
    }
#endif
}

#ifdef ARDUINO

AstroRTCInterface *Astruino::getRTC(bool begin)
{
    if (!_rtc) { allocateRTC(); }

    if (_rtc && begin && !_rtcBegan) {
        _rtcBegan = _rtc->begin(_rtcSetup.i2c.wire);
        if (_rtcBegan) {
            bool rtcBattFailBefore = _rtcBattFail;
            _rtcBattFail = _rtc->lostPower();
            if (_rtcBattFail && !rtcBattFailBefore) { logger.logWarning((int64_t)unixNow(), SFP(AStr_Log_RTCBatteryFailure)); }
            _rtcSyncProvider = _rtc;
            setSyncProvider(rtcNow);
        } else {
            deallocateRTC();
        }
    }

    return (!begin || _rtcBegan) ? _rtc : nullptr;
}

void Astruino::setRTCTime(DateTime time)
{
    auto rtc = getRTC();
    if (rtc) {
        rtc->adjust(DateTime((uint32_t)unixTime(time)));
        setSyncProvider(rtcNow);
    }
}

#endif
