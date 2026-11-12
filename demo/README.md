# Cardputer ADV + WeAct USB Display demo source

Build or modify the Arduino demo that drives a WeAct Display FS V1 0.96-inch screen from the M5Stack Cardputer ADV.

## 🎛️ What the demo does

The Cardputer's built-in TFT has **Dashboard**, **Actions**, and **Info** pages. Its keyboard controls the WeAct screen: animation pause/resume, rotation through four orientations, brightness, and screen power.

The fluid animation responds to the **BMI270 motion sensor**. `TelemetryAnimation.h` uses measured FPS, rendering time, and USB transfer time as creative inputs, with pseudo-random 80/10/10 weighting and smooth transitions. It does not measure CPU percentage. Portrait modes preserve the fluid's proportions; reverse modes use the panel's hardware orientation. The microphone is unused.

For keys and on-screen controls, see `docs/QUICKSTART.md`.

## 🛠️ Build and test

Open `demo/CardputerADV_WeActDemo/CardputerADV_WeActDemo.ino` in Arduino IDE, or compile with `tools/build-demo.ps1` and Arduino CLI. Required core and library versions are in `docs/BUILD.md`. Check `docs/HARDWARE.md` before connecting the panel.

Run the native C++ tests with `python tools/run_native_tests.py`. A newly compiled image needs its own hardware testing before distribution.

## 📦 What's included

This source package contains the sketch, reusable WeActUSBDisplay library, native tests, build tools, the project's MIT `LICENSE`, and `THIRD_PARTY_NOTICES.md`. The `LICENSES/` directory contains upstream license texts; `sources/` contains the Arduino-ESP32 3.3.2 core/FS source snapshot.

The ready-to-flash 8 MiB image is distributed separately in the **firmware package**.
