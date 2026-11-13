# Validation

## Supported target

The firmware package targets the M5Stack Cardputer ADV (ESP32-S3, 8 MiB flash) and the WeAct Display FS V1 0.96-inch panel with the observed `V1.0.0.0` firmware. Other boards and panel versions have not been validated.

## What has been checked

| Check | Result |
| --- | --- |
| Factory image structure | 8 MiB image with bootloader at `0x0`, partition table at `0x8000`, and application at `0x10000` |
| Firmware digests and identity | SHA-256 values, `build_id` and source-input revision recorded in `firmware/manifest.json`; checked by `tools/verify_release.py` |
| Partition contents | Expected layout and erased NVS area (`0x9000`–`0xCFFF`) |
| Arduino-ESP32 source snapshot | SHA-256, ZIP integrity, and 136 source/build files checked; the selected source bytes match the official 3.3.2 platform package |
| Hardware operation | The demo has been flashed and exercised on a Cardputer ADV with the WeAct panel. Animation and display controls operate on the target hardware. |
| Driver protocol | Native C++ tests for command encoding, four orientations, bounds, and transport behavior |
| Demo logic | Native C++ tests for menu actions, frame layout, recovery, telemetry effects, and portrait-vs-landscape pixel proportions (quarter-turn comparison) |
| Arduino build | Current demo and standalone `ColorCycle` example compiled using Arduino-ESP32 3.3.2 and EspUsbHost 2.10.1 |
| Rebuild consistency | The source revision and distributed firmware hashes are recorded separately. Builds on different machines can embed different source paths and compilation timestamps, so identical binary bytes are not claimed. |
| Modified LGPL core rebuild | On 2026-11-12, a clean build of the demo with a modified `esp32-hal-misc.c` from an isolated Arduino-ESP32 3.3.2 copy completed; compiler marker and linker map confirm that the modified core was compiled and linked |

From the repository, run both `python tools/run_native_tests.py` and `python tools/verify_release.py`. The demo-source ZIP supports the first command; the firmware ZIP supports the second. CI compiles the Cardputer demo and independent `ColorCycle` example, runs the native tests, and verifies the supplied firmware and ZIP packages. The packaged factory image remains separate from freshly compiled output.

## Hardware test scope

The demonstrated hardware setup is a Cardputer ADV connected to the WeAct Display FS V1 0.96-inch panel. The demo was exercised with animation, rotation, and on-device controls. Other board and panel variants, as well as the experimental protocol operations identified in the API reference, are outside the tested scope.

The modified-core build produced a separate application image for desktop
verification only. It was not flashed or physically tested; the original
toolchain remained unchanged. The source snapshot
and rebuild procedure are provided in [the source bundle](../sources/README.md)
and [the build guide](BUILD.md). The third-party component and license inventory
is in [third-party notices](../THIRD_PARTY_NOTICES.md).

If you build a different firmware image, test that image on hardware before distributing it.
