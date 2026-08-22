#!/usr/bin/env python3
from pathlib import Path
import re
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
errors = []

readme = (root / "README.md").read_text()
if "—" in readme:
    errors.append("README contains em dash")

for path in [root / "README.md", root / "src", root / "examples", root / "tests"]:
    files = [path] if path.is_file() else [p for p in path.rglob("*") if p.is_file()]
    for file in files:
        if "src/shared" in file.as_posix() or "src/min" in file.as_posix() or "src/full" in file.as_posix():
            continue
        if file.name == "validate_source.py":
            continue
        text = file.read_text(errors="ignore")
        if re.search(r"\bdoors?\b", text, re.IGNORECASE):
            errors.append(f"{file.relative_to(root)} uses Door instead of Cover terminology")

source_files = [p for p in (root / "src").glob("Astro*.*") if p.is_file()] + [root / "src/Astruino.h", root / "src/Astruino.cpp"]
source = "\n".join(p.read_text(errors="ignore") for p in source_files if p.exists())
for required in [
    "Astro_Target_M110", "Astro_Target_Moon", "Astro_Target_Neptune",
    "class AstroTargetsLibrary", "class AstroScheduler", "class AstroThermalBalancer",
    "class AstroCover", "class AstroObject", "struct AstroPin", "class AstroFactory",
    "class AstroDigitalActuator", "class AstroAnalogActuator",
    "class AstroDigitalSensor", "class AstroAnalogSensor",
]:
    if required not in source:
        errors.append(f"missing {required}")

required_core = {
    "AstroObject.h": 3000,
    "AstroObject.cpp": 5000,
    "AstroPins.h": 4500,
    "AstroPins.cpp": 6000,
    "AstroUtils.h": 2500,
    "AstroUtils.cpp": 12000,
    "AstroFactory.cpp": 1800,
    "AstroMeasurements.h": 3500,
    "AstroMeasurements.cpp": 4500,
    "AstroTriggers.h": 1500,
    "AstroTriggers.cpp": 1500,
    "AstroTargetsLibrary.cpp": 10000,
    "AstroScheduler.cpp": 4500,
    "AstroActivation.h": 2500,
    "AstroActivation.cpp": 2000,
    "AstroActuators.h": 2500,
    "AstroActuators.cpp": 3500,
    "AstroSensors.h": 3200,
    "AstroSensors.cpp": 1500,
    "AstroAttachments.h": 1200,
    "AstroAttachments.cpp": 1100,
    "AstroDrivers.h": 1400,
    "AstroDrivers.cpp": 1100,
    "AstroModules.h": 1600,
    "AstroDatas.cpp": 2400,
    "AstroPublisher.cpp": 1700,
    "AstroLogger.cpp": 2400,
    "AstroStreams.cpp": 1200,
}
for filename, minimum in required_core.items():
    path = root / "src" / filename
    if not path.exists() or path.stat().st_size < minimum:
        errors.append(f"{filename} appears incomplete ({path.stat().st_size if path.exists() else 0} bytes)")


# Family-parity regression guards.
for forbidden, description in [
    ("ASTRUINO_VERSION_MAJOR", "project-local version macros"),
    ("ASTRUINO_VERSION_MINOR", "project-local version macros"),
    ("ASTRUINO_VERSION_PATCH", "project-local version macros"),
    ("ASTRUINO_VERSION_STRING", "project-local version macros"),
]:
    if forbidden in source:
        errors.append(f"source still contains {description}: {forbidden}")
if re.search(r"\bAstroPi\b", source):
    errors.append("source still contains project-local PI constant: AstroPi")

# Computed floating-point values must not use exact equality/inequality against decimal literals.
# Use isFPEqual() or an explicit tolerance/range comparison instead.
float_equality = re.compile(r"(?:==|!=)\s*-?\d+\.\d+(?:[eE][+-]?\d+)?[fFlL]?|-?\d+\.\d+(?:[eE][+-]?\d+)?[fFlL]?\s*(?:==|!=)")
for path in [p for p in (root / "src").glob("Astro*.*") if p.suffix in {".h", ".hpp", ".hh", ".cpp"}]:
    if path.name == "AstroEnumTrie.h":
        continue
    for line_no, line in enumerate(path.read_text(errors="ignore").splitlines(), 1):
        if float_equality.search(line):
            errors.append(f"{path.name}:{line_no} uses direct floating-point equality/inequality")

# Keep floating-point equality decisions routed through isFPEqual() rather than
# duplicating the epsilon comparison at individual call sites.
for path in [p for p in (root / "src").glob("Astro*.*") if p.suffix in {".h", ".hpp", ".hh", ".cpp"}]:
    if path.name in {"AstroDefines.h", "AstroInlines.hh"}:
        continue
    for line_no, line in enumerate(path.read_text(errors="ignore").splitlines(), 1):
        if "ASTRO_FLT_EPSILON" in line or "ASTRO_DBL_EPSILON" in line:
            errors.append(f"{path.name}:{line_no} bypasses isFPEqual() with a raw epsilon comparison")

for required in [
    "enum Astro_String", "stringFromPGM", "stringFromPGMAddr", "pgmAddrForStr",
    "#define SFP", "#define CFP",
    "struct AstroLoggerSubData", "struct AstroPublisherSubData", "struct AstroSchedulerSubData",
    "class AstroMeasurementUnitsInterface", "class AstroParentMountAttachmentInterface",
    "class AstroManualTimeProvider", "class AstroFixedLocationProvider",
]:
    if required not in source:
        errors.append(f"missing family infrastructure: {required}")

for filename, minimum in {
    "Astruino.h": 9000,
    "AstroInterfaces.h": 9000,
    "AstroInterfaces.hpp": 3500,
    "AstroInlines.hh": 6500,
    "AstroFactory.h": 3500,
    "AstroFactory.cpp": 4000,
    "AstroStrings.h": 6000,
    "AstroStrings.cpp": 12000,
}.items():
    path = root / "src" / filename
    if not path.exists() or path.stat().st_size < minimum:
        errors.append(f"{filename} family layer appears incomplete ({path.stat().st_size if path.exists() else 0} bytes)")

# Stored members and enum values should retain the short inline comments used by the sibling libraries.
member_decl = re.compile(r"^(?:const\s+)?[A-Za-z_][A-Za-z0-9_:<>]*(?:\s*[*&])?\s+[A-Za-z_][A-Za-z0-9_]*(?:\[[^]]+\])?(?:\s*=.*)?;")
enum_value = re.compile(r"^Astro_[A-Za-z0-9_]+(?:\s*=\s*[^,]+)?\s*,?$")
for header in (root / "src").glob("*.h"):
    if header.name in {"AstroEnumTrie.h", "AstroStrings.h"}:
        continue
    lines = header.read_text(errors="ignore").splitlines()
    in_enum = False
    for line_no, line in enumerate(lines, 1):
        stripped = line.strip()
        if "enum " in stripped and "{" in stripped:
            in_enum = True
        if in_enum and enum_value.match(stripped) and "//" not in line:
            errors.append(f"{header.name}:{line_no} enum value is missing a comment")
        if in_enum and "};" in stripped:
            in_enum = False
        if not stripped or stripped.startswith(("#", "//", "/*", "*", "enum", "class ", "struct ", "typedef ", "using ", "return ", "case ", "friend ", "template")):
            continue
        if "(" in stripped or ")" in stripped or stripped.startswith(("virtual ", "inline ", "static ")):
            continue
        if member_decl.match(stripped) and "//" not in line:
            errors.append(f"{header.name}:{line_no} stored member is missing a comment")

required_tc_menu = [
    "tcMenu_Display_AdaFruitGfx.cpp", "tcMenu_Display_AdaFruitGfx.h", "tcMenu_Display_AdaFruitGfx.hpp",
    "tcMenu_Display_LiquidCrystal.cpp", "tcMenu_Display_LiquidCrystal.h",
    "tcMenu_Display_TfteSpi.cpp", "tcMenu_Display_TfteSpi.h",
    "tcMenu_Display_U8g2.cpp", "tcMenu_Display_U8g2.h",
    "tcMenu_Extra_BspUserSettings.h", "tcMenu_Extra_StChromaArt.cpp", "tcMenu_Extra_StChromaArt.h",
    "tcMenu_Font_AdafruitGFXArial14.h", "tcMenu_Font_tcUnicodeArial14.h",
    "tcMenu_Input_AdaTouchDriver.cpp", "tcMenu_Input_AdaTouchDriver.h",
    "tcMenu_Input_ESP32TouchKeysAbstraction.cpp", "tcMenu_Input_ESP32TouchKeysAbstraction.h",
    "tcMenu_Remote_EthernetTransport.cpp", "tcMenu_Remote_EthernetTransport.h",
    "tcMenu_Remote_SerialTransport.cpp", "tcMenu_Remote_SerialTransport.h",
    "tcMenu_Remote_SimhubConnector.cpp", "tcMenu_Remote_SimhubConnector.h",
    "tcMenu_Remote_WiFiTransport.cpp", "tcMenu_Remote_WiFiTransport.h",
    "tcMenu_Theme_CoolBlueModern.h", "tcMenu_Theme_CoolBlueTraditional.h",
    "tcMenu_Theme_DarkModeModern.h", "tcMenu_Theme_DarkModeTraditional.h",
    "tcMenu_Theme_MonoBordered.h", "tcMenu_Theme_MonoInverse.h",
]
for filename in required_tc_menu:
    path = root / "src" / "shared" / filename
    if not path.exists() or path.stat().st_size < 1000:
        errors.append(f"missing/incomplete shared tcMenu adapter: {filename}")

required_tests = [
    "AstroLibExportToCPP/AstroLibExportToCPP.ino",
    "EnumConversionTests/EnumConversionTests.ino",
    "EnumTrieExportToCPP/EnumTrieExportToCPP.ino",
    "JSONExportTests/JSONExportTests.ino",
    "host/test_astronomy.cpp",
    "host/test_catalog.cpp",
    "host/test_automation.cpp",
    "host/test_enums.cpp",
    "host/test_infrastructure.cpp",
    "host/test_serialization.cpp",
]
for relative in required_tests:
    if not (root / "tests" / relative).exists():
        errors.append(f"missing tests/{relative}")

for example in sorted((root / "examples").glob("*/*.ino")):
    text = example.read_text(errors="ignore")
    if example.stat().st_size < 1000:
        errors.append(f"{example.relative_to(root)} is still only a skeleton")
    if "void setup" not in text or "void loop" not in text:
        errors.append(f"{example.relative_to(root)} is missing setup/loop")

for path in (root / "src").glob("Astro*.*"):
    if path.suffix in {".h", ".hpp", ".hh", ".cpp"} and "TODO" in path.read_text(errors="ignore"):
        errors.append(f"non-UI TODO remains in {path.relative_to(root)}")

ui_todos = 0
for ui_root in [root / "src/shared", root / "src/min", root / "src/full"]:
    if ui_root.exists():
        ui_todos += sum("TODO" in p.read_text(errors="ignore") for p in ui_root.rglob("*") if p.is_file())
if ui_todos < 10:
    errors.append("UI WIP placeholders unexpectedly missing")

trie_check = subprocess.run(
    [sys.executable, str(root / "tests/generate_enum_trie.py"), "--check"],
    cwd=root, capture_output=True, text=True,
)
if trie_check.returncode:
    errors.append(trie_check.stdout.strip() or trie_check.stderr.strip() or "enum trie generation check failed")

if errors:
    for error in errors:
        print("FAIL:", error)
    sys.exit(1)

print("PASS source validation")
print(trie_check.stdout.strip())
