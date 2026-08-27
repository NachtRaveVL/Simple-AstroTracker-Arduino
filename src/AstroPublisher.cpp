/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Publisher
*/

#include "Astruino.h"

AstroPublisher::AstroPublisher()
    : _dataFilename(), _needsTabulation(false), _pollingFrame(0), _dataColumns(nullptr), _columnSize(0)
#if ASTRO_SYS_LEAVE_FILES_OPEN
      , _dataFileSD(nullptr)
#ifdef ASTRO_USE_WIFI_STORAGE
      , _dataFileWS(nullptr)
#endif
#endif
#ifdef ASTRO_USE_MQTT
    , _mqttClient(nullptr)
#endif
{ ; }

AstroPublisher::~AstroPublisher()
{
    if (_dataColumns) { delete [] _dataColumns; _dataColumns = nullptr; }
    #if ASTRO_SYS_LEAVE_FILES_OPEN
        if (_dataFileSD) { _dataFileSD->flush(); _dataFileSD->close(); delete _dataFileSD; _dataFileSD = nullptr; }
        #ifdef ASTRO_USE_WIFI_STORAGE
            if (_dataFileWS) { _dataFileWS->close(); delete _dataFileWS; _dataFileWS = nullptr; }
        #endif
    #endif
    #ifdef ASTRO_USE_MQTT
        if (_mqttClient) {
            if (_mqttClient->connected()) { _mqttClient->disconnect(); }
            delete _mqttClient; _mqttClient = nullptr;
        }
    #endif
}

void AstroPublisher::update()
{
    if (hasPublisherData()) {
        if (_needsTabulation) { performTabulation(); }

        publishIfNeeded();
    }
}

bool AstroPublisher::beginPublishingToSDCard(String dataFilePrefix)
{
    ASTRO_SOFT_ASSERT(hasPublisherData(), SFP(AStr_Err_NotYetInitialized));

    if (hasPublisherData() && !publisherData()->pubToSDCard) {
        auto sd = Astruino::_activeInstance->getSDCard();

        if (sd) {
            String dataFilename = getYYMMDDFilename(dataFilePrefix, SFP(AStr_csv));
            createDirectoryFor(sd, dataFilename);
            #if ASTRO_SYS_LEAVE_FILES_OPEN
                auto &dataFile = _dataFileSD ? *_dataFileSD : *(_dataFileSD = new File(sd->open(dataFilename.c_str(), FILE_WRITE)));
            #else
                auto dataFile = sd->open(dataFilename.c_str(), FILE_WRITE);
            #endif

            if (dataFile) {
                #if !ASTRO_SYS_LEAVE_FILES_OPEN
                    dataFile.close();
                    Astruino::_activeInstance->endSDCard(sd);
                #endif

                strncpy(publisherData()->dataFilePrefix, dataFilePrefix.c_str(), 16);
                publisherData()->pubToSDCard = true;
                _dataFilename = dataFilename;
                
                setNeedsTabulation();
                Astruino::_activeInstance->_systemData->bumpRevisionIfNeeded();

                return true;
            }

            #if !ASTRO_SYS_LEAVE_FILES_OPEN
                Astruino::_activeInstance->endSDCard(sd);
            #endif
        }
    }

    return false;
}

#ifdef ASTRO_USE_WIFI_STORAGE

bool AstroPublisher::beginPublishingToWiFiStorage(String dataFilePrefix)
{
    ASTRO_SOFT_ASSERT(hasPublisherData(), SFP(AStr_Err_NotYetInitialized));

    if (hasPublisherData() && !publisherData()->pubToWiFiStorage) {
        String dataFilename = getYYMMDDFilename(dataFilePrefix, SFP(AStr_csv));
        #if ASTRO_SYS_LEAVE_FILES_OPEN
            auto &dataFile = _dataFileWS ? *_dataFileWS : *(_dataFileWS = new WiFiStorageFile(WiFiStorage.open(dataFilename.c_str())));
        #else
            auto dataFile = WiFiStorage.open(dataFilename.c_str());
        #endif

        if (dataFile) {
            #if !ASTRO_SYS_LEAVE_FILES_OPEN
                dataFile.close();
            #endif

            strncpy(publisherData()->dataFilePrefix, dataFilePrefix.c_str(), 16);
            publisherData()->pubToWiFiStorage = true;
            _dataFilename = dataFilename;

            setNeedsTabulation();
            Astruino::_activeInstance->_systemData->bumpRevisionIfNeeded();

            return true;
        }
    }

    return false;
}

#endif
#ifdef ASTRO_USE_MQTT

static uint32_t mqttNow()
{
    return unixNow();
}

bool AstroPublisher::beginPublishingToMQTTClient(MQTTClient &client)
{
    ASTRO_SOFT_ASSERT(hasPublisherData(), SFP(AStr_Err_NotYetInitialized));

    if (hasPublisherData() && !_mqttClient) {
        _mqttClient = &client;
        _mqttClient->setClockSource(&mqttNow);
        if (!_mqttClient->connected()) {
            String unPw = String(F("public"));
            _mqttClient->connect(Astruino::_activeInstance->getSystemName().c_str(),
                                 unPw.c_str(), unPw.c_str());
        }

        setNeedsTabulation();

        return true;
    }

    return false;
}

#endif

void AstroPublisher::publishData(aposi_t columnIndex, AstroSingleMeasurement measurement)
{
    ASTRO_SOFT_ASSERT(hasPublisherData() && _dataColumns && _columnSize, SFP(AStr_Err_NotYetInitialized));
    if (_dataColumns && _columnSize && columnIndex >= 0 && columnIndex < _columnSize) {
        _dataColumns[columnIndex].measurement = measurement;
        publishIfNeeded();
    }
}

aposi_t AstroPublisher::getColumnIndexStart(akey_t sensorKey)
{
    ASTRO_SOFT_ASSERT(hasPublisherData() && _dataColumns && _columnSize, SFP(AStr_Err_NotYetInitialized));
    if (_dataColumns && _columnSize) {
        for (int columnIndex = 0; columnIndex < _columnSize; ++columnIndex) {
            if (_dataColumns[columnIndex].sensorKey == sensorKey) {
                return (aposi_t)columnIndex;
            }
        }
    }
    return (aposi_t)-1;
}

Signal<Pair<uint8_t, const AstroDataColumn *>, ASTRO_PUBLISH_SIGNAL_SLOTS> &AstroPublisher::getPublishSignal()
{
    return _publishSignal;
}

void AstroPublisher::notifyDateChanged()
{
    if (isPublishingEnabled()) {
        _dataFilename = getYYMMDDFilename(charsToString(publisherData()->dataFilePrefix, 16), SFP(AStr_csv));
        cleanupOldestData();
    }
}

void AstroPublisher::advancePollingFrame()
{
    ASTRO_HARD_ASSERT(hasPublisherData(), SFP(AStr_Err_NotYetInitialized));

    auto pollingFrame = Astruino::_activeInstance->getPollingFrame();

    if (pollingFrame && _pollingFrame != pollingFrame) {
        time_t timestamp = unixNow();
        _pollingFrame = pollingFrame;

        if (Astruino::_activeInstance->inOperationalMode()) {
            #ifdef ASTRO_USE_MULTITASKING
                scheduleObjectMethodCallOnce<AstroPublisher>(this, &AstroPublisher::publish, timestamp);
            #else
                publish(timestamp);
            #endif
        }
    }

    if (++pollingFrame == 0) { pollingFrame = 1; } // use only valid frame #

    Astruino::_activeInstance->_pollingFrame = pollingFrame;
}

void AstroPublisher::publishIfNeeded()
{
    if (_dataColumns && _columnSize && Astruino::_activeInstance->isPollingFrameOld(_pollingFrame)) {
        bool allCurrent = true;

        for (int columnIndex = 0; columnIndex < _columnSize; ++columnIndex) {
            if (Astruino::_activeInstance->isPollingFrameOld(_dataColumns[columnIndex].measurement.frame)) {
                allCurrent = false;
                break;
            }
        }

        if (allCurrent) {
            time_t timestamp = unixNow();
            _pollingFrame = Astruino::_activeInstance->getPollingFrame();

            if (Astruino::_activeInstance->inOperationalMode()) {
                #ifdef ASTRO_USE_MULTITASKING
                    scheduleObjectMethodCallOnce<AstroPublisher>(this, &AstroPublisher::publish, timestamp);
                #else
                    publish(timestamp);
                #endif
            }
        }
    }
}

void AstroPublisher::publish(time_t timestamp)
{
    if (isPublishingToSDCard()) {
        auto sd = Astruino::_activeInstance->getSDCard(ASTRO_LOFS_BEGIN);

        if (sd) {
            #if ASTRO_SYS_LEAVE_FILES_OPEN
                auto &dataFile = _dataFileSD ? *_dataFileSD : *(_dataFileSD = new File(sd->open(_dataFilename.c_str(), FILE_WRITE)));
            #else
                createDirectoryFor(sd, _dataFilename);
                auto dataFile = sd->open(_dataFilename.c_str(), FILE_WRITE);
            #endif

            if (dataFile) {
                dataFile.print(timestamp);

                for (int columnIndex = 0; columnIndex < _columnSize; ++columnIndex) {
                    dataFile.print(',');
                    dataFile.print(_dataColumns[columnIndex].measurement.value);
                }

                dataFile.println();

                #if !ASTRO_SYS_LEAVE_FILES_OPEN
                    dataFile.flush();
                    dataFile.close();
                #endif
            }

            #if !ASTRO_SYS_LEAVE_FILES_OPEN
                Astruino::_activeInstance->endSDCard(sd);
            #endif
        }
    }

#ifdef ASTRO_USE_WIFI_STORAGE

    if (isPublishingToWiFiStorage()) {
        #if ASTRO_SYS_LEAVE_FILES_OPEN
            auto &dataFile = _dataFileWS ? *_dataFileWS : *(_dataFileWS = new WiFiStorageFile(WiFiStorage.open(_dataFilename.c_str())));
        #else
            auto dataFile = WiFiStorage.open(_dataFilename.c_str());
        #endif

        if (dataFile) {
            auto dataFileStream = AstroWiFiStorageFileStream(dataFile, dataFile.size());
            dataFileStream.print(timestamp);

            for (int columnIndex = 0; columnIndex < _columnSize; ++columnIndex) {
                dataFileStream.print(',');
                dataFileStream.print(_dataColumns[columnIndex].measurement.value);
            }

            dataFileStream.println();
            #if !ASTRO_SYS_LEAVE_FILES_OPEN
                dataFile.close();
            #endif
        }
    }

#endif
#ifdef ASTRO_USE_MQTT

    if (isPublishingToMQTTClient()) {
        String systemName = Astruino::_activeInstance->getSystemName();
        for (int columnIndex = 0; columnIndex < _columnSize; ++columnIndex) {
            auto sensor = (AstroSensor *)(Astruino::_activeInstance->_objects[_dataColumns[columnIndex].sensorKey].get());
            if (sensor) {
                String topic; topic.reserve(systemName.length() + 1 + sensor->getKeyString().length() + 1);
                topic.concat(systemName);
                topic.concat('/');
                topic.concat(sensor->getKeyString());
                String payload = String(_dataColumns[columnIndex].measurement.value, 6); // skipping units/rounding/etc to allow MQTT broker full value data
                _mqttClient->publish(topic.c_str(), payload.c_str());
            }
        }
    }

#endif

    #ifdef ASTRO_USE_MULTITASKING
        scheduleSignalFireOnce<Pair<uint8_t, const AstroDataColumn *>>(_publishSignal, make_pair(_columnSize, (const AstroDataColumn *)_dataColumns));
    #else
        _publishSignal.fire(make_pair(_columnSize, (const AstroDataColumn *)_dataColumns));
    #endif
}

void AstroPublisher::performTabulation()
{
    ASTRO_SOFT_ASSERT(hasPublisherData(), SFP(AStr_Err_NotYetInitialized));

    bool sameOrder = _dataColumns && _columnSize ? true : false;
    int columnSize = 0;

    for (auto iter = Astruino::_activeInstance->_objects.begin(); iter != Astruino::_activeInstance->_objects.end(); ++iter) {
        if (iter->second->isSensorType()) {
            auto sensor = static_pointer_cast<AstroSensor>(iter->second);
            auto rowCount = getMeasurementRowCount(sensor->getMeasurement());

            for (int rowIndex = 0; sameOrder && rowIndex < rowCount; ++rowIndex) {
                sameOrder = sameOrder && (columnSize + rowIndex + 1 <= _columnSize) &&
                            (_dataColumns[columnSize + rowIndex].sensorKey == sensor->getKey());
            }

            columnSize += rowCount;
        }
    }
    sameOrder = sameOrder && (columnSize == _columnSize);

    if (!sameOrder) {
        if (_dataColumns && _columnSize != columnSize) { delete [] _dataColumns; _dataColumns = nullptr; }
        _columnSize = columnSize;

        if (_columnSize) {
            if (!_dataColumns) {
                _dataColumns = new AstroDataColumn[_columnSize];
                ASTRO_SOFT_ASSERT(_dataColumns, SFP(AStr_Err_AllocationFailure));
            }
            if (_dataColumns) {
                int columnIndex = 0;

                for (auto iter = Astruino::_activeInstance->_objects.begin(); iter != Astruino::_activeInstance->_objects.end(); ++iter) {
                    if (iter->second->isSensorType()) {
                        auto sensor = static_pointer_cast<AstroSensor>(iter->second);
                        auto measurement = sensor->getMeasurement();
                        auto rowCount = getMeasurementRowCount(measurement);

                        for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
                            ASTRO_HARD_ASSERT(columnIndex < _columnSize, SFP(AStr_Err_OperationFailure));
                            _dataColumns[columnIndex].measurement = getAsSingleMeasurement(measurement, rowIndex);
                            _dataColumns[columnIndex].sensorKey = sensor->getKey();
                            columnIndex++;
                        }
                    }
                }
            }
        }

        resetDataFile();
    }

    _needsTabulation = false;
}

void AstroPublisher::resetDataFile()
{
    if (isPublishingToSDCard()) {
        auto sd = Astruino::_activeInstance->getSDCard(ASTRO_LOFS_BEGIN);

        if (sd) {
            #if ASTRO_SYS_LEAVE_FILES_OPEN
                if (_dataFileSD) { _dataFileSD->flush(); _dataFileSD->close(); delete _dataFileSD; _dataFileSD = nullptr; }
            #endif
            if (sd->exists(_dataFilename.c_str())) {
                sd->remove(_dataFilename.c_str());
            }
            #if ASTRO_SYS_LEAVE_FILES_OPEN
                auto &dataFile = _dataFileSD ? *_dataFileSD : *(_dataFileSD = new File(sd->open(_dataFilename.c_str(), FILE_WRITE)));
            #else
                createDirectoryFor(sd, _dataFilename);
                auto dataFile = sd->open(_dataFilename.c_str(), FILE_WRITE);
            #endif

            if (dataFile) {
                AstroSensor *lastSensor = nullptr;
                uint8_t measurementRow = 0;

                dataFile.print(SFP(AStr_Key_Timestamp));

                for (int columnIndex = 0; columnIndex < _columnSize; ++columnIndex) {
                    dataFile.print(',');

                    auto sensor = (AstroSensor *)(Astruino::_activeInstance->_objects[_dataColumns[columnIndex].sensorKey].get());
                    if (sensor && sensor == lastSensor) { ++measurementRow; }
                    else { measurementRow = 0; lastSensor = sensor; }

                    if (sensor) {
                        dataFile.print(sensor->getKeyString());
                        dataFile.print('_');
                        dataFile.print(unitsCategoryToString(defaultCategoryForSensor(sensor->getSensorType(), measurementRow)));
                        dataFile.print('_');
                        dataFile.print(unitsTypeToSymbol(getMeasurementUnits(sensor->getMeasurement(), measurementRow)));
                    } else {
                        ASTRO_SOFT_ASSERT(false, SFP(AStr_Err_OperationFailure));
                        dataFile.print(SFP(AStr_Undefined));
                    }
                }

                dataFile.println();

                #if !ASTRO_SYS_LEAVE_FILES_OPEN
                    dataFile.flush();
                    dataFile.close();
                #endif
            }

            #if !ASTRO_SYS_LEAVE_FILES_OPEN
                Astruino::_activeInstance->endSDCard(sd);
            #endif
        }
    }

#ifdef ASTRO_USE_WIFI_STORAGE

    if (isPublishingToWiFiStorage()) {
        #if ASTRO_SYS_LEAVE_FILES_OPEN
            if (_dataFileWS) { _dataFileWS->close(); delete _dataFileWS; _dataFileWS = nullptr; }
        #endif
        if (WiFiStorage.exists(_dataFilename.c_str())) {
            WiFiStorage.remove(_dataFilename.c_str());
        }
        #if ASTRO_SYS_LEAVE_FILES_OPEN
            auto &dataFile = _dataFileWS ? *_dataFileWS : *(_dataFileWS = new WiFiStorageFile(WiFiStorage.open(_dataFilename.c_str())));
        #else
            auto dataFile = WiFiStorage.open(_dataFilename.c_str());
        #endif

        if (dataFile) {
            auto dataFileStream = AstroWiFiStorageFileStream(dataFile);
            AstroSensor *lastSensor = nullptr;
            uint8_t measurementRow = 0;

            dataFileStream.print(SFP(AStr_Key_Timestamp));

            for (int columnIndex = 0; columnIndex < _columnSize; ++columnIndex) {
                dataFileStream.print(',');

                auto sensor = (AstroSensor *)(Astruino::_activeInstance->_objects[_dataColumns[columnIndex].sensorKey].get());
                if (sensor && sensor == lastSensor) { ++measurementRow; }
                else { measurementRow = 0; lastSensor = sensor; }

                if (sensor) {
                    dataFileStream.print(sensor->getKeyString());
                    dataFileStream.print('_');
                    dataFileStream.print(unitsCategoryToString(defaultCategoryForSensor(sensor->getSensorType(), measurementRow)));
                    dataFileStream.print('_');
                    dataFileStream.print(unitsTypeToSymbol(getMeasurementUnits(sensor->getMeasurement(), measurementRow)));
                } else {
                    ASTRO_SOFT_ASSERT(false, SFP(AStr_Err_OperationFailure));
                    dataFileStream.print(SFP(AStr_Undefined));
                }
            }

            dataFileStream.println();
        }
    }

#endif
}

void AstroPublisher::cleanupOldestData(bool force)
{
    // TODO: Old data cleanup. #17 in Astruino.
}


AstroPublisherSubData::AstroPublisherSubData()
    : AstroSubData(0), dataFilePrefix{0}, pubToSDCard(false), pubToWiFiStorage(false)
{ ; }

void AstroPublisherSubData::toJSONObject(JsonObject &objectOut) const
{
    //AstroSubData::toJSONObject(objectOut); // purposeful no call to base method (ignores type)

    if (dataFilePrefix[0]) { objectOut[SFP(AStr_Key_DataFilePrefix)] = charsToString(dataFilePrefix, 16); }
    if (pubToSDCard != false) { objectOut[SFP(AStr_Key_PublishToSDCard)] = pubToSDCard; }
    if (pubToWiFiStorage != false) { objectOut[SFP(AStr_Key_PublishToWiFiStorage)] = pubToWiFiStorage; }
}

void AstroPublisherSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    //AstroSubData::fromJSONObject(objectIn); // purposeful no call to base method (ignores type)

    const char *dataFilePrefixStr = objectIn[SFP(AStr_Key_DataFilePrefix)];
    if (dataFilePrefixStr && dataFilePrefixStr[0]) { strncpy(dataFilePrefix, dataFilePrefixStr, 16); }
    pubToSDCard = objectIn[SFP(AStr_Key_PublishToSDCard)] | pubToSDCard;
    pubToWiFiStorage = objectIn[SFP(AStr_Key_PublishToWiFiStorage)] | pubToWiFiStorage;
}
