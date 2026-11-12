#!/usr/bin/env python3
"""Verify the packaged Cardputer ADV factory firmware image."""

from __future__ import annotations

import hashlib
import json
import re
import struct
from pathlib import Path
from zipfile import ZipFile

ROOT = Path(__file__).resolve().parents[1]
FIRMWARE = ROOT / "firmware"
MANIFEST = FIRMWARE / "manifest.json"
SOURCE_ARCHIVE = ROOT / "sources" / "arduino-esp32-3.3.2-corresponding-source.zip"
SOURCE_BYTES = 509236
SOURCE_SHA256 = "8e2c41cfc16736d4a0844f038c7e32df9bde8c6691f2b2a81a7f38f06c9c4be8"
EXPECTED_FILES = {
    "CardputerADV_WeActDemo.factory-8MB.bin",
    "CardputerADV_WeActDemo.app.bin",
    "CardputerADV_WeActDemo.bootloader.bin",
    "CardputerADV_WeActDemo.partitions.bin",
}
SOURCE_ROOTS = (
    ROOT / "demo" / "CardputerADV_WeActDemo",
    ROOT / "library" / "WeActUSBDisplay" / "src",
)
SOURCE_EXTRA = (
    ROOT / "library" / "WeActUSBDisplay" / "library.properties",
    ROOT / "tools" / "build-demo.ps1",
)
EXPECTED_PARTITIONS = {
    "nvs": (1, 2, 0x9000, 0x4000),
    "otadata": (1, 0, 0xD000, 0x2000),
    "phy_init": (1, 1, 0xF000, 0x1000),
    "factory": (0, 0, 0x10000, 0x600000),
    "ota_0": (0, 0x10, 0x610000, 0x1F0000),
}


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def source_revision_sha256() -> str:
    """Hash the build inputs, including relative names, without relying on Git history."""
    source_files = set(SOURCE_EXTRA)
    for root in SOURCE_ROOTS:
        source_files.update(path for path in root.rglob("*") if path.is_file()
                            and path.suffix.lower() in {".ino", ".h", ".hpp", ".c", ".cpp", ".csv"})
    digest = hashlib.sha256()
    for path in sorted(source_files, key=lambda item: item.relative_to(ROOT).as_posix()):
        require(path.is_file(), f"missing source input: {path}")
        raw = path.read_bytes()
        relative = path.relative_to(ROOT).as_posix().encode("utf-8")
        digest.update(relative + b"\0" + str(len(raw)).encode("ascii") + b"\0" + raw)
    return digest.hexdigest()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def parse_partitions(table: bytes) -> dict[str, tuple[int, int, int, int]]:
    partitions: dict[str, tuple[int, int, int, int]] = {}
    require(len(table) == 0xC00, "partition-table binary must be 0xC00 bytes")
    for offset in range(0, len(table), 32):
        magic, kind, subtype, start, length, name, flags = struct.unpack_from(
            "<HBBII16sI", table, offset
        )
        if magic in (0xEBEB, 0xFFFF):
            break
        require(magic == 0x50AA and flags == 0, "invalid partition record")
        label = name.split(b"\x00", 1)[0].decode("ascii")
        require(label not in partitions, f"duplicate partition: {label}")
        partitions[label] = (kind, subtype, start, length)
    return partitions


def verify() -> None:
    require(SOURCE_ARCHIVE.is_file(), "missing Arduino-ESP32 corresponding source archive")
    source = SOURCE_ARCHIVE.read_bytes()
    require(len(source) == SOURCE_BYTES, "Arduino-ESP32 source archive size mismatch")
    require(sha256(source) == SOURCE_SHA256, "Arduino-ESP32 source archive SHA-256 mismatch")
    with ZipFile(SOURCE_ARCHIVE) as upstream:
        require(upstream.testzip() is None, "Arduino-ESP32 source archive CRC failed")
        names = upstream.namelist()
        require(
            len(names) == 136
            and "esp32-3.3.2/cores/esp32/Arduino.h" in names
            and "esp32-3.3.2/libraries/FS/src/FS.cpp" in names
            and "esp32-3.3.2/variants/m5stack_cardputer/pins_arduino.h" in names
            and "esp32-3.3.2/platform.txt" in names
            and not any(name.lower().endswith((".bin", ".exe", ".a", ".elf", ".o"))
                        for name in names),
            "Arduino-ESP32 corresponding source snapshot is incomplete",
        )

    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    require(manifest.get("version") == "0.1.0", "unexpected release version")
    require(manifest.get("board") == "M5Stack Cardputer ADV / ESP32-S3 / 8 MiB",
            "unexpected board in manifest")
    files = manifest.get("artifacts", {})
    require(set(files) == EXPECTED_FILES, "manifest artifact set changed")
    app_sha256 = files["CardputerADV_WeActDemo.app.bin"].get("sha256", "")
    require(manifest.get("build_id") == f"{manifest['version']}+app.{app_sha256[:12]}",
            "build ID does not identify the shipped app image")
    require(manifest.get("source_revision_sha256") == source_revision_sha256(),
            "build input revision differs from the pinned source snapshot")

    data: dict[str, bytes] = {}
    for name, expected in files.items():
        path = FIRMWARE / name
        require(path.is_file(), f"missing firmware artifact: {name}")
        raw = path.read_bytes()
        require(len(raw) == expected["bytes"], f"size mismatch: {name}")
        require(sha256(raw) == expected["sha256"], f"SHA-256 mismatch: {name}")
        data[name] = raw

    image = data["CardputerADV_WeActDemo.factory-8MB.bin"]
    app = data["CardputerADV_WeActDemo.app.bin"]
    boot = data["CardputerADV_WeActDemo.bootloader.bin"]
    table = data["CardputerADV_WeActDemo.partitions.bin"]
    require(len(image) == 8 * 1024 * 1024, "factory image must be exactly 8 MiB")
    require(app[0] == boot[0] == 0xE9, "ESP image magic mismatch")
    require(image[: len(boot)] == boot, "bootloader does not match factory image")
    require(image[0x8000:0x8000 + len(table)] == table,
            "partition table does not match factory image")
    require(image[0x10000:0x10000 + len(app)] == app,
            "application does not match factory image")
    require(len(app) <= 0x600000, "application exceeds factory partition")
    require(parse_partitions(table) == EXPECTED_PARTITIONS,
            "partition layout differs from verified build")

    for start, end, label in (
        (len(boot), 0x8000, "boot padding"),
        (0x8000 + len(table), 0x9000, "partition padding"),
        (0x9000, 0xD000, "NVS"),
        (0xD000, 0xE000, "OTA data"),
        (0xF004, 0x10000, "PHY padding"),
        (0x10000 + len(app), len(image), "unused flash"),
    ):
        require(all(byte == 0xFF for byte in image[start:end]),
                f"{label} is not erased")

    require(
        not re.search(
            rb"(?:[A-Za-z]:[/\\]Users[/\\][A-Za-z0-9_. -]{2,}"
            rb"|/(?:home|Users)/[A-Za-z0-9_.-]{2,}"
            rb"|-----BEGIN[ A-Z0-9_-]*PRIVATE KEY-----)",
            image,
            re.IGNORECASE,
        ),
        "possible private data in factory image",
    )
    require(manifest.get("nvs") == {"offset": "0x9000", "size": "0x4000",
                                   "state": "erased FF"}, "NVS metadata mismatch")
    print("PASS factory: 8 MiB, bootloader, partitions, app, blank NVS")
    print(f"PASS SHA-256: {len(files)} firmware artifacts")
    print(f"PASS build ID and source revision: {manifest['build_id']}")
    print("PASS Arduino-ESP32 3.3.2 core/FS source snapshot")


if __name__ == "__main__":
    verify()

