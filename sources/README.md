# Arduino-ESP32 corresponding source

`arduino-esp32-3.3.2-corresponding-source.zip` contains an unchanged-byte
snapshot of the Arduino core and FS library sources used for this build.
The 136 files were extracted from the official Espressif Arduino-ESP32
3.3.2 board-platform archive. Unrelated board variants, bootloader images
and other libraries have been excluded.

| Property | Value |
| --- | --- |
| Upstream | [Espressif Arduino-ESP32](https://github.com/espressif/arduino-esp32/tree/3.3.2) |
| Board-platform version | 3.3.2 |
| Original upstream package | `esp32-3.3.2.zip` |
| Upstream package SHA-256 | `8195ad45a35e0ebabe0985503f97bcfa4fe923b4aec4944ec0f2f84e4eb4acab` |
| This source snapshot size | 509,236 bytes |
| This source snapshot SHA-256 | `8e2c41cfc16736d4a0844f038c7e32df9bde8c6691f2b2a81a7f38f06c9c4be8` |
| License | LGPL 2.1, with separately licensed files inside the package |

The upstream package hash matches the Espressif board-package index for
3.3.2. This source snapshot preserves the selected upstream file bytes and
relative paths:
`cores/esp32/`, `libraries/FS/`, `variants/m5stack_cardputer/`,
and seven root-level build metadata files. It is **not** a complete
installable board-platform package.
The [full LGPL license](../LICENSES/Arduino-ESP32-LGPL-2.1.txt) and
[third-party notices](../THIRD_PARTY_NOTICES.md) are distributed alongside it.

The project's own sketch, display library, partition layout and build script
are included in this repository and in the demo-source and firmware ZIPs.
Follow the [build instructions](../docs/BUILD.md) to install the complete
upstream platform with Arduino CLI, apply changes to the included core
sources, and rebuild. The toolchain and separately distributed ESP-IDF
binaries are fetched by Arduino CLI; they are not embedded in this snapshot.
