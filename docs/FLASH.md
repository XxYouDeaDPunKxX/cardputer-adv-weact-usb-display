# Flash the supplied Cardputer ADV factory firmware

**Warning:** This procedure overwrites **all 8 MiB of flash**, including the previous operating system, bootloader, partitions, application, NVS and stored settings. It is intended only for an **M5Stack Cardputer ADV with ESP32-S3 and 8 MiB flash**. Save anything you need from the current device before proceeding.

This image is **not** an app-only update, OTA package or M5Launcher package.

1. Read [supported hardware](HARDWARE.md), identify the actual ESP32-S3 board and serial port, and disconnect the external WeAct panel.
2. Obtain `firmware/CardputerADV_WeActDemo.factory-8MB.bin` or the identical file from a reviewed release package.
3. Verify its SHA-256 digest against `firmware/manifest.json` (Windows: `Get-FileHash -Algorithm SHA256`; Linux/macOS: `sha256sum` or `shasum -a 256`).
4. Install [Espressif esptool](https://docs.espressif.com/projects/esptool/en/latest/) and use the **port found on your machine**, not a copied COM number.
5. Flash the entire image at address `0x0`, verify it, and reset:

```powershell
esptool --chip esp32s3 --port COMx flash-id
esptool --chip esp32s3 --port COMx write-flash --flash-mode dio --flash-size 8MB --flash-freq 80m 0x0 firmware/CardputerADV_WeActDemo.factory-8MB.bin
esptool --chip esp32s3 --port COMx verify-flash 0x0 firmware/CardputerADV_WeActDemo.factory-8MB.bin
esptool --chip esp32s3 --port COMx run
```

`COMx` is a placeholder. Check the chip identity and detected 8 MiB capacity before any write. Some boards require entering the ROM download mode to establish a connection; consult the official board documentation if automatic reset does not work.

6. Disconnect from the PC, connect the WeAct panel, and follow the [quick start](QUICKSTART.md).

The factory image contains bootloader at `0x0`, partition table at `0x8000`, factory app at `0x10000` and an **erased NVS area** (`0x9000`–`0xCFFF`). The layout also has reserved OTA metadata and an `ota_0` slot, but the demo does **not** implement OTA updates.

The build and packaging tools in this repository perform **no flash operations**. Installing firmware is always an explicit user action.

