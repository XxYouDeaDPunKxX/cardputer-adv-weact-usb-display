# Cardputer ADV + WeAct USB Display firmware

This package contains a complete 8 MiB factory image for the M5Stack Cardputer ADV (ESP32-S3) and WeAct Display FS V1 0.96-inch screen.

## ⚠️ Before installing

The image includes the bootloader, partition table, application, and erased NVS. Flashing at address `0x0` overwrites the device's **entire 8 MiB flash**, including its current firmware and settings.

Read `docs/HARDWARE.md` to confirm the target, then follow `docs/FLASH.md` for flashing and verification commands. The demo controls are in `docs/QUICKSTART.md`; connection help is in `docs/TROUBLESHOOTING.md`.

## Firmware identity and checks

The factory image includes telemetry-driven animation and the portrait proportion correction. The demo has been flashed and tested on a Cardputer ADV with the WeAct panel. See `docs/VALIDATION.md` for the validation record.

`firmware/manifest.json` records the release version (`0.1.0`), SHA-256 checksums, and build identity. Its `build_id` uses the first 12 hexadecimal digits of the application's SHA-256. `source_revision_sha256` identifies the demo, display-library, and build-script inputs independently of Git history.

Run `python tools/verify_release.py` after extraction to check the image layout, four firmware hashes, blank NVS, and Arduino source snapshot. Rebuilding the sketch produces a new executable; the manifest identifies the supplied image independently of a local rebuild.

## 📦 Source and licenses

The package includes the demo and display-library source, build script, verified Arduino-ESP32 3.3.2 core/FS source snapshot in `sources/`, and license texts in `LICENSE` and `LICENSES/`.

Read `docs/BUILD.md` to rebuild with a modified Arduino core. The MIT license covers original project code; the firmware also contains LGPL and other third-party components. See `THIRD_PARTY_NOTICES.md` before redistributing the binaries.
