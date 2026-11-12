# WeAct FS V1 0.96-inch protocol notes

The protocol core is defined in `library/WeActUSBDisplay/src/WeActProtocol.h` and is separate from Arduino and the demonstration application. This document describes the USB commands observed on the FS V1 panel with firmware `V1.0.0.0`; no separate "revision B" compatibility class is defined or claimed.

| Operation | Wire prefix / opcode | Status |
| --- | --- | --- |
| Firmware query | `C2 0A`, then 19-byte reply | Exercised on the observed panel |
| Set orientation | `02`, orientation byte `0..3`, `0A` | Implemented and regression-tested; user-reported working demo |
| Brightness/fade | `03`, brightness byte, 16-bit little-endian fade milliseconds, `0A` | Exercised |
| Fill rectangle | `04` | Encoded by `fillScreen(color)` and `fillRect(x,y,w,h,color)`; partial fills still need panel validation |
| Raw bitmap | `05`, rectangle header, row-major RGB565 payload | Used by the demo |
| Compressed bitmap | `15`, rectangle header, FastLZ chunks | **Not validated** with the observed firmware |
| Free | `07 0A` | **Effect not validated** |

The rectangle header contains inclusive `x0,y0,x1,y1` coordinates as little-endian 16-bit values followed by `0A`. RGB565 pixels are transmitted little-endian. Dimensions and bounds are evaluated in the **selected hardware orientation**; the sender must not rotate the framebuffer a second time.

Opcode `04` addresses both full-screen and partial solid-color fills. The driver exposes `fillScreen(color)` and `fillRect(x, y, w, h, color)` for these cases; the latter is bounds-checked and covered by native protocol tests but has not yet been observed on the physical panel. For an arbitrary pixel rectangle, use `drawRect(x, y, w, h, pixels)`. The demo draws full frames by design.

The driver accepts the `V1.0.0.0` firmware version prefix observed on the hardware. The current unit's firmware does not establish FastLZ support; do not advertise the compressed path as verified.

Source references: [WeActStudio SystemMonitor](https://github.com/WeActStudio/WeActStudio.SystemMonitor) and its 0.96-inch communication implementation. This document describes the observed wire contract, not a complete specification for every WeAct product.

