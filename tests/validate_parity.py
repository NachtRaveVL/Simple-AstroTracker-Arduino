#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src"

forbidden = {
    "AstroSignal<": "parallel callback signal implementation",
    "class AstroSignal {": "parallel callback signal implementation",
    "class AstroModule": "parallel generic module lifecycle",
    "class AstroStream": "parallel stream hierarchy",
    "AstroMemoryStream": "parallel memory stream hierarchy",
    "astroJSONGet": "manual JSON parser",
    "_pendingState": "parallel trigger state machine",
    "_stableTimeMs": "parallel trigger stable-time state machine",
    "setCalibration(": "parallel sensor calibration API",
    "clearCalibration(": "parallel sensor calibration API",
    "_rawMinimum": "sensor-local calibration state",
    "_rawMaximum": "sensor-local calibration state",
    "_valueMinimum": "sensor-local calibration state",
    "_valueMaximum": "sensor-local calibration state",
    "resolveActivations(": "parallel activation resolver",
    "LogSink": "parallel logger callback sink",
    "PublishSink": "parallel publisher callback sink",
}

errors = []
for path in SRC.rglob("*"):
    if not path.is_file() or path.suffix.lower() not in {".h", ".hh", ".hpp", ".cpp"}:
        continue
    text = path.read_text(errors="ignore")
    for token, reason in forbidden.items():
        if token in text:
            errors.append(f"{path.relative_to(ROOT)}: forbidden {reason}: {token}")

required = {
    "src/Astruino.h": ["public AstroCalibrations", "public AstroObjectRegistration"],
    "src/AstroObject.h": ["struct AstroIdentity"],
    "src/AstroAttachments.h": ["class AstroDLinkObject", "class AstroAttachment", "class AstroSignalAttachment"],
    "src/AstroMeasurements.h": ["AstroBinaryMeasurement", "AstroSingleMeasurement", "AstroDoubleMeasurement", "AstroTripleMeasurement"],
    "src/AstroTriggers.h": ["class AstroTrigger", "class AstroMeasurementValueTrigger", "class AstroMeasurementRangeTrigger"],
    "src/AstroModules.h": ["class AstroCalibrations", "class AstroObjectRegistration"],
    "src/AstroCallback.hh": ["template <class ParameterType> class Slot", "template <class ParameterType, int Slots = 8> class Signal"],
    "src/AstroCoreLogic.h": ["astroElapsedTime", "AstroBinaryDataReadPlan", "astroBinaryDataReadPlan"],
    "src/AstroStreams.h": ["public Stream", "AstroPROGMEMStream"],
    "src/AstroData.h": ["struct AstroData", "struct AstroSubData", "toJSONObject", "fromJSONObject", "serializeDataToBinaryStream", "newDataFromBinaryStream"],
}
for relative, tokens in required.items():
    path = ROOT / relative
    if not path.is_file():
        errors.append(f"missing canonical framework file: {relative}")
        continue
    text = path.read_text(errors="ignore")
    for token in tokens:
        if token not in text:
            errors.append(f"{relative}: missing canonical construct: {token}")

streams = SRC / "AstroStreams.h"
if streams.is_file():
    text = streams.read_text(errors="ignore")
    if "public Stream" not in text:
        errors.append("src/AstroStreams.h: stream implementation is not Arduino Stream based")

# Calibration data follows Hydro/Helio identity-string ownership, not an alternate raw-key record.
for relative, owner_token in (("src/AstroDatas.h", "ownerName"),) if "Astro" in str(ROOT) else (("src/TerraDatas.h", "ownerName"),):
    path = ROOT / relative
    if path.is_file():
        text = path.read_text(errors="ignore")
        if owner_token not in text:
            errors.append(f"{relative}: calibration data missing canonical ownerName")
        if "ownerKey" in text:
            errors.append(f"{relative}: calibration data retains parallel ownerKey")

if errors:
    for error in errors:
        print(f"FAIL: {error}")
    sys.exit(1)

print("PASS Astruino Hydro/Helio parity guard")
