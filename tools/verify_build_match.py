#!/usr/bin/env python3
"""Compare a fresh Arduino build with the shipped app, excluding ESP metadata."""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

from verify_release import FIRMWARE, verify


def check_image(image: bytes, name: str) -> None:
    if len(image) < 0xD0 + 33 or image[0] != 0xE9:
        raise ValueError(f"{name}: invalid ESP application image")
    if hashlib.sha256(image[:-32]).digest() != image[-32:]:
        raise ValueError(f"{name}: invalid appended ESP image SHA-256")


def compare_binaries(compiled: Path) -> None:
    reference = (FIRMWARE / "CardputerADV_WeActDemo.app.bin").read_bytes()
    rebuilt = compiled.read_bytes()
    check_image(reference, "shipped app")
    check_image(rebuilt, "compiled app")
    if len(reference) != len(rebuilt):
        raise ValueError(
            f"compiled application size differs from shipped image: "
            f"built {len(rebuilt)} bytes, shipped {len(reference)} bytes"
        )

    # ESP-IDF app descriptor's app_elf_sha256 is at 0xB0..0xCF.
    # The final 33 bytes are the ESP image checksum and appended SHA-256.
    # ELF hashes can vary with paths/debug info even when all flash code matches.
    if reference[:0xB0] != rebuilt[:0xB0] or reference[0xD0:-33] != rebuilt[0xD0:-33]:
        raise ValueError("compiled executable contents differ from shipped app")
    print("PASS rebuilt app matches shipped executable bytes (ESP metadata excluded)")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("compiled_app", type=Path, help="Arduino-CLI output .ino.bin")
    args = parser.parse_args()
    verify()
    compare_binaries(args.compiled_app)


if __name__ == "__main__":
    main()
