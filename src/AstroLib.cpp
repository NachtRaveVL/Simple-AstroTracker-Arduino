/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Targets Library
*/

#include "AstroLib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __AVR__
#include <avr/pgmspace.h>
#endif

struct AstroPackedTarget {
    int32_t rightAscensionSeconds;
    int32_t declinationArcseconds;
    int16_t magnitudeCenti;
    uint16_t nameOffset;
    Astro_TargetClass targetClass;
};

static const char astroTargetStringPool[] PROGMEM = "\0Crab Nebula\0Butterfly Cluster\0Ptolemy Cluster\0Lagoon Nebula\0Wild Duck Cluster\0Hercules Cluster\0Eagle Nebula\0Omega Nebula\0Trifid Nebula\0Dumbbell Nebula\0Andromeda Galaxy\0Triangulum Galaxy\0Orion Nebula\0Beehive Cluster\0Pleiades\0Whirlpool Galaxy\0Ring Nebula\0Sunflower Galaxy\0Black Eye Galaxy\0Little Dumbbell Nebula\0Bode Galaxy\0Cigar Galaxy\0Southern Pinwheel Galaxy\0Owl Nebula\0Pinwheel Galaxy\0Sombrero Galaxy\0Sirius\0Canopus\0Arcturus\0Vega\0Capella\0Rigel Kentaurus\0Procyon\0Betelgeuse\0Achernar\0Hadar\0Altair\0Acrux\0Aldebaran\0Spica\0Antares\0Pollux\0Fomalhaut\0Deneb\0Regulus\0Polaris\0Castor\0Bellatrix\0Alnilam\0Alnitak\0Mizar\0Dubhe\0";

static const AstroPackedTarget astroFixedTargets[] PROGMEM = {
    {20071, 79252, 32767, 1, Astro_TargetClass_Nebula}, // M1
    {77607, -2964, 32767, 0, Astro_TargetClass_Unknown}, // M2
    {49331, 102158, 32767, 0, Astro_TargetClass_Unknown}, // M3
    {59015, -95492, 32767, 0, Astro_TargetClass_Unknown}, // M4
    {55113, 7491, 32767, 0, Astro_TargetClass_Unknown}, // M5
    {63600, -115980, 32767, 13, Astro_TargetClass_Unknown}, // M6
    {64431, -125254, 32767, 31, Astro_TargetClass_Unknown}, // M7
    {65017, -87792, 32767, 47, Astro_TargetClass_Nebula}, // M8
    {62351, -66658, 32767, 0, Astro_TargetClass_Unknown}, // M9
    {61029, -14758, 32767, 0, Astro_TargetClass_Unknown}, // M10
    {67866, -22560, 32767, 61, Astro_TargetClass_Unknown}, // M11
    {60434, -7015, 32767, 0, Astro_TargetClass_Unknown}, // M12
    {60101, 131255, 32767, 79, Astro_TargetClass_GlobularCluster}, // M13
    {63456, -11685, 32767, 0, Astro_TargetClass_Unknown}, // M14
    {77398, 43801, 32767, 0, Astro_TargetClass_Unknown}, // M15
    {65928, -49740, 32767, 96, Astro_TargetClass_OpenCluster}, // M16
    {66026, -58236, 32767, 109, Astro_TargetClass_Nebula}, // M17
    {65994, -61680, 32767, 0, Astro_TargetClass_Unknown}, // M18
    {61357, -94565, 32767, 0, Astro_TargetClass_Unknown}, // M19
    {64943, -82908, 32767, 122, Astro_TargetClass_Nebula}, // M20
    {65076, -81000, 32767, 0, Astro_TargetClass_Unknown}, // M21
    {66983, -86057, 32767, 0, Astro_TargetClass_Unknown}, // M22
    {64608, -68460, 32767, 0, Astro_TargetClass_Unknown}, // M23
    {65820, -66780, 32767, 0, Astro_TargetClass_Unknown}, // M24
    {66696, -69300, 32767, 0, Astro_TargetClass_Unknown}, // M25
    {67512, -33840, 32767, 0, Astro_TargetClass_Unknown}, // M26
    {71976, 81796, 32767, 136, Astro_TargetClass_PlanetaryNebula}, // M27
    {66272, -89531, 32767, 0, Astro_TargetClass_Unknown}, // M28
    {73436, 138684, 32767, 0, Astro_TargetClass_Unknown}, // M29
    {78022, -83447, 32767, 0, Astro_TargetClass_Unknown}, // M30
    {2564, 148569, 32767, 152, Astro_TargetClass_Galaxy}, // M31
    {2561, 147115, 32767, 0, Astro_TargetClass_Unknown}, // M32
    {5630, 110377, 32767, 169, Astro_TargetClass_Galaxy}, // M33
    {9726, 153960, 32767, 0, Astro_TargetClass_Unknown}, // M34
    {22146, 87660, 32767, 0, Astro_TargetClass_Unknown}, // M35
    {20172, 122884, 32767, 0, Astro_TargetClass_Unknown}, // M36
    {21138, 117182, 32767, 0, Astro_TargetClass_Unknown}, // M37
    {19722, 129078, 32767, 0, Astro_TargetClass_Unknown}, // M38
    {77502, 174360, 32767, 0, Astro_TargetClass_Unknown}, // M39
    {44532, 209099, 32767, 0, Astro_TargetClass_Unknown}, // M40
    {24360, -74760, 32767, 0, Astro_TargetClass_Unknown}, // M41
    {20117, -19408, 32767, 187, Astro_TargetClass_Nebula}, // M42
    {20136, -18960, 32767, 0, Astro_TargetClass_Unknown}, // M43
    {31224, 71940, 32767, 200, Astro_TargetClass_OpenCluster}, // M44
    {13644, 86820, 32767, 216, Astro_TargetClass_OpenCluster}, // M45
    {27708, -53340, 32767, 0, Astro_TargetClass_Unknown}, // M46
    {27396, -52200, 32767, 0, Astro_TargetClass_Unknown}, // M47
    {29622, -20700, 32767, 0, Astro_TargetClass_Unknown}, // M48
    {44986, 28802, 32767, 0, Astro_TargetClass_Unknown}, // M49
    {25392, -30000, 32767, 0, Astro_TargetClass_Unknown}, // M50
    {48592, 169903, 32767, 225, Astro_TargetClass_Galaxy}, // M51
    {84252, 221700, 32767, 0, Astro_TargetClass_Unknown}, // M52
    {47575, 65405, 32767, 0, Astro_TargetClass_Unknown}, // M53
    {68103, -109727, 32767, 0, Astro_TargetClass_Unknown}, // M54
    {70799, -111473, 32767, 0, Astro_TargetClass_Unknown}, // M55
    {69395, 108660, 32767, 0, Astro_TargetClass_Unknown}, // M56
    {68015, 118905, 32767, 242, Astro_TargetClass_PlanetaryNebula}, // M57
    {45463, 42545, 32767, 0, Astro_TargetClass_Unknown}, // M58
    {45722, 41929, 32767, 0, Astro_TargetClass_Unknown}, // M59
    {45819, 41589, 32767, 0, Astro_TargetClass_Unknown}, // M60
    {44514, 16105, 32767, 0, Astro_TargetClass_Unknown}, // M61
    {61272, -108404, 32767, 0, Astro_TargetClass_Unknown}, // M62
    {47749, 151305, 32767, 254, Astro_TargetClass_Galaxy}, // M63
    {46603, 78058, 32767, 271, Astro_TargetClass_Galaxy}, // M64
    {40735, 47132, 32767, 0, Astro_TargetClass_Unknown}, // M65
    {40815, 46770, 32767, 0, Astro_TargetClass_Unknown}, // M66
    {31878, 42540, 32767, 0, Astro_TargetClass_Unknown}, // M67
    {45567, -96279, 32767, 0, Astro_TargetClass_Unknown}, // M68
    {66683, -116453, 32767, 0, Astro_TargetClass_Unknown}, // M69
    {67392, -116252, 32767, 0, Astro_TargetClass_Unknown}, // M70
    {71626, 67605, 32767, 0, Astro_TargetClass_Unknown}, // M71
    {75207, -45134, 32767, 0, Astro_TargetClass_Unknown}, // M72
    {75534, -45480, 32767, 0, Astro_TargetClass_Unknown}, // M73
    {5801, 56821, 32767, 0, Astro_TargetClass_Unknown}, // M74
    {72364, -78916, 32767, 0, Astro_TargetClass_Unknown}, // M75
    {6144, 185671, 32767, 288, Astro_TargetClass_PlanetaryNebula}, // M76
    {9760, -48, 32767, 0, Astro_TargetClass_Unknown}, // M77
    {20806, 50, 32767, 0, Astro_TargetClass_Unknown}, // M78
    {19450, -88287, 32767, 0, Astro_TargetClass_Unknown}, // M79
    {58622, -82720, 32767, 0, Astro_TargetClass_Unknown}, // M80
    {35733, 248635, 32767, 311, Astro_TargetClass_Galaxy}, // M81
    {35752, 250847, 32767, 323, Astro_TargetClass_Galaxy}, // M82
    {49020, -107517, 32767, 336, Astro_TargetClass_Galaxy}, // M83
    {44703, 46393, 32767, 0, Astro_TargetClass_Unknown}, // M84
    {44724, 65488, 32767, 0, Astro_TargetClass_Unknown}, // M85
    {44771, 46606, 32767, 0, Astro_TargetClass_Unknown}, // M86
    {45049, 44608, 32767, 0, Astro_TargetClass_Unknown}, // M87
    {45119, 51914, 32767, 0, Astro_TargetClass_Unknown}, // M88
    {45339, 45203, 32767, 0, Astro_TargetClass_Unknown}, // M89
    {45409, 47386, 32767, 0, Astro_TargetClass_Unknown}, // M90
    {45326, 52187, 32767, 0, Astro_TargetClass_Unknown}, // M91
    {62227, 155289, 32767, 0, Astro_TargetClass_Unknown}, // M92
    {27876, -85920, 32767, 0, Astro_TargetClass_Unknown}, // M93
    {46253, 148034, 32767, 0, Astro_TargetClass_Unknown}, // M94
    {38637, 42134, 32767, 0, Astro_TargetClass_Unknown}, // M95
    {38805, 42552, 32767, 0, Astro_TargetClass_Unknown}, // M96
    {40487, 198068, 32767, 361, Astro_TargetClass_PlanetaryNebula}, // M97
    {44028, 53642, 32767, 0, Astro_TargetClass_Unknown}, // M98
    {44329, 51899, 32767, 0, Astro_TargetClass_Unknown}, // M99
    {44574, 56961, 32767, 0, Astro_TargetClass_Unknown}, // M100
    {50592, 195657, 32767, 372, Astro_TargetClass_Galaxy}, // M101
    {54389, 200748, 32767, 0, Astro_TargetClass_Unknown}, // M102
    {5592, 218520, 32767, 0, Astro_TargetClass_Unknown}, // M103
    {45599, -41843, 32767, 388, Astro_TargetClass_Galaxy}, // M104
    {38869, 45294, 32767, 0, Astro_TargetClass_Unknown}, // M105
    {44337, 170294, 32767, 0, Astro_TargetClass_Unknown}, // M106
    {59551, -46994, 32767, 0, Astro_TargetClass_Unknown}, // M107
    {40291, 200427, 32767, 0, Astro_TargetClass_Unknown}, // M108
    {43056, 192148, 32767, 0, Astro_TargetClass_Unknown}, // M109
    {2422, 150067, 32767, 0, Astro_TargetClass_Unknown}, // M110
    {24309, -60178, 32767, 404, Astro_TargetClass_Star}, // Sirius
    {23037, -189705, 32767, 411, Astro_TargetClass_Star}, // Canopus
    {51340, 69057, 32767, 419, Astro_TargetClass_Star}, // Arcturus
    {67016, 139621, 32767, 428, Astro_TargetClass_Star}, // Vega
    {19001, 165593, 32767, 433, Astro_TargetClass_Star}, // Capella
    {52776, -219007, 32767, 441, Astro_TargetClass_Star}, // Rigel Kentaurus
    {27558, 18810, 32767, 457, Astro_TargetClass_Star}, // Procyon
    {21310, 26665, 32767, 465, Astro_TargetClass_Star}, // Betelgeuse
    {5863, -206052, 32767, 476, Astro_TargetClass_Star}, // Achernar
    {50629, -217343, 32767, 485, Astro_TargetClass_Star}, // Hadar
    {71447, 31926, 32767, 491, Astro_TargetClass_Star}, // Altair
    {44796, -227157, 32767, 498, Astro_TargetClass_Star}, // Acrux
    {16555, 59433, 32767, 504, Astro_TargetClass_Star}, // Aldebaran
    {48312, -40181, 32767, 514, Astro_TargetClass_Star}, // Spica
    {59364, -95155, 32767, 520, Astro_TargetClass_Star}, // Antares
    {27919, 100894, 32767, 528, Astro_TargetClass_Star}, // Pollux
    {82659, -106640, 32767, 535, Astro_TargetClass_Star}, // Fomalhaut
    {74486, 163009, 32767, 545, Astro_TargetClass_Star}, // Deneb
    {36502, 43082, 32767, 551, Astro_TargetClass_Star}, // Regulus
    {9109, 321351, 32767, 559, Astro_TargetClass_Star}, // Polaris
    {27276, 114798, 32767, 567, Astro_TargetClass_Star}, // Castor
    {19508, 22859, 32767, 574, Astro_TargetClass_Star}, // Bellatrix
    {20173, -4327, 32767, 584, Astro_TargetClass_Star}, // Alnilam
    {20446, -6994, 32767, 592, Astro_TargetClass_Star}, // Alnitak
    {48236, 197731, 32767, 600, Astro_TargetClass_Star}, // Mizar
    {39824, 222303, 32767, 606, Astro_TargetClass_Star}, // Dubhe
};

static const char astroMovingStringPool[] PROGMEM =
    "Sun\0Moon\0Mercury\0Venus\0Mars\0Jupiter\0Saturn\0Uranus\0Neptune\0";
static const uint8_t astroMovingStringOffsets[] PROGMEM = {0, 4, 9, 17, 23, 28, 36, 43, 50};

static void copyMovingTargetString(Astro_TargetId targetId, char *bufferOut, size_t bufferSize)
{
    if (!bufferOut || !bufferSize || targetId > Astro_Target_Neptune) { return; }
#ifdef __AVR__
    uint8_t offset = pgm_read_byte(&astroMovingStringOffsets[targetId]);
    strncpy_P(bufferOut, (PGM_P)(astroMovingStringPool + offset), bufferSize - 1);
    bufferOut[bufferSize - 1] = '\0';
#else
    snprintf(bufferOut, bufferSize, "%s", astroMovingStringPool + astroMovingStringOffsets[targetId]);
#endif
}

static AstroPackedTarget readPackedTarget(int targetIndex)
{
    AstroPackedTarget packed;
#ifdef __AVR__
    memcpy_P(&packed, &astroFixedTargets[targetIndex], sizeof(AstroPackedTarget));
#else
    packed = astroFixedTargets[targetIndex];
#endif
    return packed;
}

static void copyTargetString(uint16_t offset, char *bufferOut, size_t bufferSize)
{
    if (!bufferOut || !bufferSize) { return; }
#ifdef __AVR__
    strncpy_P(bufferOut, (PGM_P)(astroTargetStringPool + offset), bufferSize - 1);
    bufferOut[bufferSize - 1] = '\0';
#else
    snprintf(bufferOut, bufferSize, "%s", astroTargetStringPool + offset);
#endif
}

AstroTargetsLibraryBook::AstroTargetsLibraryBook()
    : data(), count(1), userSet(false)
{ ; }

AstroTargetsLibraryBook::AstroTargetsLibraryBook(const AstroTargetData &dataIn)
    : data(dataIn), count(1), userSet(false)
{ ; }

AstroTargetsLibrary astroLib;

AstroTargetsLibrary::AstroTargetsLibrary()
    : _hasUserTargets(false), _libSDTargetPrefix(), _libEEPROMDataAddress((size_t)-1),
      _libSDJSONFormat(true), _libEEPROMJSONFormat(false), _externalLoader(nullptr), _externalLoaderContext(nullptr)
{
    for (size_t i = 0; i < Astro_Target_Count; ++i) { _targetsData[i] = nullptr; }
}

AstroTargetsLibrary::~AstroTargetsLibrary()
{
    for (size_t i = 0; i < Astro_Target_Count; ++i) {
        if (_targetsData[i]) { delete _targetsData[i]; _targetsData[i] = nullptr; }
    }
}

void AstroTargetsLibrary::beginTargetsLibraryFromSDCard(const AstroString &dataFilePrefix, bool jsonFormat)
{
    _libSDTargetPrefix = dataFilePrefix;
    _libSDJSONFormat = jsonFormat;
}

void AstroTargetsLibrary::beginTargetsLibraryFromEEPROM(size_t dataAddress, bool jsonFormat)
{
    _libEEPROMDataAddress = dataAddress;
    _libEEPROMJSONFormat = jsonFormat;
}

void AstroTargetsLibrary::setExternalTargetLoader(ExternalTargetLoader loader, void *context)
{
    _externalLoader = loader;
    _externalLoaderContext = context;
}

const AstroTargetData *AstroTargetsLibrary::checkoutTargetData(Astro_TargetId targetId)
{
    if ((unsigned int)targetId >= Astro_Target_Count) { return nullptr; }
    AstroTargetsLibraryBook *book = _targetsData[targetId];
    if (book) {
        ++book->count;
        return &book->data;
    }
    book = newBookFromType(targetId);
    if (book) { _targetsData[targetId] = book; }
    return book ? &book->data : nullptr;
}

void AstroTargetsLibrary::returnTargetData(const AstroTargetData *targetData)
{
    if (!targetData || (unsigned int)targetData->targetId >= Astro_Target_Count) { return; }
    Astro_TargetId targetId = targetData->targetId;
    AstroTargetsLibraryBook *book = _targetsData[targetId];
    if (!book) { return; }
    if (book->count) { --book->count; }
    if (!book->count && (!book->userSet || !book->data.modified)) {
        delete book;
        _targetsData[targetId] = nullptr;
    }
}

bool AstroTargetsLibrary::setUserTargetData(const AstroTargetData *targetData)
{
    if (!targetData || (unsigned int)targetData->targetId >= Astro_Target_Count) { return false; }
    AstroTargetsLibraryBook *book = _targetsData[targetData->targetId];
    if (!book) {
        book = new AstroTargetsLibraryBook(*targetData);
        if (!book) { return false; }
        _targetsData[targetData->targetId] = book;
    } else {
        book->data = *targetData;
    }
    book->userSet = true;
    _hasUserTargets = true;
    return true;
}

bool AstroTargetsLibrary::dropUserTargetData(const AstroTargetData *targetData)
{
    if (!targetData || (unsigned int)targetData->targetId >= Astro_Target_Count) { return false; }
    AstroTargetsLibraryBook *book = _targetsData[targetData->targetId];
    if (!book || !book->userSet) { return false; }
    delete book;
    _targetsData[targetData->targetId] = nullptr;
    _hasUserTargets = false;
    for (size_t i = 0; i < Astro_Target_Count; ++i) {
        if (_targetsData[i] && _targetsData[i]->userSet) { _hasUserTargets = true; break; }
    }
    return true;
}

uint16_t AstroTargetsLibrary::getCheckoutCount(Astro_TargetId targetId) const
{
    if ((unsigned int)targetId >= Astro_Target_Count || !_targetsData[targetId]) { return 0; }
    return _targetsData[targetId]->count;
}

size_t AstroTargetsLibrary::getLoadedBookCount() const
{
    size_t count = 0;
    for (size_t i = 0; i < Astro_Target_Count; ++i) { if (_targetsData[i]) { ++count; } }
    return count;
}

AstroTargetsLibraryBook *AstroTargetsLibrary::newBookFromType(Astro_TargetId targetId)
{
    if (_externalLoader) {
        AstroTargetData data;
        if (_externalLoader(_externalLoaderContext, targetId, &data)) {
            data.targetId = targetId;
            return new AstroTargetsLibraryBook(data);
        }
    }
    return newBookFromBuiltin(targetId);
}

AstroTargetsLibraryBook *AstroTargetsLibrary::newBookFromBuiltin(Astro_TargetId targetId)
{
    AstroTargetData data;
    data.targetId = targetId;

    if (targetId <= Astro_Target_Neptune) {
        data.targetClass = Astro_TargetClass_SolarSystem;
        data.movingTarget = true;
        copyMovingTargetString(targetId, data.id, sizeof(data.id));
        copyMovingTargetString(targetId, data.targetName, sizeof(data.targetName));
        return new AstroTargetsLibraryBook(data);
    }

    int fixedIndex = -1;
    if (targetId >= Astro_Target_M1 && targetId <= Astro_Target_M110) {
        fixedIndex = (int)targetId - (int)Astro_Target_M1;
    } else if (targetId >= Astro_Target_Sirius && targetId <= Astro_Target_Dubhe) {
        fixedIndex = 110 + ((int)targetId - (int)Astro_Target_Sirius);
    } else {
        return nullptr;
    }

    AstroPackedTarget packed = readPackedTarget(fixedIndex);
    data.targetClass = packed.targetClass;
    data.rightAscensionSeconds = packed.rightAscensionSeconds;
    data.declinationArcseconds = packed.declinationArcseconds;
    data.magnitudeCenti = packed.magnitudeCenti;
    data.movingTarget = false;

    if (targetId >= Astro_Target_M1 && targetId <= Astro_Target_M110) {
        unsigned int messier = (unsigned int)targetId - (unsigned int)Astro_Target_M1 + 1;
        snprintf(data.id, sizeof(data.id), "M%u", messier);
        if (packed.nameOffset) {
            copyTargetString(packed.nameOffset, data.targetName, sizeof(data.targetName));
        } else {
            snprintf(data.targetName, sizeof(data.targetName), "Messier M%u", messier);
        }
    } else {
        copyTargetString(packed.nameOffset, data.targetName, sizeof(data.targetName));
        strncpy(data.id, data.targetName, sizeof(data.id) - 1);
        data.id[sizeof(data.id) - 1] = '\0';
    }

    return new AstroTargetsLibraryBook(data);
}


bool astroTargetIdToString(Astro_TargetId targetId, char *bufferOut, size_t bufferSize)
{
    if (!bufferOut || !bufferSize || (unsigned int)targetId >= Astro_Target_Count) { return false; }
    if (targetId <= Astro_Target_Neptune) {
        copyMovingTargetString(targetId, bufferOut, bufferSize);
        return true;
    }
    if (targetId >= Astro_Target_M1 && targetId <= Astro_Target_M110) {
        snprintf(bufferOut, bufferSize, "M%u", (unsigned int)targetId - (unsigned int)Astro_Target_M1 + 1);
        return true;
    }
    if (targetId >= Astro_Target_Sirius && targetId <= Astro_Target_Dubhe) {
        int fixedIndex = 110 + ((int)targetId - (int)Astro_Target_Sirius);
        AstroPackedTarget packed = readPackedTarget(fixedIndex);
        copyTargetString(packed.nameOffset, bufferOut, bufferSize);
        return true;
    }
    if (targetId >= Astro_Target_Custom1 && targetId <= Astro_Target_Custom8) {
        snprintf(bufferOut, bufferSize, "Custom%u", (unsigned int)targetId - (unsigned int)Astro_Target_Custom1 + 1);
        return true;
    }
    return false;
}

Astro_TargetId astroTargetIdFromString(const char *targetString)
{
    if (!targetString || !targetString[0]) { return Astro_Target_Undefined; }
    if ((targetString[0] == 'M' || targetString[0] == 'm') && targetString[1] >= '0' && targetString[1] <= '9') {
        int number = atoi(targetString + 1);
        if (number >= 1 && number <= 110) { return (Astro_TargetId)((int)Astro_Target_M1 + number - 1); }
    }
    char name[ASTRO_TARGET_NAME_MAXSIZE];
    for (int id = Astro_Target_Sun; id <= Astro_Target_Neptune; ++id) {
        copyMovingTargetString((Astro_TargetId)id, name, sizeof(name));
        if (strcmp(targetString, name) == 0) { return (Astro_TargetId)id; }
    }
    for (int id = Astro_Target_Sirius; id <= Astro_Target_Dubhe; ++id) {
        if (astroTargetIdToString((Astro_TargetId)id, name, sizeof(name)) && strcmp(targetString, name) == 0) {
            return (Astro_TargetId)id;
        }
    }
    if (strncmp(targetString, "Custom", 6) == 0) {
        int number = atoi(targetString + 6);
        if (number >= 1 && number <= 8) { return (Astro_TargetId)((int)Astro_Target_Custom1 + number - 1); }
    }
    return Astro_Target_Undefined;
}
