#!/usr/bin/env python3
"""Create and check the library, demo-source, and firmware ZIP packages."""

from __future__ import annotations

import hashlib
import json
import posixpath
import re
from pathlib import Path
from urllib.parse import unquote
from zipfile import ZIP_DEFLATED, ZipFile, ZipInfo

from verify_release import verify

ROOT = Path(__file__).resolve().parents[1]
DIST = ROOT / "dist"
VERSION = json.loads((ROOT / "firmware" / "manifest.json").read_text(encoding="utf-8"))["version"]
STAMP = (1980, 1, 1, 0, 0, 0)
PRIVATE_PATTERNS = (
    re.compile(rb"[A-Za-z]:[/\\]Users[/\\][A-Za-z0-9_. -]+", re.IGNORECASE),
    re.compile(rb"/(?:home|Users)/[A-Za-z0-9_.-]+", re.IGNORECASE),
    re.compile(rb"-----BEGIN[ A-Z0-9_-]*PRIVATE KEY-----", re.IGNORECASE),
    re.compile(rb"\bCOM[0-9]{1,3}\b", re.IGNORECASE),
)
MARKDOWN_LINK = re.compile(r"\[[^\]]+\]\(([^)]+)\)")
REQUIRED_LICENSE_TEXTS = {
    "Adafruit-GFX-BSD.txt", "Arduino-ESP32-LGPL-2.1.txt",
    "Bodmer-TFT_eSPI-Notices.txt", "ESP-IDF-5.5-Apache-2.0.txt",
    "ESP-IDF-5.5-COPYRIGHT.rst", "EspUsbHost-MIT.txt",
    "FatFs-ChaN.txt", "FreeRTOS-MIT.txt", "GCC-GPL-3.0.txt",
    "GCC-Runtime-Library-Exception-3.1.txt", "LovyanGFX-Notices.txt",
    "lwIP-BSD.txt", "M5Cardputer-Adafruit-TCA8418-BSD.txt",
    "M5GFX-efont.txt", "M5GFX-GFXFF.txt", "M5GFX-IPA-Font.txt",
    "M5GFX-miniz-Unlicense.txt", "M5GFX-MIT.txt",
    "M5GFX-TJpgDec-ChaN.txt", "M5GFX-TomThumb-BSD-3.txt",
    "M5Unified-MIT.txt", "Newlib-COPYING.NEWLIB.txt",
    "Pngle-MIT.txt", "QOI-MIT.txt", "QRCode-MIT.txt", "result-MIT.txt",
}


def files_under(relative: str) -> list[Path]:
    base = ROOT / relative
    return sorted(path for path in base.rglob("*") if path.is_file()
                  and "__pycache__" not in path.parts and ".build" not in path.parts)


def paths_as_members(paths: list[Path]) -> list[tuple[Path, str]]:
    return [(path, path.relative_to(ROOT).as_posix()) for path in paths]


def archive(name: str, prefix: str, members: list[tuple[Path, str]]) -> Path:
    target = DIST / name
    seen: set[str] = set()
    with ZipFile(target, "w", compression=ZIP_DEFLATED, compresslevel=9) as zf:
        for path, internal in sorted(members, key=lambda item: item[1]):
            if not path.is_file():
                raise FileNotFoundError(path)
            if internal in seen or internal.startswith("/") or ".." in internal.split("/"):
                raise ValueError(f"Invalid archive path: {internal}")
            seen.add(internal)
            info = ZipInfo(f"{prefix}/{internal}", STAMP)
            info.compress_type = ZIP_DEFLATED
            info.external_attr = 0o644 << 16
            zf.writestr(info, path.read_bytes(), compress_type=ZIP_DEFLATED,
                        compresslevel=9)
    return target


def check_archive(path: Path) -> None:
    with ZipFile(path) as zf:
        bad = zf.testzip()
        if bad:
            raise ValueError(f"ZIP CRC failed: {path.name}:{bad}")
        members = set(zf.namelist())
        for member in sorted(members):
            data = zf.read(member)
            if member.lower().endswith((".bin", ".zip")):
                continue  # verify() checks the actual firmware and nested source archive.
            if any(pattern.search(data) for pattern in PRIVATE_PATTERNS):
                raise ValueError(f"Private-looking text in {path.name}:{member}")
            if not member.endswith(".md"):
                continue
            text = data.decode("utf-8")
            for link in MARKDOWN_LINK.findall(text):
                if link.startswith(("https:", "http:", "mailto:", "#")):
                    continue
                target = unquote(link.split("#", 1)[0])
                if not target:
                    continue
                resolved = posixpath.normpath(posixpath.join(
                    posixpath.dirname(member), target))
                if resolved not in members and not any(
                    item.startswith(resolved.rstrip("/") + "/") for item in members
                ):
                    raise ValueError(f"Broken archive link: {member} -> {link}")


def main() -> None:
    verify()
    DIST.mkdir(exist_ok=True)

    license_file = ROOT / "LICENSE"
    if not license_file.is_file():
        raise FileNotFoundError("Missing project LICENSE")
    license_members = [(license_file, "LICENSE")]
    third_party_licenses = paths_as_members(files_under("LICENSES"))
    missing_licenses = REQUIRED_LICENSE_TEXTS - {path.name for path, _ in third_party_licenses}
    if missing_licenses:
        raise FileNotFoundError(f"Missing third-party license texts: {sorted(missing_licenses)}")
    upstream_source_members = paths_as_members(files_under("sources"))
    notices = (ROOT / "THIRD_PARTY_NOTICES.md", "THIRD_PARTY_NOTICES.md")
    if not notices[0].is_file():
        raise FileNotFoundError("Missing third-party notices")

    library_root = ROOT / "library" / "WeActUSBDisplay"
    library_files = files_under("library/WeActUSBDisplay")
    library_members = [
        (path, path.relative_to(library_root).as_posix()) for path in library_files
    ] + [notices] + license_members + third_party_licenses

    demo_files = files_under("demo/CardputerADV_WeActDemo")
    source_tools = [ROOT / "tools" / name for name in (
        "build-demo.ps1", "run_native_tests.py"
    )]
    source_members = paths_as_members(
        library_files + demo_files + files_under("docs")
        + files_under("tests") + source_tools
    ) + [
        (ROOT / "demo" / "README.md", "README.md"),
        (ROOT / ".gitignore", ".gitignore"),
        (ROOT / ".gitattributes", ".gitattributes"),
        notices,
    ] + license_members + third_party_licenses + upstream_source_members

    firmware_files = [path for path in files_under("firmware")
                      if path.name != "README.md"]
    firmware_docs = [ROOT / "docs" / f"{name}.md" for name in (
        "BUILD", "FLASH", "HARDWARE", "QUICKSTART", "TROUBLESHOOTING", "VALIDATION"
    )]
    firmware_members = paths_as_members(
        firmware_files + firmware_docs + library_files + demo_files
        + [ROOT / "tools" / name for name in (
            "build-demo.ps1", "verify_release.py", "verify_build_match.py"
        )]
    ) + [
        (ROOT / "firmware" / "README.md", "README.md"),
        notices,
    ] + license_members + third_party_licenses + upstream_source_members

    packages = (
        archive(f"PREVIEW-WeActUSBDisplay-Library-{VERSION}.zip",
                "WeActUSBDisplay", library_members),
        archive(f"PREVIEW-CardputerADV-WeActDemo-Source-{VERSION}.zip",
                "CardputerADV-WeActDemo-Source", source_members),
        archive(f"PREVIEW-CardputerADV-WeActDemo-Firmware-{VERSION}.zip",
                "CardputerADV-WeActDemo-Firmware", firmware_members),
    )
    for path in packages:
        check_archive(path)
        print(f"PASS {path.name} ({path.stat().st_size:,} bytes)")

    sums = DIST / "SHA256SUMS.txt"
    sums.write_text("".join(
        f"{hashlib.sha256(path.read_bytes()).hexdigest()}  {path.name}\n"
        for path in packages
    ), encoding="utf-8")
    print(f"PASS {sums.name}")


if __name__ == "__main__":
    main()
