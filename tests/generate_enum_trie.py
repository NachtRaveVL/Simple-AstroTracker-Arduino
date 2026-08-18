#!/usr/bin/env python3
"""Generate Astruino's compact enum string decoders from AstroUtils.cpp."""

from __future__ import annotations

import argparse
import functools
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
UTILS = ROOT / "src" / "AstroUtils.cpp"
OUTPUT = ROOT / "src" / "AstroEnumTrie.h"
STRINGS = ROOT / "src" / "AstroStrings.cpp"

SPECS = [
    ("systemModeToString", "Astro_SystemMode", "astroDecodeSystemMode", "systemModeStr", "Astro_SystemMode_Undefined"),
    ("measurementModeToString", "Astro_MeasurementMode", "astroDecodeMeasurementMode", "measurementModeStr", "Astro_MeasurementMode_Undefined"),
    ("actuatorTypeToString", "Astro_ActuatorType", "astroDecodeActuatorType", "actuatorTypeStr", "Astro_ActuatorType_Undefined"),
    ("sensorTypeToString", "Astro_SensorType", "astroDecodeSensorType", "sensorTypeStr", "Astro_SensorType_Undefined"),
    ("mountTypeToString", "Astro_MountType", "astroDecodeMountType", "mountTypeStr", "Astro_MountType_Unknown"),
    ("railTypeToString", "Astro_RailType", "astroDecodeRailType", "railTypeStr", "Astro_RailType_Undefined"),
    ("pinModeToString", "Astro_PinMode", "astroDecodePinMode", "pinModeStr", "Astro_PinMode_Undefined"),
    ("enableModeToString", "Astro_EnableMode", "astroDecodeEnableMode", "enableModeStr", "Astro_EnableMode_Undefined"),
    ("unitsCategoryToString", "Astro_UnitsCategory", "astroDecodeUnitsCategory", "unitsCategoryStr", "Astro_UnitsCategory_Undefined"),
    ("unitsTypeToSymbol", "Astro_UnitsType", "astroDecodeUnitsType", "unitsSymbolStr", "Astro_UnitsType_Undefined"),
    ("targetClassToString", "Astro_TargetClass", "astroDecodeTargetClass", "targetClassStr", "Astro_TargetClass_Unknown"),
    ("thermalModeToString", "Astro_ThermalMode", "astroDecodeThermalMode", "thermalModeStr", "Astro_ThermalMode_Undefined"),
    ("schedulerStageToString", "Astro_SchedulerStage", "astroDecodeSchedulerStage", "schedulerStageStr", "Astro_SchedulerStage_Undefined"),
]

ALIASES = {
    "unitsTypeToSymbol": [("J/s", "Astro_UnitsType_Power_Wattage")],
}


def function_body(source: str, name: str) -> str:
    start = source.index(f"AstroString {name}(")
    brace = source.index("{", start)
    depth = 1
    pos = brace + 1
    while depth and pos < len(source):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
        pos += 1
    if depth:
        raise RuntimeError(f"Unable to parse {name}")
    return source[brace + 1:pos - 1]


def string_values() -> dict[str, str]:
    source = STRINGS.read_text()
    values: dict[str, str] = {}
    pattern = re.compile(
        r'case\s+(AStr_[A-Za-z0-9_]+):\s*\{\s*'
        r'static const char flashStr\[\] PROGMEM = "([^"\\]*(?:\\.[^"\\]*)*)";',
        re.S,
    )
    for enum_name, escaped in pattern.findall(source):
        values[enum_name] = bytes(escaped, "utf-8").decode("unicode_escape")
    return values


def values_for(source: str, name: str, default_enum: str) -> list[tuple[str, str]]:
    body = function_body(source, name)
    strings = string_values()
    pairs: list[tuple[str, str]] = []

    # Current code returns SFP(AStr_*) values so enum strings stay in program Flash.
    for enum_name, str_name in re.findall(
        r'case\s+(Astro_[A-Za-z0-9_]+):\s*return[^;]*SFP\((AStr_[A-Za-z0-9_]+)\)',
        body,
    ):
        if str_name not in strings:
            raise RuntimeError(f"Missing Flash string {str_name} used by {name}")
        pairs.append((strings[str_name], enum_name))

    # Retain support for literal returns in case a future enum intentionally does not use SFP.
    for enum_name, text in re.findall(
        r'case\s+(Astro_[A-Za-z0-9_]+):\s*return[^;]*AstroString\("([^"\\]*)"\)',
        body,
    ):
        pairs.append((text, enum_name))

    values = list(pairs)
    values.append(("Undefined", default_enum))
    values.extend(ALIASES.get(name, []))

    by_text: dict[str, str] = {}
    for text, enum_name in values:
        by_text[text] = enum_name
    return sorted(by_text.items())


def char_at(text: str, pos: int) -> str:
    return text[pos] if pos < len(text) else "\0"


def pick_tree(values: tuple[tuple[str, str], ...]):
    max_len = max(len(text) for text, _ in values)

    @functools.lru_cache(maxsize=None)
    def solve(indices: tuple[int, ...], positions: tuple[int, ...]):
        if len(indices) == 1:
            return (1, ("return", values[indices[0]][1]))

        best = None
        for pos in positions:
            groups: dict[str, list[int]] = {}
            for index in indices:
                groups.setdefault(char_at(values[index][0], pos), []).append(index)
            if len(groups) <= 1:
                continue

            remaining = tuple(p for p in positions if p != pos)
            children = []
            cost = 2 + len(groups)
            valid = True
            for ch, group in sorted(groups.items()):
                child_cost, child = solve(tuple(group), remaining)
                if child is None:
                    valid = False
                    break
                cost += child_cost
                children.append((ch, child))
            if valid:
                candidate = (cost, pos, ("switch", pos, tuple(children)))
                if best is None or candidate[:2] < best[:2]:
                    best = candidate

        if best is None:
            return (10**9, None)
        return (best[0], best[2])

    return solve(tuple(range(len(values))), tuple(range(max_len + 1)))[1]


def cpp_char(ch: str) -> str:
    if ch == "\0":
        return "'\\0'"
    if ch == "'":
        return "'\\\''"
    if ch == "\\":
        return "'\\\\'"
    return repr(ch)


def emit_tree(tree, var_name: str, default_enum: str, indent: int = 1) -> list[str]:
    pad = "    " * indent
    if tree[0] == "return":
        return [f"{pad}return {tree[1]};"]

    _, pos, children = tree
    lines = [f"{pad}switch ({var_name}.length() > {pos} ? {var_name}[{pos}] : '\\0') {{"]
    for ch, child in children:
        lines.append(f"{pad}    case {cpp_char(ch)}:")
        lines.extend(emit_tree(child, var_name, default_enum, indent + 2))
    lines.append(f"{pad}}}")
    lines.append(f"{pad}return {default_enum};")
    return lines


def generate() -> str:
    source = UTILS.read_text()
    lines = [
        "/*  Astruino enum decoding tree.",
        "    Generated by tests/generate_enum_trie.py. Do not hand edit.",
        "*/",
        "",
        "#ifndef AstroEnumTrie_H",
        "#define AstroEnumTrie_H",
        "",
        '#include "AstroDefines.h"',
        "",
    ]

    for to_string, enum_type, decoder, var_name, default_enum in SPECS:
        values = tuple(values_for(source, to_string, default_enum))
        tree = pick_tree(values)
        if tree is None:
            raise RuntimeError(f"Unable to build decoder for {to_string}")
        lines.append(f"inline {enum_type} {decoder}(const AstroString &{var_name})")
        lines.append("{")
        lines.extend(emit_tree(tree, var_name, default_enum))
        lines.append("}")
        lines.append("")

    lines.append("#endif // /ifndef AstroEnumTrie_H")
    lines.append("")
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    generated = generate()
    if args.check:
        current = OUTPUT.read_text() if OUTPUT.exists() else ""
        if current != generated:
            print("AstroEnumTrie.h is stale. Run tests/generate_enum_trie.py.")
            return 1
        print("AstroEnumTrie.h matches generated enum decoder.")
        return 0

    OUTPUT.write_text(generated)
    print(f"Wrote {OUTPUT.relative_to(ROOT)} ({len(generated)} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
