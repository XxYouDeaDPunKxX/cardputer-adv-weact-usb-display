# Third-party software notices

The repository's [MIT license](LICENSE) applies to the original WeActUSBDisplay
library, Cardputer demo code, tests, build tools, and documentation. Third-party
components retain their own copyrights and license terms. In particular, the
prebuilt files under `firmware/` contain separately licensed software and must
not be represented as an entirely MIT-licensed work.

## Software used to build the firmware

| Component | Version used | License evidence and copyright | Upstream |
| --- | --- | --- | --- |
| M5Cardputer | 1.1.1 | MIT SPDX headers, © 2025 M5Stack Technology CO LTD; includes the Adafruit TCA8418 material described below | [M5Cardputer](https://github.com/m5stack/M5Cardputer) |
| M5Unified | 0.2.17 | MIT, © 2021 M5Stack | [M5Unified LICENSE](https://github.com/m5stack/M5Unified/blob/master/LICENSE) |
| M5GFX | 0.2.23 | MIT, © 2021 M5Stack; includes additional components and fonts listed below | [M5GFX LICENSE](https://github.com/m5stack/M5GFX/blob/master/LICENSE) |
| EspUsbHost | 2.10.1 | MIT, © 2026 TANAKA Masayuki | [EspUsbHost LICENSE](https://github.com/tanakamasayuki/EspUsbHost/blob/main/LICENSE) |
| Arduino-ESP32 | 3.3.2 | LGPL-2.1 family: installed `package.json` says `LGPL-2.1-or-later`; `idf_component.yml` says `LGPL-2.1` | [Arduino-ESP32](https://github.com/espressif/arduino-esp32/tree/3.3.2) |
| ESP-IDF and precompiled ESP32 runtime components | ESP-IDF 5.5-based `esp32-arduino-libs` package bundled with Arduino-ESP32 3.3.2 | ESP-IDF framework is primarily Apache-2.0; bundled components have their own terms | [ESP-IDF v5.5](https://github.com/espressif/esp-idf/tree/v5.5) |

The project MIT terms appear in `LICENSE`. The `LICENSES/` directory holds
license texts and notices from the upstream projects listed above, including
the LGPL 2.1 text from the Arduino-ESP32 3.3.2 source tree and the
Apache-2.0 license plus copyright overview from ESP-IDF 5.5. These files
are included with each of the three ZIP packages.

### Components incorporated in the firmware

The ESP32-S3 application link map was inspected using the pinned toolchain
(Arduino-ESP32 3.3.2, ESP-IDF 5.5-based precompiled libraries, Xtensa GCC
14.2.0). The result includes Arduino core/FS, M5Stack and USB-host code,
ESP-IDF components and GNU runtime libraries. The supplied application image
differs from the local reference build only in build metadata and checksums;
the linked source and component set is the same.

| Linked component | Applicable terms | Notice or license text |
| --- | --- | --- |
| Arduino core and FS library | LGPL 2.1 family, plus file-specific licenses | `Arduino-ESP32-LGPL-2.1.txt`; source under `sources/` |
| ESP-IDF core, USB host, NVS, drivers and bootloader | Apache-2.0 with component-specific exceptions | `ESP-IDF-5.5-Apache-2.0.txt`, `ESP-IDF-5.5-COPYRIGHT.rst` |
| FreeRTOS kernel | MIT; Amazon.com, Inc. and contributors | `FreeRTOS-MIT.txt` |
| lwIP network stack | BSD-3-Clause; Swedish Institute of Computer Science and contributors | `lwIP-BSD.txt` |
| Newlib C library, including `libc` and `libm` | Multiple BSD-style notices | `Newlib-COPYING.NEWLIB.txt` |
| FatFs | ChaN's permissive source/binary terms | `FatFs-ChaN.txt` |
| Mbed TLS crypto routines | Apache-2.0 OR GPL-2.0-or-later; redistribution uses Apache-2.0 | `ESP-IDF-5.5-Apache-2.0.txt` |
| Xtensa HAL | MIT; Tensilica Inc. | `ESP-IDF-5.5-COPYRIGHT.rst` |
| Espressif Bluetooth, Wi-Fi coexistence and PHY precompiled objects | Apache-2.0, including binary/object distribution | `ESP-IDF-5.5-Apache-2.0.txt` |
| Espressif Diagnostics 1.2.1 | Apache-2.0 | `ESP-IDF-5.5-Apache-2.0.txt` |
| GCC `libgcc` and `libstdc++` runtime | GPLv3 with GCC Runtime Library Exception 3.1 | `GCC-GPL-3.0.txt`, `GCC-Runtime-Library-Exception-3.1.txt` |

The radio object libraries are provided by
[esp32-bt-lib](https://github.com/espressif/esp32-bt-lib),
[esp32-wifi-lib](https://github.com/espressif/esp32-wifi-lib) and
[esp-phy-lib](https://github.com/espressif/esp-phy-lib). Their upstream
repositories explicitly license the compiled objects under Apache-2.0.
The linked [Espressif Diagnostics](https://components.espressif.com/components/espressif/esp_diagnostics/versions/1.2.1)
component is also Apache-2.0. The GNU runtime exception permits the
corresponding runtime code to be combined with a non-GPL program under its
stated conditions. The LGPL Arduino core retains its separate obligations.

### Licenses embedded in M5Stack libraries

M5Cardputer 1.1.1 contains adapted Adafruit TCA8418 keypad driver sources.
Those files identify a **BSD** license and refer to a `license.txt` that was
not included in the installed M5Cardputer package. The original license text
is supplied as `LICENSES/M5Cardputer-Adafruit-TCA8418-BSD.txt`, obtained from
[Adafruit TCA8418](https://github.com/adafruit/Adafruit_TCA8418).

The [M5GFX license inventory](https://github.com/m5stack/M5GFX/blob/master/README.md#license)
also credits these bundled components:

| Component or asset | Upstream license / attribution |
| --- | --- |
| LovyanGFX | FreeBSD-style license, lovyan03 |
| TJpgDec | ChaN's own permissive terms (copyright notice retained for source redistribution) |
| Pngle | MIT, kikuchan |
| QRCode | MIT, Richard Moore and Nayuki |
| QOI image decoder | MIT, Dominic Szablewski (reference format implementation); the M5GFX adaptation has no explicit upstream header |
| miniz | Public domain / Unlicense, Rich Geldreich |
| result | MIT, Matthew Rodusek |
| GFX / GLCD fonts | BSD 2-clause, Adafruit Industries |
| Fonts 2, 4, 6, 7, 8 | FreeBSD-style license, Bodmer |
| Converted IPA font | IPA Font License |
| efont | BSD 3-clause, Electronic Font Open Laboratory |
| TomThumb font | BSD 3-clause, Brian J. Swetland and contributors |

This table inventories the M5GFX distribution; it does not claim that every
optional font or decoder is present in the linked firmware. Relevant original
copyright notices and conditions must accompany any redistributed binaries.
The applicable upstream texts from LovyanGFX, Adafruit GFX, Bodmer, IPA font,
efont, QOI, miniz and additional permissive components are also supplied in
`LICENSES/`.

## Display protocol references

The WeAct Display FS V1 protocol was researched using
[WeActStudio SystemMonitor](https://github.com/WeActStudio/WeActStudio.SystemMonitor)
and [turing-smart-screen-python](https://github.com/mathoudebine/turing-smart-screen-python).
Those Python projects are GPL-licensed. Their code is **not bundled** in this
repository; the C++ library implements the observed USB protocol. The GPL
license of a reference implementation does not, by itself, license the
independently written protocol implementation.

## Firmware redistribution

The Arduino-ESP32 LGPL obligations remain applicable even though the original
demo source is MIT-licensed. This repository includes the Arduino core
and FS source snapshot (`sources/arduino-esp32-3.3.2-corresponding-source.zip`)
extracted from the verified upstream 3.3.2 board-platform package,
the project source, partition table, build script and all license texts.
The firmware ZIP also includes these materials to let recipients modify
the LGPL-covered Arduino core and rebuild the executable.
The precompiled ESP32 libraries and M5Stack's bundled third-party code
retain their own notice and source/license obligations.

The source and rebuild procedure were exercised on a separate copy of the
Arduino-ESP32 3.3.2 platform: a modified core source file compiled, its object
was linked into the demo, and a new application image was generated.
The supplied firmware image was not replaced by this test build.

The firmware package includes the notices, corresponding Arduino source,
original demo and library source, and build instructions. Recipients who
redistribute a changed firmware must retain the relevant notices and comply
with the terms of each included component.
