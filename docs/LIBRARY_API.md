# Reusable library API

Install `library/WeActUSBDisplay` as an Arduino library or use its pure C++ protocol headers in your own project. The [ColorCycle example](../library/WeActUSBDisplay/examples/ColorCycle/ColorCycle.ino) demonstrates a complete ESP32-S3 USB Host connection without the Cardputer demo.

The core `weact::Driver` uses an injected `weact::ByteTransport`. It has no dependency on the Cardputer demo or Arduino core. `EspUsbHostTransport.h` supplies the optional ESP32-S3 adapter using the separate EspUsbHost library.

| API | Purpose |
| --- | --- |
| `connect(Device)` | Check `1A86:FE0C`, open CDC, query firmware, set landscape and initial 60% brightness. |
| `firmwareVersion()` | Return the reported version after handshake. |
| `setOrientation(Orientation)` | Set one of four hardware orientations; `width()` and `height()` follow it. |
| `setBrightness(percent, fadeMs)` | Set brightness 0–100% with an optional fade (default 1000 ms). |
| `screenOff()` / `screenOn()` | Set brightness to zero / restore prior nonzero brightness. |
| `fillScreen(rgb565)` | Send a full-panel color fill command. |
| `fillRect(x,y,w,h,rgb565)` | Fill a logical rectangle with one RGB565 color; bounds checked, physical partial-fill behavior pending validation. |
| `drawRect(x,y,w,h,pixels)` | Transfer row-major, little-endian RGB565 pixels for a logical rectangle. |
| `drawRectFastLz(...)` | Send caller-compressed FastLZ chunks; support has not been validated on the target panel. |
| `sendFreeCommand()` | Expose reference `Free` opcode; physical effect unknown. |
| `state()`, `lastError()`, `frames()` | Connection state, last error, completed draw count. |
| `onRemoved()` | Reset transport and state after display removal. |

`ByteTransport::write()` returns accepted bytes; `flush(expectedBytes)` must confirm exactly that many bytes were sent since the prior flush. Incomplete writes or flush failures put the driver in `Faulted`; unplug and call `onRemoved()` before reconnecting.

The driver does not maintain a framebuffer, render fonts or animations, or calculate dirty rectangles. Those belong to the caller.

Only the WeAct FS V1 0.96-inch screen with the observed `V1.0.0.0` firmware family is within the current compatibility claim. See [protocol](PROTOCOL.md) and [hardware](HARDWARE.md).

