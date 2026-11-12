# Troubleshooting

## WeAct stays on WAITING

Check the exact panel model (WeAct FS V1 0.96-inch), the USB-C **data** cable, the Cardputer's USB Host power and whether the device is visible on the Info page. The driver recognizes VID:PID `1A86:FE0C` and requires a supported CDC interface and firmware reply.

## Version or connection error

This library currently accepts the `V1.0.0.0` firmware family. A different firmware revision is not assumed compatible. After a fault, disconnect and reconnect the panel; the driver deliberately does not continue sending arbitrary data to a faulted link.

## Unexpected orientation

In **Actions**, select `ROTATE` and press `ENTER`. The sequence is LAND → PORT → REV-LAND → REV-PORT. Colored corner markers are displayed briefly. The Cardputer's built-in TFT remains in its own fixed orientation.

## Flicker, partial frame or slow motion

The demo currently transfers the **entire logical framebuffer** every frame; no dirty-region optimization is claimed. Check USB transfer timing and FPS on the Dashboard/Info page. A device-wide flash or display update problem cannot be diagnosed from a successful compiler run alone. Do not enable FastLZ as an untested shortcut on the observed V1.0.0.0 panel.

## Microphone

The microphone is deliberately **not used** by this revision. The fluid responds to the BMI270 and Cardputer keyboard.

## Connection from PC after using USB Host

The Cardputer has one relevant USB-C port. A change from the external screen to PC flashing requires unplugging/reconnecting the cable; automatic mode switching is not promised here.

For bug reports, include Cardputer model, WeAct model and firmware version, toolchain versions, page status, exact steps, observed behavior, and (if possible) a short video.

