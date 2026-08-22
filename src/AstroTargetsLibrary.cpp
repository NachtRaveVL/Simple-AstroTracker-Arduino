/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Targets Library
*/

#include "Astruino.h"

AstroTargetsLibraryBook::AstroTargetsLibraryBook()
    : data(), count(1), userSet(false)
{ ; }

AstroTargetsLibraryBook::AstroTargetsLibraryBook(String jsonStringIn)
    : data(), count(1), userSet(false)
{
    StaticJsonDocument<ASTRO_JSON_DOC_DEFSIZE> doc;
    deserializeJson(doc, jsonStringIn);
    auto targetsLibDataObj = doc.as<JsonObjectConst>();
    data.fromJSONObject(targetsLibDataObj);
}

AstroTargetsLibraryBook::AstroTargetsLibraryBook(Stream &streamIn, bool jsonFormat)
    : data(), count(1), userSet(false)
{
    if (jsonFormat) {
        StaticJsonDocument<ASTRO_JSON_DOC_DEFSIZE> doc;
        deserializeJson(doc, streamIn);
        auto targetsLibDataObj = doc.as<JsonObjectConst>();
        data.fromJSONObject(targetsLibDataObj);
    } else {
        deserializeDataFromBinaryStream(&data, &streamIn);
    }
}

AstroTargetsLibraryBook::AstroTargetsLibraryBook(const AstroTargetsLibData &dataIn)
    : data(dataIn), count(1), userSet(false)
{ ; }


AstroTargetsLibrary hydroTargetsLib;

void AstroTargetsLibrary::beginTargetsLibraryFromSDCard(String dataFilePrefix, bool jsonFormat)
{
    _libSDTargetPrefix = dataFilePrefix;
    _libSDJSONFormat = jsonFormat;
}

void AstroTargetsLibrary::beginTargetsLibraryFromEEPROM(size_t dataAddress, bool jsonFormat)
{
    _libEEPROMDataAddress = dataAddress;
    _libEEPROMJSONFormat = jsonFormat;
}

const AstroTargetsLibData *AstroTargetsLibrary::checkoutTargetsData(Astro_TargetType targetType)
{
    ASTRO_SOFT_ASSERT((int)targetType >= 0 && targetType < Astro_TargetType_Count, SFP(AStr_Err_InvalidParameter));

    AstroTargetsLibraryBook *book = nullptr;
    auto iter = _targetsData.find(targetType);

    if (iter != _targetsData.end()) {
        book = iter->second;
        if (book) {
            book->count += 1;
        }
    } else {
        book = newBookFromType(targetType);

        ASTRO_SOFT_ASSERT(book || targetType >= Astro_TargetType_CustomTarget1, SFP(AStr_Err_AllocationFailure));
        if (book) {
            _targetsData[targetType] = book;
            ASTRO_HARD_ASSERT(_targetsData.find(targetType) != _targetsData.end(), SFP(AStr_Err_OperationFailure));
        }
    }

    return book ? &(book->data) : nullptr;
}

void AstroTargetsLibrary::returnTargetsData(const AstroTargetsLibData *targetData)
{
    ASTRO_HARD_ASSERT(targetData, SFP(AStr_Err_InvalidParameter));

    auto iter = _targetsData.find(targetData->targetType);
    ASTRO_SOFT_ASSERT(iter != _targetsData.end(), F("No check outs for target type"));

    if (iter != _targetsData.end()) {
        auto book = iter->second;
        if (book) {
            book->count--;

            if (book->count <= 0 && (!book->userSet || !book->data.isModified())) {
                delete iter->second;
                _targetsData.erase(iter);
            }
        }
    }
}

bool AstroTargetsLibrary::setUserTargetData(const AstroTargetsLibData *targetData)
{
    ASTRO_HARD_ASSERT(targetData, SFP(AStr_Err_InvalidParameter));

    auto iter = _targetsData.find(targetData->targetType);
    bool retVal = false;

    if (iter == _targetsData.end()) {
        auto book = new AstroTargetsLibraryBook(*targetData);
        ASTRO_HARD_ASSERT(book, SFP(AStr_Err_AllocationFailure));

        book->userSet = true;
        _targetsData[targetData->targetType] = book;
        retVal = (_targetsData.find(targetData->targetType) != _targetsData.end());
    } else {
        iter->second->data = *targetData;
        iter->second->userSet = true;
        retVal = true;
    }

    if (retVal) {
        _hasUserTargets = true;
        updateTargetsOfType(targetData->targetType);
        return true;
    }
    return false;
}

bool AstroTargetsLibrary::dropUserTargetData(const AstroTargetsLibData *targetData)
{
    ASTRO_HARD_ASSERT(targetData, SFP(AStr_Err_InvalidParameter));

    auto iter = _targetsData.find(targetData->targetType);

    if (iter != _targetsData.end()) {
        delete iter->second;
        _targetsData.erase(iter);

        updateHasUserTargets();
        updateTargetsOfType(targetData->targetType);
        return true;
    }

    return false;
}

bool AstroTargetsLibrary::updateHasUserTargets()
{
    for (auto iter = _targetsData.begin(); iter != _targetsData.end(); ++iter) {
        if (iter->second->userSet) {
            return (_hasUserTargets = true);
        }
    }
    return (_hasUserTargets = false);
}

void AstroTargetsLibrary::updateTargetsOfType(Astro_TargetType targetType)
{
    if (Hydruino::_activeInstance) {
        for (auto iter = Hydruino::_activeInstance->_objects.begin(); iter != Hydruino::_activeInstance->_objects.end(); ++iter) {
            if (iter->second->isTargetType()) {
                auto target = static_pointer_cast<AstroTarget>(iter->second);
                if (target->getTargetType() == targetType) {
                    bool incCount = false;
                    if (_targetsData.find(targetType) != _targetsData.end()) {
                        _targetsData[targetType]->count++; // prevents auto-deletion of underlying data
                        incCount = true;
                    }

                    target->returnTargetsLibData(); // forces new data checkout
                    target->recalcGrowthParams();

                    if (incCount) {
                        _targetsData[targetType]->count--;
                    }
                }
            }
        }
    }
}

inline String getTargetFilename(const String &libSDTargetPrefix, Astro_TargetType targetType)
{
    String filename; filename.reserve(libSDTargetPrefix.length() + 4 + 1);
    filename.concat(libSDTargetPrefix);
    filename.concat('c');
    filename.concat('r');
    filename.concat('o');
    filename.concat('p');
    return getNNFilename(filename, (unsigned int)targetType, SFP(AStr_dat));
}

AstroTargetsLibraryBook *AstroTargetsLibrary::newBookFromType(Astro_TargetType targetType)
{
    if (_libSDTargetPrefix.length()) {
        AstroTargetsLibraryBook *retVal = nullptr;
        auto sd = getController()->getSDCard();

        if (sd) {
            auto file = sd->open(getTargetFilename(_libSDTargetPrefix, targetType).c_str(), FILE_READ);

            if (file) {
                retVal = new AstroTargetsLibraryBook(file, _libSDJSONFormat);
                file.close();
            }

            getController()->endSDCard(sd);
        }

        if (retVal) { return retVal; }
    }

    if (_libEEPROMDataAddress != (size_t)-1) {
        AstroTargetsLibraryBook *retVal = nullptr;
        auto eeprom = getController()->getEEPROM();

        if (eeprom) {
            uint16_t lookupOffset = 0;
            eeprom->readBlock(_libEEPROMDataAddress + (((int)targetType + 1) * sizeof(uint16_t)), // +1 for initial total size word
                              (uint8_t *)&lookupOffset, sizeof(uint16_t));

            if (lookupOffset) {
                auto eepromStream = AstroEEPROMStream(lookupOffset, sizeof(AstroTargetsLibData));
                retVal = new AstroTargetsLibraryBook(eepromStream, _libEEPROMJSONFormat);
            }
        }

        if (retVal) { return retVal; }
    }

    #ifndef ASTRO_DISABLE_BUILTIN_DATA
    {   AstroPROGMEMStream progmemStream(0, 0);
        switch (targetType) {
            case Astro_TargetType_AloeVera: {
                static const char flashStr_AloeVera[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"AloeVera\",\"targetName\":\"Aloe Vera\",\"totalGrowWeeks\":52,\"phaseDurationWeeks\":\"4,24,24\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"7,8.5\",\"tdsRange\":\"1.8,2.5\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,28\",\"co2Levels\":\"600,700\",\"flags\":\"invasive,perennial,toxic\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_AloeVera);
            } break;
            case Astro_TargetType_Anise: {
                static const char flashStr_Anise[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Anise\",\"targetName\":\"Anise\",\"totalGrowWeeks\":12,\"phaseDurationWeeks\":\"2,6,4\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"5.8,6.4\",\"tdsRange\":\"0.9,1.4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"19,23\",\"airTempRange\":\"18,26\",\"co2Levels\":\"600,750\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Anise);
            } break;
            case Astro_TargetType_Artichoke: {
                static const char flashStr_Artichoke[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Artichoke\",\"targetName\":\"Artichoke\",\"totalGrowWeeks\":30,\"phaseDurationWeeks\":\"10,12,8\",\"dailyLightHours\":\"16,16,14\",\"phRange\":\"6.5,7.5\",\"tdsRange\":\"0.8,1.8\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,23\",\"airTempRange\":\"16,26\",\"co2Levels\":\"650,800\",\"flags\":\"perennial\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Artichoke);
            } break;
            case Astro_TargetType_Arugula: {
                static const char flashStr_Arugula[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Arugula\",\"targetName\":\"Arugula\",\"totalGrowWeeks\":5,\"phaseDurationWeeks\":\"1,3,1\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6,7.5\",\"tdsRange\":\"0.8,1.8\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,23\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Arugula);
            } break;
            case Astro_TargetType_Asparagus: {
                static const char flashStr_Asparagus[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Asparagus\",\"targetName\":\"Asparagus\",\"totalGrowWeeks\":52,\"phaseDurationWeeks\":\"4,24,24\",\"dailyLightHours\":\"16,16,14\",\"phRange\":\"6,6.8\",\"tdsRange\":\"1.4,1.8\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,23\",\"airTempRange\":\"16,26\",\"co2Levels\":\"650,800\",\"flags\":\"perennial,pruning\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Asparagus);
            } break;
            case Astro_TargetType_Basil: {
                static const char flashStr_Basil[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Basil\",\"targetName\":\"Basil\",\"totalGrowWeeks\":6,\"phaseDurationWeeks\":\"1,4,1\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"5.5,6\",\"tdsRange\":\"1,1.6\",\"nightlyFeedRate\":1,\"waterTempRange\":\"19,23\",\"airTempRange\":\"18,26\",\"co2Levels\":\"600,750\",\"flags\":\"pruning\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Basil);
            } break;
            case Astro_TargetType_Bean: {
                static const char flashStr_Bean[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Bean\",\"targetName\":\"Bean (common)\",\"totalGrowWeeks\":8,\"phaseDurationWeeks\":\"1,4,3\",\"dailyLightHours\":\"16,16,14\",\"phRange\":\"6,6\",\"tdsRange\":\"2,4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"18,25\",\"co2Levels\":\"600,800\",\"flags\":\"pruning\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Bean);
            } break;
            case Astro_TargetType_BeanBroad: {
                static const char flashStr_BeanBroad[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"BeanBroad\",\"targetName\":\"Bean (broad)\",\"totalGrowWeeks\":14,\"phaseDurationWeeks\":\"2,6,6\",\"dailyLightHours\":\"16,16,14\",\"phRange\":\"6,6.5\",\"tdsRange\":\"2,4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"18,25\",\"co2Levels\":\"600,800\",\"flags\":\"pruning\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_BeanBroad);
            } break;
            case Astro_TargetType_Beetroot: {
                static const char flashStr_Beetroot[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Beetroot\",\"targetName\":\"Beetroot\",\"totalGrowWeeks\":9,\"phaseDurationWeeks\":\"1,6,2\",\"dailyLightHours\":\"16,14,12\",\"phRange\":\"6,6.5\",\"tdsRange\":\"1.8,2.5\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,24\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Beetroot);
            } break;
            case Astro_TargetType_BlackCurrant: {
                static const char flashStr_BlackCurrant[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"BlackCurrant\",\"targetName\":\"Black Currant\",\"totalGrowWeeks\":52,\"phaseDurationWeeks\":\"4,24,24\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1.4,1.8\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,24\",\"co2Levels\":\"600,750\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_BlackCurrant);
            } break;
            case Astro_TargetType_Blueberry: {
                static const char flashStr_Blueberry[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Blueberry\",\"targetName\":\"Blueberry\",\"totalGrowWeeks\":52,\"phaseDurationWeeks\":\"4,24,24\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"4,5\",\"tdsRange\":\"1.8,2\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,24\",\"co2Levels\":\"600,750\",\"flags\":\"perennial\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Blueberry);
            } break;
            case Astro_TargetType_BokChoi: {
                static const char flashStr_BokChoi[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"BokChoi\",\"targetName\":\"Bok-choi\",\"totalGrowWeeks\":6,\"phaseDurationWeeks\":\"1,4,1\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"7,7\",\"tdsRange\":\"1.5,2\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,23\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_BokChoi);
            } break;
            case Astro_TargetType_Broccoli: {
                static const char flashStr_Broccoli[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Broccoli\",\"targetName\":\"Broccoli\",\"totalGrowWeeks\":14,\"phaseDurationWeeks\":\"5,6,3\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6,6.8\",\"tdsRange\":\"2.8,3.5\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,23\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Broccoli);
            } break;
            case Astro_TargetType_BrusselsSprout: {
                static const char flashStr_BrusselsSprout[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"BrusselsSprout\",\"targetName\":\"Brussels Sprouts\",\"totalGrowWeeks\":19,\"phaseDurationWeeks\":\"5,7,7\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6.5,7.5\",\"tdsRange\":\"2.5,3\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,23\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_BrusselsSprout);
            } break;
            case Astro_TargetType_Cabbage: {
                static const char flashStr_Cabbage[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Cabbage\",\"targetName\":\"Cabbage\",\"totalGrowWeeks\":16,\"phaseDurationWeeks\":\"5,7,4\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6.5,7\",\"tdsRange\":\"2.5,3\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,23\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Cabbage);
            } break;
            case Astro_TargetType_Cannabis: {
                static const char flashStr_Cannabis[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Cannabis\",\"targetName\":\"Cannabis (generic)\",\"totalGrowWeeks\":16,\"phaseDurationWeeks\":\"2,6,8\",\"dailyLightHours\":\"18,18,12\",\"phRange\":\"5.5,6.1\",\"tdsRange\":\"1,2.5\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,23\",\"airTempRange\":\"20,28\",\"co2Levels\":\"800,1000\",\"flags\":\"large\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Cannabis);
            } break;
            case Astro_TargetType_Capsicum: {
                static const char flashStr_Capsicum[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Capsicum\",\"targetName\":\"Capsicum\",\"totalGrowWeeks\":18,\"phaseDurationWeeks\":\"8,5,5\",\"dailyLightHours\":\"18,16,14\",\"phRange\":\"5.5,6\",\"tdsRange\":\"0.8,1.8\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,28\",\"co2Levels\":\"700,900\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Capsicum);
            } break;
            case Astro_TargetType_Carrots: {
                static const char flashStr_Carrots[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Carrots\",\"targetName\":\"Carrots\",\"totalGrowWeeks\":10,\"phaseDurationWeeks\":\"1,6,3\",\"dailyLightHours\":\"16,14,12\",\"phRange\":\"6.3,6.3\",\"tdsRange\":\"1.6,2\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,24\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Carrots);
            } break;
            case Astro_TargetType_Catnip: {
                static const char flashStr_Catnip[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Catnip\",\"targetName\":\"Catnip\",\"totalGrowWeeks\":10,\"phaseDurationWeeks\":\"2,5,3\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1,1.6\",\"nightlyFeedRate\":1,\"waterTempRange\":\"19,23\",\"airTempRange\":\"18,26\",\"co2Levels\":\"600,750\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Catnip);
            } break;
            case Astro_TargetType_Cauliflower: {
                static const char flashStr_Cauliflower[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Cauliflower\",\"targetName\":\"Cauliflower\",\"totalGrowWeeks\":16,\"phaseDurationWeeks\":\"5,8,3\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6,7\",\"tdsRange\":\"0.5,2\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,23\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Cauliflower);
            } break;
            case Astro_TargetType_Celery: {
                static const char flashStr_Celery[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Celery\",\"targetName\":\"Celery\",\"totalGrowWeeks\":27,\"phaseDurationWeeks\":\"11,12,4\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6.5,6.5\",\"tdsRange\":\"1.8,2.4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,23\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Celery);
            } break;
            case Astro_TargetType_Chamomile: {
                static const char flashStr_Chamomile[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Chamomile\",\"targetName\":\"Chamomile\",\"totalGrowWeeks\":10,\"phaseDurationWeeks\":\"2,5,3\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1,1.6\",\"nightlyFeedRate\":1,\"waterTempRange\":\"19,23\",\"airTempRange\":\"18,26\",\"co2Levels\":\"600,750\",\"flags\":\"toxic\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Chamomile);
            } break;
            case Astro_TargetType_Chicory: {
                static const char flashStr_Chicory[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Chicory\",\"targetName\":\"Chicory\",\"totalGrowWeeks\":10,\"phaseDurationWeeks\":\"2,5,3\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"5.5,6\",\"tdsRange\":\"2,2.4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,23\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Chicory);
            } break;
            case Astro_TargetType_Chives: {
                static const char flashStr_Chives[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Chives\",\"targetName\":\"Chives\",\"totalGrowWeeks\":10,\"phaseDurationWeeks\":\"2,5,3\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6,6.5\",\"tdsRange\":\"1.8,2.4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,24\",\"co2Levels\":\"600,700\",\"flags\":\"perennial,toxic\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Chives);
            } break;
            case Astro_TargetType_Cilantro: {
                static const char flashStr_Cilantro[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Cilantro\",\"targetName\":\"Cilantro\",\"totalGrowWeeks\":9,\"phaseDurationWeeks\":\"1,5,3\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6.5,6.7\",\"tdsRange\":\"1.3,1.8\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,24\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Cilantro);
            } break;
            case Astro_TargetType_Coriander: {
                static const char flashStr_Coriander[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Coriander\",\"targetName\":\"Coriander\",\"totalGrowWeeks\":12,\"phaseDurationWeeks\":\"1,6,5\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"5.8,6.4\",\"tdsRange\":\"1.2,1.8\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,24\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Coriander);
            } break;
            case Astro_TargetType_CornSweet: {
                static const char flashStr_CornSweet[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"CornSweet\",\"targetName\":\"Corn (sweet)\",\"totalGrowWeeks\":11,\"phaseDurationWeeks\":\"1,6,4\",\"dailyLightHours\":\"18,16,14\",\"phRange\":\"5.8,6.5\",\"tdsRange\":\"1.6,2.4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,30\",\"co2Levels\":\"700,900\",\"flags\":\"large,toxic\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_CornSweet);
            } break;
            case Astro_TargetType_Cucumber: {
                static const char flashStr_Cucumber[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Cucumber\",\"targetName\":\"Cucumber\",\"totalGrowWeeks\":13,\"phaseDurationWeeks\":\"4,4,5\",\"dailyLightHours\":\"18,16,14\",\"phRange\":\"5,5.5\",\"tdsRange\":\"1.7,2\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,28\",\"co2Levels\":\"700,900\",\"flags\":\"pruning\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Cucumber);
            } break;
            case Astro_TargetType_Dill: {
                static const char flashStr_Dill[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Dill\",\"targetName\":\"Dill\",\"totalGrowWeeks\":8,\"phaseDurationWeeks\":\"1,4,3\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"5.5,6.4\",\"tdsRange\":\"1,1.6\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,24\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Dill);
            } break;
            case Astro_TargetType_Eggplant: {
                static const char flashStr_Eggplant[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Eggplant\",\"targetName\":\"Eggplant\",\"totalGrowWeeks\":19,\"phaseDurationWeeks\":\"8,6,5\",\"dailyLightHours\":\"18,16,14\",\"phRange\":\"6,6\",\"tdsRange\":\"2.5,3.5\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,28\",\"co2Levels\":\"700,900\",\"flags\":\"pruning\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Eggplant);
            } break;
            case Astro_TargetType_Endive: {
                static const char flashStr_Endive[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Endive\",\"targetName\":\"Endive\",\"totalGrowWeeks\":13,\"phaseDurationWeeks\":\"2,8,3\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"5.5,5.5\",\"tdsRange\":\"2,2.4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,23\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Endive);
            } break;
            case Astro_TargetType_Fennel: {
                static const char flashStr_Fennel[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Fennel\",\"targetName\":\"Fennel\",\"totalGrowWeeks\":12,\"phaseDurationWeeks\":\"2,6,4\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6.4,6.8\",\"tdsRange\":\"1,1.4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"19,23\",\"airTempRange\":\"18,26\",\"co2Levels\":\"600,750\",\"flags\":\"perennial\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Fennel);
            } break;
            case Astro_TargetType_Fodder: {
                static const char flashStr_Fodder[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Fodder\",\"targetName\":\"Fodder\",\"totalGrowWeeks\":4,\"phaseDurationWeeks\":\"1,2,1\",\"dailyLightHours\":\"16,14,12\",\"phRange\":\"5.8,6.5\",\"tdsRange\":\"1.8,2\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,24\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Fodder);
            } break;
            case Astro_TargetType_Flowers: {
                static const char flashStr_Flowers[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Flowers\",\"targetName\":\"Flowers (generic)\",\"totalGrowWeeks\":16,\"phaseDurationWeeks\":\"2,6,8\",\"dailyLightHours\":\"18,16,14\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1.5,2.5\",\"nightlyFeedRate\":1,\"waterTempRange\":\"19,23\",\"airTempRange\":\"18,26\",\"co2Levels\":\"700,900\",\"flags\":\"toxic,pruning\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Flowers);
            } break;
            case Astro_TargetType_Garlic: {
                static const char flashStr_Garlic[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Garlic\",\"targetName\":\"Garlic\",\"totalGrowWeeks\":24,\"phaseDurationWeeks\":\"3,10,11\",\"dailyLightHours\":\"16,14,12\",\"phRange\":\"6,6.5\",\"tdsRange\":\"1.4,1.8\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,24\",\"co2Levels\":\"600,700\",\"flags\":\"perennial,toxic\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Garlic);
            } break;
            case Astro_TargetType_Ginger: {
                static const char flashStr_Ginger[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Ginger\",\"targetName\":\"Ginger\",\"totalGrowWeeks\":40,\"phaseDurationWeeks\":\"4,24,12\",\"dailyLightHours\":\"16,14,12\",\"phRange\":\"5.8,6\",\"tdsRange\":\"1.5,2\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,28\",\"co2Levels\":\"600,750\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Ginger);
            } break;
            case Astro_TargetType_Kale: {
                static const char flashStr_Kale[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Kale\",\"targetName\":\"Kale\",\"totalGrowWeeks\":8,\"phaseDurationWeeks\":\"1,5,2\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1.25,1.5\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,23\",\"co2Levels\":\"600,700\",\"flags\":\"perennial\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Kale);
            } break;
            case Astro_TargetType_Lavender: {
                static const char flashStr_Lavender[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Lavender\",\"targetName\":\"Lavender\",\"totalGrowWeeks\":16,\"phaseDurationWeeks\":\"2,6,8\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6.4,6.8\",\"tdsRange\":\"1,1.4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"19,23\",\"airTempRange\":\"18,26\",\"co2Levels\":\"600,750\",\"flags\":\"perennial,toxic\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Lavender);
            } break;
            case Astro_TargetType_Leek: {
                static const char flashStr_Leek[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Leek\",\"targetName\":\"Leek\",\"totalGrowWeeks\":23,\"phaseDurationWeeks\":\"7,12,4\",\"dailyLightHours\":\"16,14,12\",\"phRange\":\"6.5,7\",\"tdsRange\":\"1.4,1.8\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,24\",\"co2Levels\":\"600,700\",\"flags\":\"toxic\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Leek);
            } break;
            case Astro_TargetType_LemonBalm: {
                static const char flashStr_LemonBalm[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"LemonBalm\",\"targetName\":\"Lemon Balm\",\"totalGrowWeeks\":12,\"phaseDurationWeeks\":\"2,6,4\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1,1.6\",\"nightlyFeedRate\":1,\"waterTempRange\":\"19,23\",\"airTempRange\":\"18,26\",\"co2Levels\":\"600,750\",\"flags\":\"perennial\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_LemonBalm);
            } break;
            case Astro_TargetType_Lettuce: {
                static const char flashStr_Lettuce[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Lettuce\",\"targetName\":\"Lettuce\",\"totalGrowWeeks\":5,\"phaseDurationWeeks\":\"2,2,1\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6,7\",\"tdsRange\":\"1.2,1.8\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,23\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Lettuce);
            } break;
            case Astro_TargetType_Marrow: {
                static const char flashStr_Marrow[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Marrow\",\"targetName\":\"Marrow\",\"totalGrowWeeks\":12,\"phaseDurationWeeks\":\"2,6,4\",\"dailyLightHours\":\"18,16,14\",\"phRange\":\"6,6\",\"tdsRange\":\"1.8,2.4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,28\",\"co2Levels\":\"700,900\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Marrow);
            } break;
            case Astro_TargetType_Melon: {
                static const char flashStr_Melon[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Melon\",\"targetName\":\"Melon\",\"totalGrowWeeks\":16,\"phaseDurationWeeks\":\"4,6,6\",\"dailyLightHours\":\"18,16,14\",\"phRange\":\"5.5,6\",\"tdsRange\":\"2,2.5\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,28\",\"co2Levels\":\"700,900\",\"flags\":\"large\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Melon);
            } break;
            case Astro_TargetType_Mint: {
                static const char flashStr_Mint[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Mint\",\"targetName\":\"Mint\",\"totalGrowWeeks\":10,\"phaseDurationWeeks\":\"2,5,3\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"5.5,6\",\"tdsRange\":\"2,2.4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"19,23\",\"airTempRange\":\"18,26\",\"co2Levels\":\"600,750\",\"flags\":\"invasive,perennial,toxic\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Mint);
            } break;
            case Astro_TargetType_MustardCress: {
                static const char flashStr_MustardCress[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"MustardCress\",\"targetName\":\"Mustard Cress\",\"totalGrowWeeks\":6,\"phaseDurationWeeks\":\"1,3,2\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6,6.5\",\"tdsRange\":\"1.2,2.4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,23\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_MustardCress);
            } break;
            case Astro_TargetType_Okra: {
                static const char flashStr_Okra[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Okra\",\"targetName\":\"Okra\",\"totalGrowWeeks\":9,\"phaseDurationWeeks\":\"1,5,3\",\"dailyLightHours\":\"18,16,14\",\"phRange\":\"6.5,6.5\",\"tdsRange\":\"2,2.4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,28\",\"co2Levels\":\"700,900\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Okra);
            } break;
            case Astro_TargetType_Onions: {
                static const char flashStr_Onions[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Onions\",\"targetName\":\"Onions\",\"totalGrowWeeks\":16,\"phaseDurationWeeks\":\"2,6,8\",\"dailyLightHours\":\"16,14,12\",\"phRange\":\"6,6.7\",\"tdsRange\":\"1.4,1.8\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,24\",\"co2Levels\":\"600,700\",\"flags\":\"perennial,toxic\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Onions);
            } break;
            case Astro_TargetType_Oregano: {
                static const char flashStr_Oregano[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Oregano\",\"targetName\":\"Oregano\",\"totalGrowWeeks\":12,\"phaseDurationWeeks\":\"2,6,4\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6,7\",\"tdsRange\":\"1.8,2.3\",\"nightlyFeedRate\":1,\"waterTempRange\":\"19,23\",\"airTempRange\":\"18,26\",\"co2Levels\":\"600,750\",\"flags\":\"perennial,toxic\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Oregano);
            } break;
            case Astro_TargetType_PakChoi: {
                static const char flashStr_PakChoi[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"PakChoi\",\"targetName\":\"Pak-choi\",\"totalGrowWeeks\":6,\"phaseDurationWeeks\":\"1,4,1\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"7,7\",\"tdsRange\":\"1.5,2\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,23\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_PakChoi);
            } break;
            case Astro_TargetType_Parsley: {
                static const char flashStr_Parsley[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Parsley\",\"targetName\":\"Parsley\",\"totalGrowWeeks\":10,\"phaseDurationWeeks\":\"3,5,2\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6,6.5\",\"tdsRange\":\"1.8,2.2\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,24\",\"co2Levels\":\"600,700\",\"flags\":\"perennial,toxic\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Parsley);
            } break;
            case Astro_TargetType_Parsnip: {
                static const char flashStr_Parsnip[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Parsnip\",\"targetName\":\"Parsnip\",\"totalGrowWeeks\":16,\"phaseDurationWeeks\":\"2,6,8\",\"dailyLightHours\":\"16,14,12\",\"phRange\":\"6,6.5\",\"tdsRange\":\"1.4,1.8\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,24\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Parsnip);
            } break;
            case Astro_TargetType_Pea: {
                static const char flashStr_Pea[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Pea\",\"targetName\":\"Pea (common)\",\"totalGrowWeeks\":9,\"phaseDurationWeeks\":\"1,5,3\",\"dailyLightHours\":\"16,16,14\",\"phRange\":\"6,7\",\"tdsRange\":\"0.8,1.8\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"18,25\",\"co2Levels\":\"600,800\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Pea);
            } break;
            case Astro_TargetType_PeaSugar: {
                static const char flashStr_PeaSugar[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"PeaSugar\",\"targetName\":\"Pea (sugar)\",\"totalGrowWeeks\":9,\"phaseDurationWeeks\":\"1,5,3\",\"dailyLightHours\":\"16,16,14\",\"phRange\":\"6,6.8\",\"tdsRange\":\"0.8,1.9\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"18,25\",\"co2Levels\":\"600,800\",\"flags\":\"toxic\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_PeaSugar);
            } break;
            case Astro_TargetType_Pepino: {
                static const char flashStr_Pepino[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Pepino\",\"targetName\":\"Pepino\",\"totalGrowWeeks\":16,\"phaseDurationWeeks\":\"2,6,8\",\"dailyLightHours\":\"18,16,14\",\"phRange\":\"6,6.5\",\"tdsRange\":\"2,2.5\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,28\",\"co2Levels\":\"700,900\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Pepino);
            } break;
            case Astro_TargetType_PeppersBell: {
                static const char flashStr_PeppersBell[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"PeppersBell\",\"targetName\":\"Peppers (bell)\",\"totalGrowWeeks\":18,\"phaseDurationWeeks\":\"8,5,5\",\"dailyLightHours\":\"18,16,14\",\"phRange\":\"5.5,6\",\"tdsRange\":\"0.8,1.8\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,28\",\"co2Levels\":\"700,900\",\"flags\":\"pruning\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_PeppersBell);
            } break;
            case Astro_TargetType_PeppersHot: {
                static const char flashStr_PeppersHot[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"PeppersHot\",\"targetName\":\"Peppers (hot)\",\"totalGrowWeeks\":18,\"phaseDurationWeeks\":\"8,5,5\",\"dailyLightHours\":\"18,16,14\",\"phRange\":\"5.5,6\",\"tdsRange\":\"0.8,1.8\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,28\",\"co2Levels\":\"700,900\",\"flags\":\"pruning\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_PeppersHot);
            } break;
            case Astro_TargetType_Potato: {
                static const char flashStr_Potato[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Potato\",\"targetName\":\"Potato (common)\",\"totalGrowWeeks\":16,\"phaseDurationWeeks\":\"2,6,8\",\"dailyLightHours\":\"16,14,12\",\"phRange\":\"5,6\",\"tdsRange\":\"2,2.5\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,24\",\"co2Levels\":\"600,700\",\"flags\":\"perennial\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Potato);
            } break;
            case Astro_TargetType_PotatoSweet: {
                static const char flashStr_PotatoSweet[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"PotatoSweet\",\"targetName\":\"Potato (sweet)\",\"totalGrowWeeks\":18,\"phaseDurationWeeks\":\"6,8,4\",\"dailyLightHours\":\"16,14,12\",\"phRange\":\"5,6\",\"tdsRange\":\"2,2.5\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,28\",\"co2Levels\":\"600,750\",\"flags\":\"perennial\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_PotatoSweet);
            } break;
            case Astro_TargetType_Pumpkin: {
                static const char flashStr_Pumpkin[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Pumpkin\",\"targetName\":\"Pumpkin\",\"totalGrowWeeks\":19,\"phaseDurationWeeks\":\"4,8,7\",\"dailyLightHours\":\"18,16,14\",\"phRange\":\"5.5,7.5\",\"tdsRange\":\"1.8,2.4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,28\",\"co2Levels\":\"700,900\",\"flags\":\"large,pruning\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Pumpkin);
            } break;
            case Astro_TargetType_Radish: {
                static const char flashStr_Radish[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Radish\",\"targetName\":\"Radish\",\"totalGrowWeeks\":5,\"phaseDurationWeeks\":\"1,2,2\",\"dailyLightHours\":\"16,14,12\",\"phRange\":\"6,7\",\"tdsRange\":\"1.6,2.2\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,24\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Radish);
            } break;
            case Astro_TargetType_Rhubarb: {
                static const char flashStr_Rhubarb[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Rhubarb\",\"targetName\":\"Rhubarb\",\"totalGrowWeeks\":52,\"phaseDurationWeeks\":\"4,24,24\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"5.5,6\",\"tdsRange\":\"1.6,2\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,24\",\"co2Levels\":\"600,750\",\"flags\":\"perennial,toxic\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Rhubarb);
            } break;
            case Astro_TargetType_Rosemary: {
                static const char flashStr_Rosemary[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Rosemary\",\"targetName\":\"Rosemary\",\"totalGrowWeeks\":16,\"phaseDurationWeeks\":\"2,6,8\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"5.5,6\",\"tdsRange\":\"1,1.6\",\"nightlyFeedRate\":1,\"waterTempRange\":\"19,23\",\"airTempRange\":\"18,26\",\"co2Levels\":\"600,750\",\"flags\":\"perennial\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Rosemary);
            } break;
            case Astro_TargetType_Sage: {
                static const char flashStr_Sage[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Sage\",\"targetName\":\"Sage\",\"totalGrowWeeks\":12,\"phaseDurationWeeks\":\"2,6,4\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1,1.6\",\"nightlyFeedRate\":1,\"waterTempRange\":\"19,23\",\"airTempRange\":\"18,26\",\"co2Levels\":\"600,750\",\"flags\":\"perennial\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Sage);
            } break;
            case Astro_TargetType_Silverbeet: {
                static const char flashStr_Silverbeet[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Silverbeet\",\"targetName\":\"Silverbeet\",\"totalGrowWeeks\":8,\"phaseDurationWeeks\":\"1,5,2\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6,7\",\"tdsRange\":\"1.8,2.3\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,23\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Silverbeet);
            } break;
            case Astro_TargetType_Spinach: {
                static const char flashStr_Spinach[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Spinach\",\"targetName\":\"Spinach\",\"totalGrowWeeks\":6,\"phaseDurationWeeks\":\"1,4,1\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6,7\",\"tdsRange\":\"1.8,2.3\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,23\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Spinach);
            } break;
            case Astro_TargetType_Squash: {
                static const char flashStr_Squash[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Squash\",\"targetName\":\"Squash\",\"totalGrowWeeks\":13,\"phaseDurationWeeks\":\"1,7,5\",\"dailyLightHours\":\"18,16,14\",\"phRange\":\"5,6.5\",\"tdsRange\":\"1.8,2.4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,28\",\"co2Levels\":\"700,900\",\"flags\":\"large,pruning\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Squash);
            } break;
            case Astro_TargetType_Sunflower: {
                static const char flashStr_Sunflower[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Sunflower\",\"targetName\":\"Sunflower\",\"totalGrowWeeks\":12,\"phaseDurationWeeks\":\"2,6,4\",\"dailyLightHours\":\"18,16,14\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1.2,1.8\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"18,28\",\"co2Levels\":\"700,900\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Sunflower);
            } break;
            case Astro_TargetType_Strawberries: {
                static const char flashStr_Strawberries[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Strawberries\",\"targetName\":\"Strawberries\",\"totalGrowWeeks\":10,\"phaseDurationWeeks\":\"2,4,4\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6,6\",\"tdsRange\":\"1.8,2.2\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,24\",\"co2Levels\":\"600,750\",\"flags\":\"perennial\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Strawberries);
            } break;
            case Astro_TargetType_SwissChard: {
                static const char flashStr_SwissChard[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"SwissChard\",\"targetName\":\"Swiss Chard\",\"totalGrowWeeks\":8,\"phaseDurationWeeks\":\"1,5,2\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6,6.5\",\"tdsRange\":\"1.8,2.3\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,23\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_SwissChard);
            } break;
            case Astro_TargetType_Taro: {
                static const char flashStr_Taro[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Taro\",\"targetName\":\"Taro\",\"totalGrowWeeks\":40,\"phaseDurationWeeks\":\"4,24,12\",\"dailyLightHours\":\"16,14,12\",\"phRange\":\"5,5.5\",\"tdsRange\":\"2.5,3\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,28\",\"co2Levels\":\"600,750\",\"flags\":\"toxic\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Taro);
            } break;
            case Astro_TargetType_Tarragon: {
                static const char flashStr_Tarragon[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Tarragon\",\"targetName\":\"Tarragon\",\"totalGrowWeeks\":12,\"phaseDurationWeeks\":\"2,6,4\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"5.5,6.5\",\"tdsRange\":\"1,1.8\",\"nightlyFeedRate\":1,\"waterTempRange\":\"19,23\",\"airTempRange\":\"18,26\",\"co2Levels\":\"600,750\",\"flags\":\"toxic\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Tarragon);
            } break;
            case Astro_TargetType_Thyme: {
                static const char flashStr_Thyme[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Thyme\",\"targetName\":\"Thyme\",\"totalGrowWeeks\":12,\"phaseDurationWeeks\":\"2,6,4\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"5,7\",\"tdsRange\":\"0.8,1.6\",\"nightlyFeedRate\":1,\"waterTempRange\":\"19,23\",\"airTempRange\":\"18,26\",\"co2Levels\":\"600,750\",\"flags\":\"perennial\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Thyme);
            } break;
            case Astro_TargetType_Tomato: {
                static const char flashStr_Tomato[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Tomato\",\"targetName\":\"Tomato\",\"totalGrowWeeks\":16,\"phaseDurationWeeks\":\"5,5,6\",\"dailyLightHours\":\"18,16,14\",\"phRange\":\"6,6.5\",\"tdsRange\":\"2,4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,28\",\"co2Levels\":\"700,900\",\"flags\":\"toxic,pruning\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Tomato);
            } break;
            case Astro_TargetType_Turnip: {
                static const char flashStr_Turnip[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Turnip\",\"targetName\":\"Turnip\",\"totalGrowWeeks\":7,\"phaseDurationWeeks\":\"1,4,2\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6,6.5\",\"tdsRange\":\"1.8,2.4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,23\",\"co2Levels\":\"600,700\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Turnip);
            } break;
            case Astro_TargetType_Watercress: {
                static const char flashStr_Watercress[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Watercress\",\"targetName\":\"Watercress\",\"totalGrowWeeks\":8,\"phaseDurationWeeks\":\"1,4,3\",\"dailyLightHours\":\"16,14,14\",\"phRange\":\"6.5,6.8\",\"tdsRange\":\"1.5,2\",\"nightlyFeedRate\":1,\"waterTempRange\":\"18,22\",\"airTempRange\":\"16,23\",\"co2Levels\":\"600,700\",\"flags\":\"perennial,toxic\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Watercress);
            } break;
            case Astro_TargetType_Watermelon: {
                static const char flashStr_Watermelon[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Watermelon\",\"targetName\":\"Watermelon\",\"totalGrowWeeks\":17,\"phaseDurationWeeks\":\"4,6,7\",\"dailyLightHours\":\"18,16,14\",\"phRange\":\"5.8,5.8\",\"tdsRange\":\"1.5,2.4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,28\",\"co2Levels\":\"700,900\",\"flags\":\"large\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Watermelon);
            } break;
            case Astro_TargetType_Zucchini: {
                static const char flashStr_Zucchini[] PROGMEM = {"{\"type\":\"HCLD\",\"id\":\"Zucchini\",\"targetName\":\"Zucchini\",\"totalGrowWeeks\":11,\"phaseDurationWeeks\":\"4,3,4\",\"dailyLightHours\":\"18,16,14\",\"phRange\":\"6,6\",\"tdsRange\":\"1.8,2.4\",\"nightlyFeedRate\":1,\"waterTempRange\":\"20,24\",\"airTempRange\":\"20,28\",\"co2Levels\":\"700,900\",\"flags\":\"large\"}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Zucchini);
            } break;
            default: break;
        }

        if (progmemStream.available()) { return new AstroTargetsLibraryBook(progmemStream, true); }
    }
    #endif // /ifndef ASTRO_DISABLE_BUILTIN_DATA
    return nullptr;
}
