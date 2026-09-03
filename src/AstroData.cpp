/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Data
*/

#include "Astruino.h"
#include "AstroCoreLogic.h"

static size_t skipBinaryStreamBytes(Stream *streamIn, size_t bytesToSkip)
{
    size_t skippedBytes = 0;
    uint8_t skipBuffer[16];

    while (skippedBytes < bytesToSkip) {
        size_t chunkSize = bytesToSkip - skippedBytes;
        if (chunkSize > sizeof(skipBuffer)) { chunkSize = sizeof(skipBuffer); }

        const size_t skipped = streamIn->readBytes(skipBuffer, chunkSize);
        skippedBytes += skipped;
        if (skipped != chunkSize) { break; }
    }

    return skippedBytes;
}

size_t serializeDataToBinaryStream(const AstroData *data, Stream *streamOut, size_t skipBytes)
{
    return streamOut->write((const uint8_t *)data + skipBytes, data->_size - skipBytes);
}

size_t deserializeDataFromBinaryStream(AstroData *data, Stream *streamIn, size_t skipBytes)
{
    return streamIn->readBytes((uint8_t *)data + skipBytes, data->_size - skipBytes);
}

AstroData *newDataFromBinaryStream(Stream *streamIn)
{
    AstroData baseDecode;
    const size_t baseSize = baseDecode._size;
    const size_t basePayloadSize = baseSize - sizeof(void*);
    size_t readBytes = deserializeDataFromBinaryStream(&baseDecode, streamIn, sizeof(void*));
    const size_t serializedSize = baseDecode._size;
    const bool baseReadValid = readBytes == basePayloadSize && serializedSize >= baseSize;
    ASTRO_SOFT_ASSERT(baseReadValid, SFP(AStr_Err_ImportFailure));
    if (!baseReadValid) { return nullptr; }

    AstroData *data = _allocateDataFromBaseDecode(baseDecode);
    ASTRO_SOFT_ASSERT(data, SFP(AStr_Err_AllocationFailure));
    if (!data) { return nullptr; }

    const auto readPlan = astroBinaryDataReadPlan(serializedSize, data->_size, baseSize);
    if (readPlan.copyBytes) {
        readBytes += streamIn->readBytes((uint8_t *)data + baseSize, readPlan.copyBytes);
    }

    const size_t skippedBytes = skipBinaryStreamBytes(streamIn, readPlan.skipBytes);
    const bool payloadReadValid = readBytes == basePayloadSize + readPlan.copyBytes &&
                                  skippedBytes == readPlan.skipBytes;
    ASTRO_SOFT_ASSERT(payloadReadValid, SFP(AStr_Err_ImportFailure));
    if (!payloadReadValid) {
        delete data;
        return nullptr;
    }

    data->migrateFromBinaryVersion(baseDecode._version);
    return data;
}

AstroData *newDataFromJSONObject(JsonObjectConst &objectIn)
{
    AstroData baseDecode;
    baseDecode.fromJSONObject(objectIn);

    AstroData *data = _allocateDataFromBaseDecode(baseDecode);
    ASTRO_SOFT_ASSERT(data, SFP(AStr_Err_AllocationFailure));

    if (data) {
        data->fromJSONObject(objectIn);
        return data;
    }

    return nullptr;
}


AstroData::AstroData()
    : id{.chars={'\000','\000','\000','\000'}}, _version(1), _revision(-1)
{
    _size = sizeof(*this);
}

AstroData::AstroData(char id0, char id1, char id2, char id3, uint8_t version, uint8_t revision)
    : id{.chars={id0,id1,id2,id3}}, _version(version), _revision((int8_t)revision)
{
    _size = sizeof(*this);
    ASTRO_HARD_ASSERT(isStandardData(), SFP(AStr_Err_InvalidParameter));
}

AstroData::AstroData(aid_t idType, aid_t objType, aposi_t posIndex, aid_t classType, uint8_t version, uint8_t revision)
    : id{.object={idType,objType,posIndex,classType}}, _version(version), _revision((int8_t)revision)
{
    _size = sizeof(*this);
}

AstroData::AstroData(const AstroIdentity &id)
    : AstroData(id.type, id.objTypeAs.idType, id.posIndex, -1, 1, 0)
{
    _size = sizeof(*this);
}

void AstroData::toJSONObject(JsonObject &objectOut) const
{
    if (isStandardData()) {
        objectOut[SFP(AStr_Key_Type)] = charsToString(id.chars, sizeof(id.chars));
    } else {
        int8_t typeVals[4] = {id.object.idType, id.object.objType, id.object.posIndex, id.object.classType};
        objectOut[SFP(AStr_Key_Type)] = commaStringFromArray(typeVals, 4);
    }
    if (_version > 1) { objectOut[SFP(AStr_Key_Version)] = _version; }
    if (getRevision() > 1) { objectOut[SFP(AStr_Key_Revision)] = getRevision(); }
}

void AstroData::fromJSONObject(JsonObjectConst &objectIn)
{
    JsonVariantConst idVar = objectIn[SFP(AStr_Key_Type)];
    const char *idStr = idVar.as<const char *>();
    if (idStr && idStr[0] == 'A') {
        strncpy(id.chars, idStr, 4);
    } else if (idStr) {
        int8_t typeVals[4];
        commaStringToArray(idStr, typeVals, 4);
        id.object.idType = typeVals[0];
        id.object.objType = typeVals[1];
        id.object.posIndex = typeVals[2];
        id.object.classType = typeVals[3];
    }
    _version = objectIn[SFP(AStr_Key_Version)] | _version;
    _revision = objectIn[SFP(AStr_Key_Revision)] | _revision;
    _revision = abs(_revision);
}


AstroSubData::AstroSubData()
    : type(aid_none)
{ ; }

AstroSubData::AstroSubData(aid_t dataType)
    : type(dataType)
{ ; }

void AstroSubData::toJSONObject(JsonObject &objectOut) const
{
    if (type != aid_none) { objectOut[SFP(AStr_Key_Type)] = type; }
}

void AstroSubData::fromJSONObject(JsonObjectConst &objectIn)
{
    type = objectIn[SFP(AStr_Key_Type)] | type;
}
