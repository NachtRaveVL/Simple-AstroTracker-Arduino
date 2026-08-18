#!/usr/bin/env python3
from pathlib import Path
import shutil
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
compiler = shutil.which("g++") or shutil.which("c++")
if not compiler:
    print("No host C++ compiler found")
    raise SystemExit(1)

failures = []
for example in sorted((root / "examples").glob("*/*.ino")):
    command = [
        compiler, "-std=c++17", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
        "-x", "c++", "-fsyntax-only",
        "-I", str(root / "src"),
        "-include", str(root / "tests/host/arduino_example_stubs.h"),
        str(example),
    ]
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode:
        failures.append((example.relative_to(root), result.stderr.strip()))

if failures:
    for example, output in failures:
        print(f"FAIL: {example}")
        print(output)
    raise SystemExit(1)

print(f"PASS example syntax ({len(list((root / 'examples').glob('*/*.ino')))} sketches)")
