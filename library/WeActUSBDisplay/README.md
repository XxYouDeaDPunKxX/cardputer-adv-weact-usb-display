# WeActUSBDisplay

C++ driver for the WeAct Studio Display FS V1 0.96-inch USB-C panel (observed firmware `V1.0.0.0`, USB VID:PID `1A86:FE0C`). The protocol core works with any transport that implements `weact::ByteTransport`. The optional `EspUsbHostTransport` adapter uses the separate EspUsbHost library on an ESP32-S3 USB Host board.

## Get started

Install this `WeActUSBDisplay` folder as an Arduino library. For an ESP32-S3 project, install Arduino-ESP32 3.3.2 and EspUsbHost 2.10.1, then open `examples/ColorCycle/ColorCycle.ino`. The example connects to the panel over USB CDC and alternates the screen color. It does not require the Cardputer demo or M5Cardputer library.

If you use a different transport, implement `open`, `connected`, `write`, `flush`, `readExact`, and `close` from `weact::ByteTransport`. The `flush(expectedBytes)` method must confirm the bytes were actually transferred. Create a `weact::Driver` with your transport and call `connect()` after the USB CDC interface becomes available.

## Driver operations

| Method | Operation |
| --- | --- |
| `connect(Device)` | Open the panel, read firmware version, initialize landscape and 60% brightness |
| `setOrientation(Orientation)` | Portrait, reverse portrait, landscape, or reverse landscape |
| `width()` / `height()` | Current logical dimensions |
| `setBrightness(percent, fadeMs)` | Set brightness from 0 to 100%, optionally with a fade |
| `screenOff()` / `screenOn()` | Turn the backlight off and restore its previous level |
| `fillScreen(color)` | Fill the panel with one RGB565 color |
| `fillRect(x, y, w, h, color)` | Solid-color rectangle; protocol tested, physical partial-fill effect pending validation |
| `drawRect(x, y, w, h, pixels)` | Send row-major, little-endian RGB565 pixels to a logical rectangle |
| `firmwareVersion()`, `state()`, `lastError()` | Inspect connection state and errors |
| `onRemoved()` | Reset the driver after disconnecting the panel |

The driver does not allocate a full framebuffer or draw text. Your application supplies pixels and can update just the rectangles that changed.

`drawRectFastLz()` accepts caller-compressed FastLZ chunks, and `sendFreeCommand()` exposes the `Free` opcode. Their effects have not been validated on the target panel, so use the uncompressed drawing path for supported behavior.

The library archive contains this README, the source headers, metadata, and the `ColorCycle` example. The original library source is MIT-licensed; the optional EspUsbHost dependency is installed separately and retains its own MIT copyright notice. The package includes `LICENSE`, `THIRD_PARTY_NOTICES.md`, and the `LICENSES/` directory.
