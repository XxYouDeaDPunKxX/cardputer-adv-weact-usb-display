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
| Device flashing | The telemetry/rotation demo has been reported working on a Cardputer ADV. The test record does not include a flash readback identifying the exact SHA-256 of the currently supplied factory image; acceptance of that specific image is still pending. |
| Driver protocol | Native C++ tests for command encoding, four orientations, bounds, and transport behavior |
| Demo logic | Native C++ tests for menu actions, frame layout, recovery, telemetry effects, and portrait-vs-landscape pixel proportions (quarter-turn comparison) |
| Arduino build | Current demo and standalone `ColorCycle` example compiled using Arduino-ESP32 3.3.2 and EspUsbHost 2.10.1 |
| Binary/source correspondence | A fresh demo build matches the shipped application's executable bytes when the ESP-IDF ELF hash and image footer metadata are excluded; checked by `tools/verify_build_match.py`. This does not identify which bytes were installed on the user's hardware. |
| Modified LGPL core rebuild | On 2026-11-12, a clean build of the demo with a modified `esp32-hal-misc.c` from an isolated Arduino-ESP32 3.3.2 copy completed; compiler marker and linker map confirm that the modified core was compiled and linked |

The checks can be run with `python tools/run_native_tests.py` and `python tools/verify_release.py`. After compiling the sketch, use `python tools/verify_build_match.py .build/arduino-output/CardputerADV_WeActDemo.ino.bin` to compare the rebuilt executable with the packaged application. The packaged factory image remains separate from freshly compiled output. The CI also compiles both Arduino sketches and compares the demo build.

## Hardware acceptance still needed

The current image combines the telemetry-driven animation with the portrait aspect-ratio correction. A device test linked to the exact `CardputerADV_WeActDemo.factory-8MB.bin` SHA-256 in the manifest is needed, including all orientations (especially both reverse modes), the Cardputer TFT menu, brightness and screen power, pause/resume, IMU response, cold boot, disconnect/reconnect, and frame pacing. Native pixel comparisons and USB transfer completion alone cannot establish how every operation appears on the panel.

The modified-core build produced a separate application image for desktop
verification only. It was not flashed or physically tested; the original
toolchain remained unchanged. The source snapshot
and rebuild procedure are provided in [the source bundle](../sources/README.md)
and [the build guide](BUILD.md). The third-party component and license inventory
is in [third-party notices](../THIRD_PARTY_NOTICES.md).

If you build a different firmware image, test that image on hardware before distributing it.
