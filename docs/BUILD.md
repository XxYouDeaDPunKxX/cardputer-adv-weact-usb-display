# Build the Cardputer ADV demo

The Arduino sketch is in `demo/CardputerADV_WeActDemo/CardputerADV_WeActDemo.ino`. It uses the WeActUSBDisplay library, M5Cardputer for the built-in interface, and EspUsbHost for USB CDC communication.

## Dependencies

| Dependency | Version used |
| --- | --- |
| Arduino-ESP32 board core | 3.3.2 |
| EspUsbHost | 2.10.1 |
| M5Cardputer | 1.1.1 |
| M5Unified | 0.2.17 |
| M5GFX | 0.2.23 |

Install Arduino CLI, the board core, and these libraries. The Espressif board index is `https://espressif.github.io/arduino-esp32/package_esp32_index.json`.

```powershell
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.2 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install "EspUsbHost@2.10.1"
arduino-cli lib install "M5Cardputer@1.1.1"
arduino-cli lib install "M5Unified@0.2.17"
arduino-cli lib install "M5GFX@0.2.23"
.\tools\build-demo.ps1
```

The PowerShell script uses `arduino-cli` from `PATH`. Use `-Cli`, `-Config`, or `-EspUsbHostDir` if your installation is elsewhere. Build output goes to `.build/` and does not overwrite the supplied firmware image.

GitHub Actions compiles the demo and the independent `ColorCycle` example with these pinned dependencies. It also runs native C++ tests and verifies the supplied firmware hashes and image layout.

The `firmware/manifest.json` file records the hashes of the distributed binaries and a separate revision hash for the build inputs. Rebuilding creates a new image; its bytes can differ because the toolchain embeds source paths and compilation timestamps. The **demo-source ZIP** contains the project source and native tests, while the **firmware ZIP** also includes the prebuilt image and its integrity checks.

The board profile is `esp32:esp32:m5stack_cardputer` with `USBMode=hwcdc`, `CDCOnBoot=cdc`, `FlashSize=8M`, `PartitionScheme=custom`, and `PSRAM=disabled`. The custom `partitions.csv` is beside the sketch.

You can also open the sketch in Arduino IDE after installing the board core and libraries.

## Rebuilding with a modified Arduino core

The firmware uses Arduino-ESP32 3.3.2, which contains LGPL-covered code.
The [corresponding source snapshot](../sources/README.md) is included in the
repository so recipients can modify the Arduino core and rebuild the demo.
The snapshot is extracted from the official Arduino-ESP32 3.3.2 platform
archive and contains the complete `cores/esp32` source, the `FS` library,
the Cardputer variant header and build metadata.

First install the pinned dependencies above. Use `arduino-cli config dump`
to locate the Arduino CLI data directory, then find
`packages/esp32/hardware/esp32/3.3.2` inside it. The source tree in
`sources/arduino-esp32-3.3.2-corresponding-source.zip` contains these
source files with their original relative paths. Modify the installed
Arduino core source as desired, then run
`tools/build-demo.ps1` again. A rebuilt application is a new firmware
image and must pass its own checks before distribution.

The full project source, partition table and build script are supplied
alongside the core archive. See [third-party notices](../THIRD_PARTY_NOTICES.md)
for the applicable LGPL terms and other component licenses.

This rebuild path was tested on a separate Arduino-ESP32 3.3.2 installation
with a modified `cores/esp32/esp32-hal-misc.c`. The compiler used the modified
source and produced a linked application image. This test image was not flashed
or substituted for the supplied firmware.

## Tests

```powershell
python tools/run_native_tests.py
```

The native tests are included in the demo source ZIP. They require a C++17 compiler; on Windows, use an x64 Developer Command Prompt for MSVC or another shell with `g++` or `clang++` available.

From a complete repository checkout that also contains the supplied firmware binaries, you can check and package those binaries:

```powershell
python tools/verify_release.py
python tools/package_release.py
```

`verify_release.py` checks the supplied firmware binaries. Changes to the current demo or library source are tested separately and do not alter that image. A rebuilt image needs its own hardware verification.
