# Quick start

## ⚠️ Before you flash

The supplied factory image replaces **all 8 MiB** of the Cardputer's flash: previous firmware, settings, and NVS. Confirm that your board is a Cardputer ADV and check the image's SHA-256 before proceeding.

Follow [FLASH.md](FLASH.md) for the exact commands. Nothing in this project flashes the device automatically.

## 🎛️ Connect and use the demo

After flashing and rebooting, disconnect the Cardputer from the PC and connect the WeAct panel to its USB-C port.

The Cardputer's built-in TFT opens on **Dashboard**. Press `TAB` to move between **Dashboard**, **Actions**, and **Info**.

| Key | On Dashboard / Info | On Actions |
| --- | --- | --- |
| `TAB` | Change page | Change page |
| `UP` / `DOWN` | Adjust brightness | Move through commands |
| `ENTER` | Pause or resume | Execute the selected command |
| Printable key | Pulse the animation (Dashboard only) | — |

**Actions** has five commands: pause/resume animation, rotate, brightness +10%, brightness −10%, and screen on/off. Select one with `UP`/`DOWN`, then press `ENTER`.

Brightness moves in 10-point steps from 20% to 100%; `DOWN` at 20% does nothing. `SCREEN > OFF` sets the backlight to 0% without powering down the panel. `SCREEN > ON` (or `UP` while off) restores the previous level.

`ROTATE` cycles **landscape → portrait → reverse landscape → reverse portrait**. Colored corner markers briefly show the direction. The fluid keeps its proportions in portrait; reversed orientations use the panel's hardware rotation.

## What drives the animation

Tilt the Cardputer to change the fluid's movement. Printable keys on Dashboard add a pulse.

Once per second, the demo also samples FPS, rendering time, and USB transfer time. It uses pseudo-random 80/10/10 weighting to vary shape, color, and speed, with smooth transitions. These are visual effects rather than measurements of CPU load. Until new samples arrive after connection, the animation uses its default appearance.

## If the display does not connect

Check that the panel is the **WeAct FS V1 0.96-inch** model, the USB-C cable carries data, and USB Host power is adequate. The **Info** page shows firmware version, connection state, and errors.

After a transport fault, disconnect and reconnect the panel. See [troubleshooting](TROUBLESHOOTING.md) for further checks.

