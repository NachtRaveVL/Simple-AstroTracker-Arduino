/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Object
*/

#ifndef AstroObject_H
#define AstroObject_H

struct AstroIdentity;
class AstroObject;
class AstroSubObject;

struct AstroObjectData;

#include "Astruino.h"
#include "AstroData.h"

// Creates object from passed object data (return ownership transfer - user code *must* delete returned object)
extern AstroObject *newObjectFromData(const AstroData *dataIn);

// Shortcut to get shared pointer for object with built-in cast.
template<class T = AstroObjInterface> inline SharedPtr<T> getSharedPtr(const AstroObjInterface *obj) { return obj ? reinterpret_pointer_cast<T>(obj->getSharedPtr()) : nullptr; }


// Simple class for referencing an object in the Astruino system.
// This class is mainly used to simplify object key generation, which is used when we
// want to uniquely refer to objects in the Astruino system.
struct AstroIdentity {
    enum : signed char { Actuator, Sensor, Mount, Rail, Unknown = -1 } type; // Object type (custom RTTI)
    inline bool isActuatorType() const { return type == Actuator; }
    inline bool isSensorType() const { return type == Sensor; }
    inline bool isMountType() const { return type == Mount; }
    inline bool isRailType() const { return type == Rail; }
    inline bool isUnknownType() const { return type <= Unknown; }

    union {
        Astro_ActuatorType actuatorType;                    // As actuator type enumeration
        Astro_SensorType sensorType;                        // As sensor type enumeration
        Astro_MountType mountType;                          // As mount type enumeration
        Astro_RailType railType;                            // As rail type enumeration
        aid_t idType;                                       // As standard id type enumeration
    } objTypeAs;                                            // Object type union
    aposi_t posIndex;                                       // Position index
    String keyString;                                       // String key
    akey_t key;                                             // UInt Key

    // Default/copy key (incomplete id)
    inline AstroIdentity(akey_t key = -1) : type(Unknown), objTypeAs{.idType=Unknown}, posIndex(-1), keyString(), key(key) { ; }
    // Copy into keyStr (incomplete id)
    inline AstroIdentity(const char *idKeyStr) : type(Unknown), objTypeAs{.idType=Unknown}, posIndex(-1), keyString(idKeyStr), key(stringHash(idKeyStr)) { ; }
    // Copy into keyStr (incomplete id)
    inline AstroIdentity(String idKey) : type(Unknown), objTypeAs{.idType=Unknown}, posIndex(-1), keyString(idKey), key(stringHash(idKey.c_str())) { ; }

    // Copy id with new position index
    inline AstroIdentity(const AstroIdentity &id, aposi_t positionIndex) : type(id.type), objTypeAs{.idType=id.objTypeAs.idType}, posIndex(positionIndex), keyString(), key(akey_none) { regenKey(); }

    // Actuator id constructor
    inline AstroIdentity(Astro_ActuatorType actuatorTypeIn,
                         aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG) : type(Actuator), objTypeAs{.actuatorType=actuatorTypeIn}, posIndex(positionIndex), keyString(), key(akey_none) { regenKey(); }
    // Sensor id constructor
    inline AstroIdentity(Astro_SensorType sensorTypeIn,
                         aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG) : type(Sensor), objTypeAs{.sensorType=sensorTypeIn}, posIndex(positionIndex), keyString(), key(akey_none) { regenKey(); }
    // Mount id constructor
    inline AstroIdentity(Astro_MountType mountTypeIn,
                         aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG) : type(Mount), objTypeAs{.mountType=mountTypeIn}, posIndex(positionIndex), keyString(), key(akey_none) { regenKey(); }
    // Rail id constructor
    inline AstroIdentity(Astro_RailType railTypeIn,
                         aposi_t positionIndex = ASTRO_POS_SEARCH_FROMBEG) : type(Rail), objTypeAs{.railType=railTypeIn}, posIndex(positionIndex), keyString(), key(akey_none) { regenKey(); }

    // Data constructor
    inline AstroIdentity(const AstroData *dataIn) : type((typeof(type))(dataIn->id.object.idType)), objTypeAs{.idType=dataIn->id.object.objType}, posIndex(dataIn->id.object.posIndex), keyString(), key(akey_none) { regenKey(); }

    // Used to update key value after modification, returning new key by convenience
    akey_t regenKey();

    // Returns displayable string associated with this identity (type + name)
    String getDisplayString();

    inline operator bool() const { return key != akey_none; }
    inline bool operator==(const AstroIdentity &otherId) const { return key == otherId.key; }
    inline bool operator!=(const AstroIdentity &otherId) const { return key != otherId.key; }
};


// Object Base
// A simple base class for referring to objects in the Astruino system.
class AstroObject : public AstroObjInterface {
public:
    inline bool isActuatorType() const { return _id.isActuatorType(); }
    inline bool isSensorType() const { return _id.isSensorType(); }
    inline bool isMountType() const { return _id.isMountType(); }
    inline bool isRailType() const { return _id.isRailType(); }
    inline bool isUnknownType() const { return _id.isUnknownType(); }

    inline AstroObject(AstroIdentity id) : _id(id), _revision(-1), _linksSize(0), _links(nullptr) { ; }
    inline AstroObject(const AstroData *data) : _id(data), _revision(data->_revision), _linksSize(0), _links(nullptr) { ; }
    virtual ~AstroObject();

    // Called over intervals of time by runloop
    virtual void update();
    // Called upon low memory condition to try and free memory up
    virtual void handleLowMemory();

    // Saves object state to proper backing data
    AstroData *newSaveData();

    // (Re)allocates linkage list of specified size
    void allocateLinkages(size_t size = 1);
    // Adds linkage to this object, returns true upon initial add
    virtual bool addLinkage(AstroObject *obj);
    // Removes linkage from this object, returns true upon last remove
    virtual bool removeLinkage(AstroObject *obj);
    // Checks object linkage to this object
    bool hasLinkage(AstroObject *obj) const;

    // Returns the linkages this object contains, along with refcount for how many times it has registered itself as linked (via attachment points).
    // Objects are considered strong pointers, since existence -> SharedPtr ref to this instance exists.
    inline Pair<uint8_t, Pair<AstroObject *, int8_t> *> getLinkages() const { return make_pair(_linksSize, _links); }

    // Unresolves any dlinks to obj prior to caching
    virtual void unresolveAny(AstroObject *obj) override;
    // Unresolves this instance from any dlinks
    inline void unresolve() { unresolveAny(this); }

    // Returns the unique Identity of the object
    virtual AstroIdentity getId() const override;
    // Returns the unique key of the object
    virtual akey_t getKey() const override;
    // Returns the key string of the object
    virtual String getKeyString() const override;
    // Returns the SharedPtr instance for this object
    virtual SharedPtr<AstroObjInterface> getSharedPtr() const override;
    // Returns the SharedPtr instance for passed object
    virtual SharedPtr<AstroObjInterface> getSharedPtrFor(const AstroObjInterface *obj) const override;
    // Returns true for object
    virtual bool isObject() const override;

    // Returns revision #
    inline uint8_t getRevision() const { return abs(_revision); }
    // If revision has been modified since last saved
    inline bool isModified() const { return _revision < 0; }
    // Bumps revision # if not already modified, and sets modified flag (called after modifying data)
    inline void bumpRevisionIfNeeded() { if (!isModified()) { _revision = -(abs(_revision) + 1); } }
    // Unsets modified flag from revision (called after save-out)
    inline void unsetModified() { _revision = abs(_revision); }

protected:
    AstroIdentity _id;                                      // Object id
    int8_t _revision;                                       // Revision # of stored data (uses -vals for modified flag)
    uint8_t _linksSize;                                     // Number of object linkages
    Pair<AstroObject *, int8_t> *_links;                    // Object linkages array (owned, lazily allocated/grown/shrunk)

    virtual AstroData *allocateData() const;                // Only up to base type classes (sensor, crop, etc.) does this need overriden
    virtual void saveToData(AstroData *dataOut);            // *ALL* derived classes must override and implement

private:
    // Private constructor to disable derived/public access
    inline AstroObject() : _id(), _revision(-1), _linksSize(0), _links(nullptr) { ; }
};


// Sub Object Base
// A base class for sub objects that are typically found embedded in bigger main objects,
// but want to replicate some of the same functionality. Not required to be inherited from.
class AstroSubObject : public AstroObjInterface {
public:
    inline AstroSubObject(AstroObjInterface *parent = nullptr) : _parent(parent) { ; }

    virtual void setParent(AstroObjInterface *parent);
    inline AstroObjInterface *getParent() const { return _parent; }

    virtual void unresolveAny(AstroObject *obj) override;

    virtual AstroIdentity getId() const override;
    virtual akey_t getKey() const override;
    virtual String getKeyString() const override;
    virtual SharedPtr<AstroObjInterface> getSharedPtr() const override;
    virtual SharedPtr<AstroObjInterface> getSharedPtrFor(const AstroObjInterface *obj) const override;

    virtual bool isObject() const override;

    inline uint8_t getRevision() const { return _parent && _parent->isObject() ? ((AstroObject *)_parent)->getRevision() : 0; }
    inline bool isModified() const { return _parent && _parent->isObject() ? ((AstroObject *)_parent)->isModified() : false; }
    inline void bumpRevisionIfNeeded() { if (_parent && _parent->isObject()) { ((AstroObject *)_parent)->bumpRevisionIfNeeded(); } }
    inline void unsetModified() { ; }

protected:
    AstroObjInterface *_parent;                             // Parent object pointer (reverse ownership)
};


// Object Data Intermediate
// Intermediate data class for object data.
struct AstroObjectData : public AstroData {
    char name[ASTRO_NAME_MAXSIZE];

    AstroObjectData();
    virtual void toJSONObject(JsonObject &objectOut) const override;
    virtual void fromJSONObject(JsonObjectConst &objectIn) override;
};

#endif // /ifndef AstroObject_H
