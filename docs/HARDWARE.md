# Supported hardware

## Verified target

- **Host board:** M5Stack Cardputer ADV, ESP32-S3, 8 MiB embedded flash, built-in 240×135 TFT, keyboard and BMI270 IMU.
- **External panel:** WeAct Studio **Display FS V1 0.96-inch** USB-C screen, reported VID:PID `1A86:FE0C`, USB CDC interface.
- **Observed panel firmware:** `V1.0.0.0`.
- **Pixels:** RGB565 little-endian; 160×80 in landscape, 80×160 in portrait.

The native USB-C port is used for USB Host operation when the WeAct display is connected. The same physical port is needed for USB flashing/debugging from a PC, so ordinary testing cannot maintain both connections simultaneously. Use a data-capable cable and adequate power.

This project is **not** validated for the original Cardputer, different ESP32-S3 boards, larger WeAct panels, generic USB serial displays, or all WeAct firmware revisions. USB VID:PID alone is not proof of protocol compatibility.

The display's four hardware orientation values are:

| Value | Orientation | Logical size |
| ---: | --- | --- |
| 0 | Portrait | 80×160 |
| 1 | Reverse portrait | 80×160 |
| 2 | Landscape | 160×80 |
| 3 | Reverse landscape | 160×80 |

The Cardputer's own TFT orientation remains independent of the external WeAct rotation.

