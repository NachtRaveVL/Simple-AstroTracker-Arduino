/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Attachments
*/

#ifndef AstroAttachments_H
#define AstroAttachments_H

class AstroActuator;
class AstroSensor;
class AstroAxisDriver;
class AstroMount;
class AstroRail;
class AstroTrigger;
class AstroObservationDevice;

#include "AstroDefines.h"
#include "AstroMeasurements.h"

// Object Attachment
// Lightweight non-owning attachment used to link independently created system objects.
// The parent sub-index follows the same pattern used by the sibling libraries for axis,
// channel, or row references associated with the attachment.
template<class TObject>
class AstroAttachment {
public:
    AstroAttachment(TObject *object = nullptr, aposi_t parentSubIndex = 0)
        : _object(object), _parentSubIndex(parentSubIndex) { ; }

    inline void setObject(TObject *object) { _object = object; }
    inline TObject *getObject() const { return _object; }
    inline TObject *operator->() const { return _object; }
    inline operator bool() const { return _object != nullptr; }

    inline void setParentSubIndex(aposi_t parentSubIndex) { _parentSubIndex = parentSubIndex; }
    inline aposi_t getParentSubIndex() const { return _parentSubIndex; }

protected:
    TObject *_object;                                       // Attached object pointer, not owned
    aposi_t _parentSubIndex;                                // Parent axis/channel/sub-object index
};

// Actuator Attachment
// Convenience wrapper for normalized power control through an attached actuator.
class AstroActuatorAttachment : public AstroAttachment<AstroActuator> {
public:
    AstroActuatorAttachment(AstroActuator *object = nullptr, aposi_t parentSubIndex = 0);

    void setPower(float power);
    float getPower() const;
};

// Sensor Attachment
// Wraps sensor polling and selected measurement-row metadata.
class AstroSensorAttachment : public AstroAttachment<AstroSensor> {
public:
    AstroSensorAttachment(AstroSensor *object = nullptr, uint8_t measurementRow = 0,
                          double convertParam = 0.0, aposi_t parentSubIndex = 0);

    bool poll(int64_t timestamp = 0, aframe_t frame = 1);
    const AstroSingleMeasurement *getMeasurement() const;

    inline void setMeasurementRow(uint8_t measurementRow) { _measurementRow = measurementRow; }
    inline uint8_t getMeasurementRow() const { return _measurementRow; }
    inline void setMeasurementConvertParam(double convertParam) { _convertParam = convertParam; }
    inline double getMeasurementConvertParam() const { return _convertParam; }

protected:
    uint8_t _measurementRow;                                // Selected source measurement row
    double _convertParam;                                   // Optional units conversion parameter
};

// Axis Driver Attachment
// Provides a common setpoint/stop wrapper for any supported mount-axis driver.
class AstroAxisDriverAttachment : public AstroAttachment<AstroAxisDriver> {
public:
    AstroAxisDriverAttachment(AstroAxisDriver *object = nullptr, aposi_t axisIndex = 0);

    void setTargetDegrees(double targetDegrees);
    void stop();
    double getTargetDegrees() const;
};

// Mount Attachment
class AstroMountAttachment : public AstroAttachment<AstroMount> {
public:
    using AstroAttachment<AstroMount>::AstroAttachment;
};

// Power Rail Attachment
class AstroRailAttachment : public AstroAttachment<AstroRail> {
public:
    using AstroAttachment<AstroRail>::AstroAttachment;
};

// Trigger Attachment
class AstroTriggerAttachment : public AstroAttachment<AstroTrigger> {
public:
    using AstroAttachment<AstroTrigger>::AstroAttachment;
};

// Observation Device Attachment
class AstroObservationDeviceAttachment : public AstroAttachment<AstroObservationDevice> {
public:
    using AstroAttachment<AstroObservationDevice>::AstroAttachment;
};

#include "AstroAttachments.hpp"

#endif // /ifndef AstroAttachments_H
