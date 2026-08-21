/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Object
*/

#ifndef AstroObject_H
#define AstroObject_H

#include "AstroDefines.h"
#include "AstroInterfaces.h"

struct AstroObjectData;

// Shortcut to get shared pointer for object with built-in cast.
template<class T = AstroObjInterface> inline SharedPtr<T> getSharedPtr(const AstroObjInterface *object) { return object ? reinterpret_pointer_cast<T>(object->getSharedPtr()) : nullptr; }

// Object Identity
// Compact object identity used to generate stable object keys and names.
struct AstroIdentity {
    enum : signed char { Actuator, Sensor, Mount, Rail, Cover, ObservationDevice, Unknown = -1 } type;

    union {
        Astro_ActuatorType actuatorType;                     // Actuator type
        Astro_SensorType sensorType;                         // Sensor type
        Astro_MountType mountType;                           // Mount type
        Astro_RailType railType;                             // Rail type
        int16_t idType;                                      // ID type
    } objTypeAs;                                             // Obj type as

    aposi_t posIndex;                                        // Position index
    AstroString keyString;                                   // Human-readable object key
    akey_t key;                                              // Hashed object key

    AstroIdentity(akey_t keyIn = akey_none);
    AstroIdentity(const char *keyStringIn);
    AstroIdentity(Astro_ActuatorType actuatorTypeIn, aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    AstroIdentity(Astro_SensorType sensorTypeIn, aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    AstroIdentity(Astro_MountType mountTypeIn, aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    AstroIdentity(Astro_RailType railTypeIn, aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);
    AstroIdentity(int objectType, int16_t subType, aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG);

    inline bool isActuatorType() const { return type == Actuator; }
    inline bool isSensorType() const { return type == Sensor; }
    inline bool isMountType() const { return type == Mount; }
    inline bool isRailType() const { return type == Rail; }
    inline bool isCoverType() const { return type == Cover; }
    inline bool isObservationDeviceType() const { return type == ObservationDevice; }
    inline bool isUnknownType() const { return type <= Unknown; }

    akey_t regenKey();
    AstroString getDisplayString() const;

    inline operator bool() const { return key != akey_none; }
    inline bool operator==(const AstroIdentity &other) const { return key == other.key; }
    inline bool operator!=(const AstroIdentity &other) const { return key != other.key; }
};

// Object Serialization Data
// Base serialized state shared by Astruino object types.
struct AstroObjectData {
    int8_t idType;                                           // ID type
    int16_t objType;                                         // Obj type
    aposi_t posIndex;                                        // Position index
    uint8_t revision;                                        // Revision
    char name[ASTRO_NAME_MAXSIZE];                           // Name

    AstroObjectData();
    bool toJSON(char *bufferOut, size_t bufferSize) const;
    bool fromJSON(const char *jsonIn);
};

// Object Link
// Tracks a non-owning relationship between registered Astruino objects.
struct AstroObjectLink {
    AstroObject *object;                                     // Object, not owned
    int8_t count;                                            // Count

    AstroObjectLink(AstroObject *objectIn = nullptr, int8_t countIn = 0)
        : object(objectIn), count(countIn)
    { ; }
};

// Object Base
// Base class for registered Astruino devices and logical system objects.
class AstroObject : public AstroObjInterface {
public:
    AstroObject(AstroIdentity id = AstroIdentity());
    AstroObject(const AstroObjectData *dataIn);
    virtual ~AstroObject();

    virtual void update();
    virtual void handleLowMemory();

    AstroObjectData *newSaveData();

    void allocateLinkages(size_t size = 1);
    virtual bool addLinkage(AstroObject *object);
    virtual bool removeLinkage(AstroObject *object);
    bool hasLinkage(AstroObject *object) const;

    inline void unresolve() { unresolveAny(this); }
    virtual void unresolveAny(AstroObject *object) override;
    virtual akey_t getKey() const override;
    virtual AstroString getKeyString() const override;
    virtual bool isObject() const override;

    virtual AstroIdentity getId() const override { return _id; }
    virtual SharedPtr<AstroObjInterface> getSharedPtr() const override;
    virtual SharedPtr<AstroObjInterface> getSharedPtrFor(const AstroObjInterface *object) const override;
    inline uint8_t getRevision() const { return _revision < 0 ? (uint8_t)-_revision : (uint8_t)_revision; }
    inline bool isModified() const { return _revision < 0; }
    inline void bumpRevisionIfNeeded() { if (!isModified()) { _revision = -(int8_t)(getRevision() + 1); } }
    inline void unsetModified() { if (_revision < 0) { _revision = -_revision; } }

    inline size_t getLinkagesSize() const { return _linksSize; }
    inline const AstroObjectLink *getLinkages() const { return _links; }

protected:
    AstroIdentity _id;                                       // Object identity
    int8_t _revision;                                        // Stored object revision, negative when modified
    size_t _linksSize;                                       // Allocated linkage slot count
    AstroObjectLink *_links;                                 // Object linkages array, owned

    virtual AstroObjectData *allocateData() const;
    virtual void saveToData(AstroObjectData *dataOut) const;
};

// Sub Object Base
// Lightweight child object that borrows identity and revision state from a parent object.
class AstroSubObject : public AstroObjInterface {
public:
    AstroSubObject(AstroObjInterface *parent = nullptr) : _parent(parent) { ; }

    virtual void setParent(AstroObjInterface *parent) { _parent = parent; }
    inline AstroObjInterface *getParent() const { return _parent; }

    virtual void unresolveAny(AstroObject *object) override { (void)object; }
    virtual AstroIdentity getId() const override;
    virtual akey_t getKey() const override;
    virtual AstroString getKeyString() const override;
    virtual SharedPtr<AstroObjInterface> getSharedPtr() const override;
    virtual SharedPtr<AstroObjInterface> getSharedPtrFor(const AstroObjInterface *object) const override;
    virtual bool isObject() const override { return false; }

    inline uint8_t getRevision() const { return _parent && _parent->isObject() ? static_cast<AstroObject *>(_parent)->getRevision() : 0; }
    inline bool isModified() const { return _parent && _parent->isObject() ? static_cast<AstroObject *>(_parent)->isModified() : false; }
    inline void bumpRevisionIfNeeded() { if (_parent && _parent->isObject()) { static_cast<AstroObject *>(_parent)->bumpRevisionIfNeeded(); } }
    inline void unsetModified() { ; }

protected:
    AstroObjInterface *_parent;                              // Parent, not owned
};

#endif // /ifndef AstroObject_H
