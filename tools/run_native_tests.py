#!/usr/bin/env python3
"""Build and run native C++ tests for the display driver and demo."""

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / ".build" / "native"
TESTS = ("weact-api-tests.cpp", "demo-tests.cpp")


def find_compiler(requested: str | None) -> str:
    if requested:
        found = shutil.which(requested)
        if found:
            return found
        path = Path(requested)
        if path.is_file():
            return str(path)
        raise SystemExit(f"Compiler not found: {requested}")
    for name in ("g++", "clang++", "cl"):
        found = shutil.which(name)
        if found:
            return found
    raise SystemExit(
        "No C++ compiler on PATH. Install g++/clang++ or start an x64 "
        "Visual Studio Developer Command Prompt for cl.exe."
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", help="Optional compiler command/path")
    args = parser.parse_args()
    compiler = find_compiler(args.compiler)
    OUT.mkdir(parents=True, exist_ok=True)
    lib = ROOT / "library" / "WeActUSBDisplay" / "src"
    demo = ROOT / "demo" / "CardputerADV_WeActDemo"
    msvc = Path(compiler).name.lower() in ("cl", "cl.exe")

    for source in TESTS:
        src = ROOT / "tests" / source
        exe = OUT / (src.stem + (".exe" if sys.platform == "win32" else ""))
        if msvc:
            command = [
                compiler, "/nologo", "/std:c++17", "/EHsc", "/W4",
                f"/I{lib}", f"/I{demo}", f"/Fe:{exe}", str(src),
            ]
        else:
            command = [
                compiler, "-std=c++17", "-Wall", "-Wextra", "-O2",
                "-I", str(lib), "-I", str(demo), str(src), "-o", str(exe),
            ]
        print("BUILD", source, flush=True)
        subprocess.run(command, cwd=ROOT, check=True)
        print("RUN", exe.name, flush=True)
        subprocess.run([str(exe)], cwd=ROOT, check=True)
    print(f"PASS {len(TESTS)} native C++ test programs")


if __name__ == "__main__":
    main()

