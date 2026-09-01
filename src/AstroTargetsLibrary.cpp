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


AstroTargetsLibrary astroTargetsLib;

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
        if (iter->second->count > 1) { return false; }
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
    if (Astruino::_activeInstance) {
        for (auto iter = Astruino::_activeInstance->_objects.begin(); iter != Astruino::_activeInstance->_objects.end(); ++iter) {
            if (iter->second->isTargetType()) {
                auto target = static_pointer_cast<AstroTarget>(iter->second);
                if (target->getTargetType() == targetType) {
                    bool incCount = false;
                    if (_targetsData.find(targetType) != _targetsData.end()) {
                        _targetsData[targetType]->count++; // prevents auto-deletion of underlying data
                        incCount = true;
                    }

                    target->returnTargetsLibData(); // forces new data checkout

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
    String filename; filename.reserve(libSDTargetPrefix.length() + 6 + 1);
    filename.concat(libSDTargetPrefix);
    filename.concat('t');
    filename.concat('a');
    filename.concat('r');
    filename.concat('g');
    filename.concat('e');
    filename.concat('t');
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
            case Astro_TargetType_Sun: {
                static const char flashStr_Sun[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Sun\",\"catalogId\":\"Sun\",\"targetName\":\"Sun\",\"targetClass\":\"SolarSystem\",\"movingTarget\":true}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Sun);
            } break;
            case Astro_TargetType_Moon: {
                static const char flashStr_Moon[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Moon\",\"catalogId\":\"Moon\",\"targetName\":\"Moon\",\"targetClass\":\"SolarSystem\",\"movingTarget\":true}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Moon);
            } break;
            case Astro_TargetType_Mercury: {
                static const char flashStr_Mercury[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Mercury\",\"catalogId\":\"Mercury\",\"targetName\":\"Mercury\",\"targetClass\":\"SolarSystem\",\"movingTarget\":true}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Mercury);
            } break;
            case Astro_TargetType_Venus: {
                static const char flashStr_Venus[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Venus\",\"catalogId\":\"Venus\",\"targetName\":\"Venus\",\"targetClass\":\"SolarSystem\",\"movingTarget\":true}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Venus);
            } break;
            case Astro_TargetType_Mars: {
                static const char flashStr_Mars[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Mars\",\"catalogId\":\"Mars\",\"targetName\":\"Mars\",\"targetClass\":\"SolarSystem\",\"movingTarget\":true}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Mars);
            } break;
            case Astro_TargetType_Jupiter: {
                static const char flashStr_Jupiter[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Jupiter\",\"catalogId\":\"Jupiter\",\"targetName\":\"Jupiter\",\"targetClass\":\"SolarSystem\",\"movingTarget\":true}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Jupiter);
            } break;
            case Astro_TargetType_Saturn: {
                static const char flashStr_Saturn[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Saturn\",\"catalogId\":\"Saturn\",\"targetName\":\"Saturn\",\"targetClass\":\"SolarSystem\",\"movingTarget\":true}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Saturn);
            } break;
            case Astro_TargetType_Uranus: {
                static const char flashStr_Uranus[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Uranus\",\"catalogId\":\"Uranus\",\"targetName\":\"Uranus\",\"targetClass\":\"SolarSystem\",\"movingTarget\":true}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Uranus);
            } break;
            case Astro_TargetType_Neptune: {
                static const char flashStr_Neptune[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Neptune\",\"catalogId\":\"Neptune\",\"targetName\":\"Neptune\",\"targetClass\":\"SolarSystem\",\"movingTarget\":true}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Neptune);
            } break;
            case Astro_TargetType_M1: {
                static const char flashStr_M1[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M1\",\"catalogId\":\"M1\",\"targetName\":\"Crab Nebula\",\"targetClass\":\"Nebula\",\"rightAscensionSeconds\":20071,\"declinationArcseconds\":79252}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M1);
            } break;
            case Astro_TargetType_M2: {
                static const char flashStr_M2[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M2\",\"catalogId\":\"M2\",\"targetName\":\"Messier M2\",\"rightAscensionSeconds\":77607,\"declinationArcseconds\":-2964}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M2);
            } break;
            case Astro_TargetType_M3: {
                static const char flashStr_M3[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M3\",\"catalogId\":\"M3\",\"targetName\":\"Messier M3\",\"rightAscensionSeconds\":49331,\"declinationArcseconds\":102158}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M3);
            } break;
            case Astro_TargetType_M4: {
                static const char flashStr_M4[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M4\",\"catalogId\":\"M4\",\"targetName\":\"Messier M4\",\"rightAscensionSeconds\":59015,\"declinationArcseconds\":-95492}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M4);
            } break;
            case Astro_TargetType_M5: {
                static const char flashStr_M5[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M5\",\"catalogId\":\"M5\",\"targetName\":\"Messier M5\",\"rightAscensionSeconds\":55113,\"declinationArcseconds\":7491}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M5);
            } break;
            case Astro_TargetType_M6: {
                static const char flashStr_M6[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M6\",\"catalogId\":\"M6\",\"targetName\":\"Butterfly Cluster\",\"rightAscensionSeconds\":63600,\"declinationArcseconds\":-115980}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M6);
            } break;
            case Astro_TargetType_M7: {
                static const char flashStr_M7[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M7\",\"catalogId\":\"M7\",\"targetName\":\"Ptolemy Cluster\",\"rightAscensionSeconds\":64431,\"declinationArcseconds\":-125254}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M7);
            } break;
            case Astro_TargetType_M8: {
                static const char flashStr_M8[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M8\",\"catalogId\":\"M8\",\"targetName\":\"Lagoon Nebula\",\"targetClass\":\"Nebula\",\"rightAscensionSeconds\":65017,\"declinationArcseconds\":-87792}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M8);
            } break;
            case Astro_TargetType_M9: {
                static const char flashStr_M9[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M9\",\"catalogId\":\"M9\",\"targetName\":\"Messier M9\",\"rightAscensionSeconds\":62351,\"declinationArcseconds\":-66658}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M9);
            } break;
            case Astro_TargetType_M10: {
                static const char flashStr_M10[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M10\",\"catalogId\":\"M10\",\"targetName\":\"Messier M10\",\"rightAscensionSeconds\":61029,\"declinationArcseconds\":-14758}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M10);
            } break;
            case Astro_TargetType_M11: {
                static const char flashStr_M11[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M11\",\"catalogId\":\"M11\",\"targetName\":\"Wild Duck Cluster\",\"rightAscensionSeconds\":67866,\"declinationArcseconds\":-22560}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M11);
            } break;
            case Astro_TargetType_M12: {
                static const char flashStr_M12[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M12\",\"catalogId\":\"M12\",\"targetName\":\"Messier M12\",\"rightAscensionSeconds\":60434,\"declinationArcseconds\":-7015}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M12);
            } break;
            case Astro_TargetType_M13: {
                static const char flashStr_M13[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M13\",\"catalogId\":\"M13\",\"targetName\":\"Hercules Cluster\",\"targetClass\":\"GlobularCluster\",\"rightAscensionSeconds\":60101,\"declinationArcseconds\":131255}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M13);
            } break;
            case Astro_TargetType_M14: {
                static const char flashStr_M14[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M14\",\"catalogId\":\"M14\",\"targetName\":\"Messier M14\",\"rightAscensionSeconds\":63456,\"declinationArcseconds\":-11685}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M14);
            } break;
            case Astro_TargetType_M15: {
                static const char flashStr_M15[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M15\",\"catalogId\":\"M15\",\"targetName\":\"Messier M15\",\"rightAscensionSeconds\":77398,\"declinationArcseconds\":43801}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M15);
            } break;
            case Astro_TargetType_M16: {
                static const char flashStr_M16[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M16\",\"catalogId\":\"M16\",\"targetName\":\"Eagle Nebula\",\"targetClass\":\"OpenCluster\",\"rightAscensionSeconds\":65928,\"declinationArcseconds\":-49740}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M16);
            } break;
            case Astro_TargetType_M17: {
                static const char flashStr_M17[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M17\",\"catalogId\":\"M17\",\"targetName\":\"Omega Nebula\",\"targetClass\":\"Nebula\",\"rightAscensionSeconds\":66026,\"declinationArcseconds\":-58236}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M17);
            } break;
            case Astro_TargetType_M18: {
                static const char flashStr_M18[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M18\",\"catalogId\":\"M18\",\"targetName\":\"Messier M18\",\"rightAscensionSeconds\":65994,\"declinationArcseconds\":-61680}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M18);
            } break;
            case Astro_TargetType_M19: {
                static const char flashStr_M19[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M19\",\"catalogId\":\"M19\",\"targetName\":\"Messier M19\",\"rightAscensionSeconds\":61357,\"declinationArcseconds\":-94565}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M19);
            } break;
            case Astro_TargetType_M20: {
                static const char flashStr_M20[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M20\",\"catalogId\":\"M20\",\"targetName\":\"Trifid Nebula\",\"targetClass\":\"Nebula\",\"rightAscensionSeconds\":64943,\"declinationArcseconds\":-82908}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M20);
            } break;
            case Astro_TargetType_M21: {
                static const char flashStr_M21[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M21\",\"catalogId\":\"M21\",\"targetName\":\"Messier M21\",\"rightAscensionSeconds\":65076,\"declinationArcseconds\":-81000}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M21);
            } break;
            case Astro_TargetType_M22: {
                static const char flashStr_M22[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M22\",\"catalogId\":\"M22\",\"targetName\":\"Messier M22\",\"rightAscensionSeconds\":66983,\"declinationArcseconds\":-86057}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M22);
            } break;
            case Astro_TargetType_M23: {
                static const char flashStr_M23[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M23\",\"catalogId\":\"M23\",\"targetName\":\"Messier M23\",\"rightAscensionSeconds\":64608,\"declinationArcseconds\":-68460}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M23);
            } break;
            case Astro_TargetType_M24: {
                static const char flashStr_M24[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M24\",\"catalogId\":\"M24\",\"targetName\":\"Messier M24\",\"rightAscensionSeconds\":65820,\"declinationArcseconds\":-66780}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M24);
            } break;
            case Astro_TargetType_M25: {
                static const char flashStr_M25[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M25\",\"catalogId\":\"M25\",\"targetName\":\"Messier M25\",\"rightAscensionSeconds\":66696,\"declinationArcseconds\":-69300}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M25);
            } break;
            case Astro_TargetType_M26: {
                static const char flashStr_M26[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M26\",\"catalogId\":\"M26\",\"targetName\":\"Messier M26\",\"rightAscensionSeconds\":67512,\"declinationArcseconds\":-33840}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M26);
            } break;
            case Astro_TargetType_M27: {
                static const char flashStr_M27[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M27\",\"catalogId\":\"M27\",\"targetName\":\"Dumbbell Nebula\",\"targetClass\":\"PlanetaryNebula\",\"rightAscensionSeconds\":71976,\"declinationArcseconds\":81796}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M27);
            } break;
            case Astro_TargetType_M28: {
                static const char flashStr_M28[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M28\",\"catalogId\":\"M28\",\"targetName\":\"Messier M28\",\"rightAscensionSeconds\":66272,\"declinationArcseconds\":-89531}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M28);
            } break;
            case Astro_TargetType_M29: {
                static const char flashStr_M29[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M29\",\"catalogId\":\"M29\",\"targetName\":\"Messier M29\",\"rightAscensionSeconds\":73436,\"declinationArcseconds\":138684}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M29);
            } break;
            case Astro_TargetType_M30: {
                static const char flashStr_M30[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M30\",\"catalogId\":\"M30\",\"targetName\":\"Messier M30\",\"rightAscensionSeconds\":78022,\"declinationArcseconds\":-83447}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M30);
            } break;
            case Astro_TargetType_M31: {
                static const char flashStr_M31[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M31\",\"catalogId\":\"M31\",\"targetName\":\"Andromeda Galaxy\",\"targetClass\":\"Galaxy\",\"rightAscensionSeconds\":2564,\"declinationArcseconds\":148569}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M31);
            } break;
            case Astro_TargetType_M32: {
                static const char flashStr_M32[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M32\",\"catalogId\":\"M32\",\"targetName\":\"Messier M32\",\"rightAscensionSeconds\":2561,\"declinationArcseconds\":147115}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M32);
            } break;
            case Astro_TargetType_M33: {
                static const char flashStr_M33[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M33\",\"catalogId\":\"M33\",\"targetName\":\"Triangulum Galaxy\",\"targetClass\":\"Galaxy\",\"rightAscensionSeconds\":5630,\"declinationArcseconds\":110377}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M33);
            } break;
            case Astro_TargetType_M34: {
                static const char flashStr_M34[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M34\",\"catalogId\":\"M34\",\"targetName\":\"Messier M34\",\"rightAscensionSeconds\":9726,\"declinationArcseconds\":153960}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M34);
            } break;
            case Astro_TargetType_M35: {
                static const char flashStr_M35[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M35\",\"catalogId\":\"M35\",\"targetName\":\"Messier M35\",\"rightAscensionSeconds\":22146,\"declinationArcseconds\":87660}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M35);
            } break;
            case Astro_TargetType_M36: {
                static const char flashStr_M36[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M36\",\"catalogId\":\"M36\",\"targetName\":\"Messier M36\",\"rightAscensionSeconds\":20172,\"declinationArcseconds\":122884}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M36);
            } break;
            case Astro_TargetType_M37: {
                static const char flashStr_M37[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M37\",\"catalogId\":\"M37\",\"targetName\":\"Messier M37\",\"rightAscensionSeconds\":21138,\"declinationArcseconds\":117182}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M37);
            } break;
            case Astro_TargetType_M38: {
                static const char flashStr_M38[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M38\",\"catalogId\":\"M38\",\"targetName\":\"Messier M38\",\"rightAscensionSeconds\":19722,\"declinationArcseconds\":129078}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M38);
            } break;
            case Astro_TargetType_M39: {
                static const char flashStr_M39[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M39\",\"catalogId\":\"M39\",\"targetName\":\"Messier M39\",\"rightAscensionSeconds\":77502,\"declinationArcseconds\":174360}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M39);
            } break;
            case Astro_TargetType_M40: {
                static const char flashStr_M40[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M40\",\"catalogId\":\"M40\",\"targetName\":\"Messier M40\",\"rightAscensionSeconds\":44532,\"declinationArcseconds\":209099}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M40);
            } break;
            case Astro_TargetType_M41: {
                static const char flashStr_M41[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M41\",\"catalogId\":\"M41\",\"targetName\":\"Messier M41\",\"rightAscensionSeconds\":24360,\"declinationArcseconds\":-74760}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M41);
            } break;
            case Astro_TargetType_M42: {
                static const char flashStr_M42[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M42\",\"catalogId\":\"M42\",\"targetName\":\"Orion Nebula\",\"targetClass\":\"Nebula\",\"rightAscensionSeconds\":20117,\"declinationArcseconds\":-19408}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M42);
            } break;
            case Astro_TargetType_M43: {
                static const char flashStr_M43[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M43\",\"catalogId\":\"M43\",\"targetName\":\"Messier M43\",\"rightAscensionSeconds\":20136,\"declinationArcseconds\":-18960}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M43);
            } break;
            case Astro_TargetType_M44: {
                static const char flashStr_M44[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M44\",\"catalogId\":\"M44\",\"targetName\":\"Beehive Cluster\",\"targetClass\":\"OpenCluster\",\"rightAscensionSeconds\":31224,\"declinationArcseconds\":71940}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M44);
            } break;
            case Astro_TargetType_M45: {
                static const char flashStr_M45[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M45\",\"catalogId\":\"M45\",\"targetName\":\"Pleiades\",\"targetClass\":\"OpenCluster\",\"rightAscensionSeconds\":13644,\"declinationArcseconds\":86820}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M45);
            } break;
            case Astro_TargetType_M46: {
                static const char flashStr_M46[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M46\",\"catalogId\":\"M46\",\"targetName\":\"Messier M46\",\"rightAscensionSeconds\":27708,\"declinationArcseconds\":-53340}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M46);
            } break;
            case Astro_TargetType_M47: {
                static const char flashStr_M47[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M47\",\"catalogId\":\"M47\",\"targetName\":\"Messier M47\",\"rightAscensionSeconds\":27396,\"declinationArcseconds\":-52200}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M47);
            } break;
            case Astro_TargetType_M48: {
                static const char flashStr_M48[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M48\",\"catalogId\":\"M48\",\"targetName\":\"Messier M48\",\"rightAscensionSeconds\":29622,\"declinationArcseconds\":-20700}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M48);
            } break;
            case Astro_TargetType_M49: {
                static const char flashStr_M49[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M49\",\"catalogId\":\"M49\",\"targetName\":\"Messier M49\",\"rightAscensionSeconds\":44986,\"declinationArcseconds\":28802}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M49);
            } break;
            case Astro_TargetType_M50: {
                static const char flashStr_M50[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M50\",\"catalogId\":\"M50\",\"targetName\":\"Messier M50\",\"rightAscensionSeconds\":25392,\"declinationArcseconds\":-30000}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M50);
            } break;
            case Astro_TargetType_M51: {
                static const char flashStr_M51[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M51\",\"catalogId\":\"M51\",\"targetName\":\"Whirlpool Galaxy\",\"targetClass\":\"Galaxy\",\"rightAscensionSeconds\":48592,\"declinationArcseconds\":169903}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M51);
            } break;
            case Astro_TargetType_M52: {
                static const char flashStr_M52[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M52\",\"catalogId\":\"M52\",\"targetName\":\"Messier M52\",\"rightAscensionSeconds\":84252,\"declinationArcseconds\":221700}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M52);
            } break;
            case Astro_TargetType_M53: {
                static const char flashStr_M53[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M53\",\"catalogId\":\"M53\",\"targetName\":\"Messier M53\",\"rightAscensionSeconds\":47575,\"declinationArcseconds\":65405}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M53);
            } break;
            case Astro_TargetType_M54: {
                static const char flashStr_M54[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M54\",\"catalogId\":\"M54\",\"targetName\":\"Messier M54\",\"rightAscensionSeconds\":68103,\"declinationArcseconds\":-109727}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M54);
            } break;
            case Astro_TargetType_M55: {
                static const char flashStr_M55[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M55\",\"catalogId\":\"M55\",\"targetName\":\"Messier M55\",\"rightAscensionSeconds\":70799,\"declinationArcseconds\":-111473}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M55);
            } break;
            case Astro_TargetType_M56: {
                static const char flashStr_M56[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M56\",\"catalogId\":\"M56\",\"targetName\":\"Messier M56\",\"rightAscensionSeconds\":69395,\"declinationArcseconds\":108660}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M56);
            } break;
            case Astro_TargetType_M57: {
                static const char flashStr_M57[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M57\",\"catalogId\":\"M57\",\"targetName\":\"Ring Nebula\",\"targetClass\":\"PlanetaryNebula\",\"rightAscensionSeconds\":68015,\"declinationArcseconds\":118905}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M57);
            } break;
            case Astro_TargetType_M58: {
                static const char flashStr_M58[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M58\",\"catalogId\":\"M58\",\"targetName\":\"Messier M58\",\"rightAscensionSeconds\":45463,\"declinationArcseconds\":42545}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M58);
            } break;
            case Astro_TargetType_M59: {
                static const char flashStr_M59[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M59\",\"catalogId\":\"M59\",\"targetName\":\"Messier M59\",\"rightAscensionSeconds\":45722,\"declinationArcseconds\":41929}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M59);
            } break;
            case Astro_TargetType_M60: {
                static const char flashStr_M60[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M60\",\"catalogId\":\"M60\",\"targetName\":\"Messier M60\",\"rightAscensionSeconds\":45819,\"declinationArcseconds\":41589}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M60);
            } break;
            case Astro_TargetType_M61: {
                static const char flashStr_M61[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M61\",\"catalogId\":\"M61\",\"targetName\":\"Messier M61\",\"rightAscensionSeconds\":44514,\"declinationArcseconds\":16105}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M61);
            } break;
            case Astro_TargetType_M62: {
                static const char flashStr_M62[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M62\",\"catalogId\":\"M62\",\"targetName\":\"Messier M62\",\"rightAscensionSeconds\":61272,\"declinationArcseconds\":-108404}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M62);
            } break;
            case Astro_TargetType_M63: {
                static const char flashStr_M63[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M63\",\"catalogId\":\"M63\",\"targetName\":\"Sunflower Galaxy\",\"targetClass\":\"Galaxy\",\"rightAscensionSeconds\":47749,\"declinationArcseconds\":151305}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M63);
            } break;
            case Astro_TargetType_M64: {
                static const char flashStr_M64[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M64\",\"catalogId\":\"M64\",\"targetName\":\"Black Eye Galaxy\",\"targetClass\":\"Galaxy\",\"rightAscensionSeconds\":46603,\"declinationArcseconds\":78058}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M64);
            } break;
            case Astro_TargetType_M65: {
                static const char flashStr_M65[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M65\",\"catalogId\":\"M65\",\"targetName\":\"Messier M65\",\"rightAscensionSeconds\":40735,\"declinationArcseconds\":47132}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M65);
            } break;
            case Astro_TargetType_M66: {
                static const char flashStr_M66[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M66\",\"catalogId\":\"M66\",\"targetName\":\"Messier M66\",\"rightAscensionSeconds\":40815,\"declinationArcseconds\":46770}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M66);
            } break;
            case Astro_TargetType_M67: {
                static const char flashStr_M67[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M67\",\"catalogId\":\"M67\",\"targetName\":\"Messier M67\",\"rightAscensionSeconds\":31878,\"declinationArcseconds\":42540}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M67);
            } break;
            case Astro_TargetType_M68: {
                static const char flashStr_M68[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M68\",\"catalogId\":\"M68\",\"targetName\":\"Messier M68\",\"rightAscensionSeconds\":45567,\"declinationArcseconds\":-96279}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M68);
            } break;
            case Astro_TargetType_M69: {
                static const char flashStr_M69[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M69\",\"catalogId\":\"M69\",\"targetName\":\"Messier M69\",\"rightAscensionSeconds\":66683,\"declinationArcseconds\":-116453}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M69);
            } break;
            case Astro_TargetType_M70: {
                static const char flashStr_M70[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M70\",\"catalogId\":\"M70\",\"targetName\":\"Messier M70\",\"rightAscensionSeconds\":67392,\"declinationArcseconds\":-116252}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M70);
            } break;
            case Astro_TargetType_M71: {
                static const char flashStr_M71[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M71\",\"catalogId\":\"M71\",\"targetName\":\"Messier M71\",\"rightAscensionSeconds\":71626,\"declinationArcseconds\":67605}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M71);
            } break;
            case Astro_TargetType_M72: {
                static const char flashStr_M72[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M72\",\"catalogId\":\"M72\",\"targetName\":\"Messier M72\",\"rightAscensionSeconds\":75207,\"declinationArcseconds\":-45134}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M72);
            } break;
            case Astro_TargetType_M73: {
                static const char flashStr_M73[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M73\",\"catalogId\":\"M73\",\"targetName\":\"Messier M73\",\"rightAscensionSeconds\":75534,\"declinationArcseconds\":-45480}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M73);
            } break;
            case Astro_TargetType_M74: {
                static const char flashStr_M74[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M74\",\"catalogId\":\"M74\",\"targetName\":\"Messier M74\",\"rightAscensionSeconds\":5801,\"declinationArcseconds\":56821}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M74);
            } break;
            case Astro_TargetType_M75: {
                static const char flashStr_M75[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M75\",\"catalogId\":\"M75\",\"targetName\":\"Messier M75\",\"rightAscensionSeconds\":72364,\"declinationArcseconds\":-78916}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M75);
            } break;
            case Astro_TargetType_M76: {
                static const char flashStr_M76[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M76\",\"catalogId\":\"M76\",\"targetName\":\"Little Dumbbell Nebula\",\"targetClass\":\"PlanetaryNebula\",\"rightAscensionSeconds\":6144,\"declinationArcseconds\":185671}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M76);
            } break;
            case Astro_TargetType_M77: {
                static const char flashStr_M77[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M77\",\"catalogId\":\"M77\",\"targetName\":\"Messier M77\",\"rightAscensionSeconds\":9760,\"declinationArcseconds\":-48}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M77);
            } break;
            case Astro_TargetType_M78: {
                static const char flashStr_M78[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M78\",\"catalogId\":\"M78\",\"targetName\":\"Messier M78\",\"rightAscensionSeconds\":20806,\"declinationArcseconds\":50}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M78);
            } break;
            case Astro_TargetType_M79: {
                static const char flashStr_M79[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M79\",\"catalogId\":\"M79\",\"targetName\":\"Messier M79\",\"rightAscensionSeconds\":19450,\"declinationArcseconds\":-88287}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M79);
            } break;
            case Astro_TargetType_M80: {
                static const char flashStr_M80[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M80\",\"catalogId\":\"M80\",\"targetName\":\"Messier M80\",\"rightAscensionSeconds\":58622,\"declinationArcseconds\":-82720}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M80);
            } break;
            case Astro_TargetType_M81: {
                static const char flashStr_M81[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M81\",\"catalogId\":\"M81\",\"targetName\":\"Bode Galaxy\",\"targetClass\":\"Galaxy\",\"rightAscensionSeconds\":35733,\"declinationArcseconds\":248635}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M81);
            } break;
            case Astro_TargetType_M82: {
                static const char flashStr_M82[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M82\",\"catalogId\":\"M82\",\"targetName\":\"Cigar Galaxy\",\"targetClass\":\"Galaxy\",\"rightAscensionSeconds\":35752,\"declinationArcseconds\":250847}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M82);
            } break;
            case Astro_TargetType_M83: {
                static const char flashStr_M83[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M83\",\"catalogId\":\"M83\",\"targetName\":\"Southern Pinwheel Galaxy\",\"targetClass\":\"Galaxy\",\"rightAscensionSeconds\":49020,\"declinationArcseconds\":-107517}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M83);
            } break;
            case Astro_TargetType_M84: {
                static const char flashStr_M84[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M84\",\"catalogId\":\"M84\",\"targetName\":\"Messier M84\",\"rightAscensionSeconds\":44703,\"declinationArcseconds\":46393}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M84);
            } break;
            case Astro_TargetType_M85: {
                static const char flashStr_M85[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M85\",\"catalogId\":\"M85\",\"targetName\":\"Messier M85\",\"rightAscensionSeconds\":44724,\"declinationArcseconds\":65488}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M85);
            } break;
            case Astro_TargetType_M86: {
                static const char flashStr_M86[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M86\",\"catalogId\":\"M86\",\"targetName\":\"Messier M86\",\"rightAscensionSeconds\":44771,\"declinationArcseconds\":46606}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M86);
            } break;
            case Astro_TargetType_M87: {
                static const char flashStr_M87[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M87\",\"catalogId\":\"M87\",\"targetName\":\"Messier M87\",\"rightAscensionSeconds\":45049,\"declinationArcseconds\":44608}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M87);
            } break;
            case Astro_TargetType_M88: {
                static const char flashStr_M88[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M88\",\"catalogId\":\"M88\",\"targetName\":\"Messier M88\",\"rightAscensionSeconds\":45119,\"declinationArcseconds\":51914}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M88);
            } break;
            case Astro_TargetType_M89: {
                static const char flashStr_M89[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M89\",\"catalogId\":\"M89\",\"targetName\":\"Messier M89\",\"rightAscensionSeconds\":45339,\"declinationArcseconds\":45203}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M89);
            } break;
            case Astro_TargetType_M90: {
                static const char flashStr_M90[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M90\",\"catalogId\":\"M90\",\"targetName\":\"Messier M90\",\"rightAscensionSeconds\":45409,\"declinationArcseconds\":47386}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M90);
            } break;
            case Astro_TargetType_M91: {
                static const char flashStr_M91[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M91\",\"catalogId\":\"M91\",\"targetName\":\"Messier M91\",\"rightAscensionSeconds\":45326,\"declinationArcseconds\":52187}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M91);
            } break;
            case Astro_TargetType_M92: {
                static const char flashStr_M92[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M92\",\"catalogId\":\"M92\",\"targetName\":\"Messier M92\",\"rightAscensionSeconds\":62227,\"declinationArcseconds\":155289}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M92);
            } break;
            case Astro_TargetType_M93: {
                static const char flashStr_M93[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M93\",\"catalogId\":\"M93\",\"targetName\":\"Messier M93\",\"rightAscensionSeconds\":27876,\"declinationArcseconds\":-85920}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M93);
            } break;
            case Astro_TargetType_M94: {
                static const char flashStr_M94[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M94\",\"catalogId\":\"M94\",\"targetName\":\"Messier M94\",\"rightAscensionSeconds\":46253,\"declinationArcseconds\":148034}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M94);
            } break;
            case Astro_TargetType_M95: {
                static const char flashStr_M95[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M95\",\"catalogId\":\"M95\",\"targetName\":\"Messier M95\",\"rightAscensionSeconds\":38637,\"declinationArcseconds\":42134}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M95);
            } break;
            case Astro_TargetType_M96: {
                static const char flashStr_M96[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M96\",\"catalogId\":\"M96\",\"targetName\":\"Messier M96\",\"rightAscensionSeconds\":38805,\"declinationArcseconds\":42552}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M96);
            } break;
            case Astro_TargetType_M97: {
                static const char flashStr_M97[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M97\",\"catalogId\":\"M97\",\"targetName\":\"Owl Nebula\",\"targetClass\":\"PlanetaryNebula\",\"rightAscensionSeconds\":40487,\"declinationArcseconds\":198068}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M97);
            } break;
            case Astro_TargetType_M98: {
                static const char flashStr_M98[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M98\",\"catalogId\":\"M98\",\"targetName\":\"Messier M98\",\"rightAscensionSeconds\":44028,\"declinationArcseconds\":53642}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M98);
            } break;
            case Astro_TargetType_M99: {
                static const char flashStr_M99[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M99\",\"catalogId\":\"M99\",\"targetName\":\"Messier M99\",\"rightAscensionSeconds\":44329,\"declinationArcseconds\":51899}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M99);
            } break;
            case Astro_TargetType_M100: {
                static const char flashStr_M100[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M100\",\"catalogId\":\"M100\",\"targetName\":\"Messier M100\",\"rightAscensionSeconds\":44574,\"declinationArcseconds\":56961}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M100);
            } break;
            case Astro_TargetType_M101: {
                static const char flashStr_M101[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M101\",\"catalogId\":\"M101\",\"targetName\":\"Pinwheel Galaxy\",\"targetClass\":\"Galaxy\",\"rightAscensionSeconds\":50592,\"declinationArcseconds\":195657}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M101);
            } break;
            case Astro_TargetType_M102: {
                static const char flashStr_M102[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M102\",\"catalogId\":\"M102\",\"targetName\":\"Messier M102\",\"rightAscensionSeconds\":54389,\"declinationArcseconds\":200748}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M102);
            } break;
            case Astro_TargetType_M103: {
                static const char flashStr_M103[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M103\",\"catalogId\":\"M103\",\"targetName\":\"Messier M103\",\"rightAscensionSeconds\":5592,\"declinationArcseconds\":218520}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M103);
            } break;
            case Astro_TargetType_M104: {
                static const char flashStr_M104[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M104\",\"catalogId\":\"M104\",\"targetName\":\"Sombrero Galaxy\",\"targetClass\":\"Galaxy\",\"rightAscensionSeconds\":45599,\"declinationArcseconds\":-41843}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M104);
            } break;
            case Astro_TargetType_M105: {
                static const char flashStr_M105[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M105\",\"catalogId\":\"M105\",\"targetName\":\"Messier M105\",\"rightAscensionSeconds\":38869,\"declinationArcseconds\":45294}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M105);
            } break;
            case Astro_TargetType_M106: {
                static const char flashStr_M106[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M106\",\"catalogId\":\"M106\",\"targetName\":\"Messier M106\",\"rightAscensionSeconds\":44337,\"declinationArcseconds\":170294}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M106);
            } break;
            case Astro_TargetType_M107: {
                static const char flashStr_M107[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M107\",\"catalogId\":\"M107\",\"targetName\":\"Messier M107\",\"rightAscensionSeconds\":59551,\"declinationArcseconds\":-46994}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M107);
            } break;
            case Astro_TargetType_M108: {
                static const char flashStr_M108[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M108\",\"catalogId\":\"M108\",\"targetName\":\"Messier M108\",\"rightAscensionSeconds\":40291,\"declinationArcseconds\":200427}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M108);
            } break;
            case Astro_TargetType_M109: {
                static const char flashStr_M109[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M109\",\"catalogId\":\"M109\",\"targetName\":\"Messier M109\",\"rightAscensionSeconds\":43056,\"declinationArcseconds\":192148}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M109);
            } break;
            case Astro_TargetType_M110: {
                static const char flashStr_M110[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"M110\",\"catalogId\":\"M110\",\"targetName\":\"Messier M110\",\"rightAscensionSeconds\":2422,\"declinationArcseconds\":150067}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_M110);
            } break;
            case Astro_TargetType_Sirius: {
                static const char flashStr_Sirius[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Sirius\",\"catalogId\":\"Sirius\",\"targetName\":\"Sirius\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":24309,\"declinationArcseconds\":-60178}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Sirius);
            } break;
            case Astro_TargetType_Canopus: {
                static const char flashStr_Canopus[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Canopus\",\"catalogId\":\"Canopus\",\"targetName\":\"Canopus\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":23037,\"declinationArcseconds\":-189705}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Canopus);
            } break;
            case Astro_TargetType_Arcturus: {
                static const char flashStr_Arcturus[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Arcturus\",\"catalogId\":\"Arcturus\",\"targetName\":\"Arcturus\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":51340,\"declinationArcseconds\":69057}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Arcturus);
            } break;
            case Astro_TargetType_Vega: {
                static const char flashStr_Vega[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Vega\",\"catalogId\":\"Vega\",\"targetName\":\"Vega\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":67016,\"declinationArcseconds\":139621}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Vega);
            } break;
            case Astro_TargetType_Capella: {
                static const char flashStr_Capella[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Capella\",\"catalogId\":\"Capella\",\"targetName\":\"Capella\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":19001,\"declinationArcseconds\":165593}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Capella);
            } break;
            case Astro_TargetType_RigelKentaurus: {
                static const char flashStr_RigelKentaurus[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"RigelKentaurus\",\"catalogId\":\"Rigel Kentaurus\",\"targetName\":\"Rigel Kentaurus\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":52776,\"declinationArcseconds\":-219007}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_RigelKentaurus);
            } break;
            case Astro_TargetType_Procyon: {
                static const char flashStr_Procyon[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Procyon\",\"catalogId\":\"Procyon\",\"targetName\":\"Procyon\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":27558,\"declinationArcseconds\":18810}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Procyon);
            } break;
            case Astro_TargetType_Betelgeuse: {
                static const char flashStr_Betelgeuse[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Betelgeuse\",\"catalogId\":\"Betelgeuse\",\"targetName\":\"Betelgeuse\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":21310,\"declinationArcseconds\":26665}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Betelgeuse);
            } break;
            case Astro_TargetType_Achernar: {
                static const char flashStr_Achernar[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Achernar\",\"catalogId\":\"Achernar\",\"targetName\":\"Achernar\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":5863,\"declinationArcseconds\":-206052}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Achernar);
            } break;
            case Astro_TargetType_Hadar: {
                static const char flashStr_Hadar[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Hadar\",\"catalogId\":\"Hadar\",\"targetName\":\"Hadar\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":50629,\"declinationArcseconds\":-217343}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Hadar);
            } break;
            case Astro_TargetType_Altair: {
                static const char flashStr_Altair[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Altair\",\"catalogId\":\"Altair\",\"targetName\":\"Altair\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":71447,\"declinationArcseconds\":31926}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Altair);
            } break;
            case Astro_TargetType_Acrux: {
                static const char flashStr_Acrux[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Acrux\",\"catalogId\":\"Acrux\",\"targetName\":\"Acrux\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":44796,\"declinationArcseconds\":-227157}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Acrux);
            } break;
            case Astro_TargetType_Aldebaran: {
                static const char flashStr_Aldebaran[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Aldebaran\",\"catalogId\":\"Aldebaran\",\"targetName\":\"Aldebaran\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":16555,\"declinationArcseconds\":59433}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Aldebaran);
            } break;
            case Astro_TargetType_Spica: {
                static const char flashStr_Spica[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Spica\",\"catalogId\":\"Spica\",\"targetName\":\"Spica\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":48312,\"declinationArcseconds\":-40181}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Spica);
            } break;
            case Astro_TargetType_Antares: {
                static const char flashStr_Antares[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Antares\",\"catalogId\":\"Antares\",\"targetName\":\"Antares\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":59364,\"declinationArcseconds\":-95155}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Antares);
            } break;
            case Astro_TargetType_Pollux: {
                static const char flashStr_Pollux[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Pollux\",\"catalogId\":\"Pollux\",\"targetName\":\"Pollux\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":27919,\"declinationArcseconds\":100894}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Pollux);
            } break;
            case Astro_TargetType_Fomalhaut: {
                static const char flashStr_Fomalhaut[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Fomalhaut\",\"catalogId\":\"Fomalhaut\",\"targetName\":\"Fomalhaut\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":82659,\"declinationArcseconds\":-106640}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Fomalhaut);
            } break;
            case Astro_TargetType_Deneb: {
                static const char flashStr_Deneb[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Deneb\",\"catalogId\":\"Deneb\",\"targetName\":\"Deneb\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":74486,\"declinationArcseconds\":163009}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Deneb);
            } break;
            case Astro_TargetType_Regulus: {
                static const char flashStr_Regulus[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Regulus\",\"catalogId\":\"Regulus\",\"targetName\":\"Regulus\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":36502,\"declinationArcseconds\":43082}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Regulus);
            } break;
            case Astro_TargetType_Polaris: {
                static const char flashStr_Polaris[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Polaris\",\"catalogId\":\"Polaris\",\"targetName\":\"Polaris\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":9109,\"declinationArcseconds\":321351}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Polaris);
            } break;
            case Astro_TargetType_Castor: {
                static const char flashStr_Castor[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Castor\",\"catalogId\":\"Castor\",\"targetName\":\"Castor\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":27276,\"declinationArcseconds\":114798}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Castor);
            } break;
            case Astro_TargetType_Bellatrix: {
                static const char flashStr_Bellatrix[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Bellatrix\",\"catalogId\":\"Bellatrix\",\"targetName\":\"Bellatrix\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":19508,\"declinationArcseconds\":22859}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Bellatrix);
            } break;
            case Astro_TargetType_Alnilam: {
                static const char flashStr_Alnilam[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Alnilam\",\"catalogId\":\"Alnilam\",\"targetName\":\"Alnilam\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":20173,\"declinationArcseconds\":-4327}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Alnilam);
            } break;
            case Astro_TargetType_Alnitak: {
                static const char flashStr_Alnitak[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Alnitak\",\"catalogId\":\"Alnitak\",\"targetName\":\"Alnitak\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":20446,\"declinationArcseconds\":-6994}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Alnitak);
            } break;
            case Astro_TargetType_Mizar: {
                static const char flashStr_Mizar[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Mizar\",\"catalogId\":\"Mizar\",\"targetName\":\"Mizar\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":48236,\"declinationArcseconds\":197731}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Mizar);
            } break;
            case Astro_TargetType_Dubhe: {
                static const char flashStr_Dubhe[] PROGMEM = {"{\"type\":\"ATLD\",\"id\":\"Dubhe\",\"catalogId\":\"Dubhe\",\"targetName\":\"Dubhe\",\"targetClass\":\"Star\",\"rightAscensionSeconds\":39824,\"declinationArcseconds\":222303}"};
                progmemStream = AstroPROGMEMStream((uintptr_t)flashStr_Dubhe);
            } break;
            default: break;
        }

        if (progmemStream.available()) { return new AstroTargetsLibraryBook(progmemStream, true); }
    }
    #endif // /ifndef ASTRO_DISABLE_BUILTIN_DATA
    return nullptr;
}
