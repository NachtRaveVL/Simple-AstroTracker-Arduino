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


Astruino::Astruino(Astro_MountType mountType, Astro_RTCType rtcType, AstroDeviceSetup rtcSetup)
    : scheduler(), logger(), publisher(), _systemData(),
      _mount(new AstroMount(mountType, 0)), _cover(new AstroCover((aposi_t)0)), _camera(new AstroCameraTrigger(nullptr, nullptr, 0)),
      _thermal(), _rtcType(rtcType), _rtcSetup(rtcSetup)
#ifdef ARDUINO
      , _rtc(nullptr)
#endif
      , _rtcBegan(false), _rtcBattFail(false), _initialized(false), _suspended(true)
{
    if (!_activeInstance) { _activeInstance = this; }

    registerObject(_mount);
    registerObject(_cover);
    registerObject(_camera);

    scheduler.setMount(_mount);
    scheduler.setCover(_cover);
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
    _systemData.observer = observer;
    _mount->setObserver(observer);
}

void Astruino::applySystemData()
{
    _mount->setObserver(_systemData.observer);
    scheduler.setConfig(_systemData.scheduler);
    logger.setSubData(&_systemData.logger);
    publisher.setSubData(&_systemData.publisher);
}

bool Astruino::unregisterObject(SharedPtr<AstroObject> object)
{
    if (!object) { return false; }
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

    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
        if (iter->second) { iter->second->update(); }
    }
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
            if (_rtcBattFail && !rtcBattFailBefore) { logger.logWarning((int64_t)unixNow(), "RTC battery failure"); }
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
