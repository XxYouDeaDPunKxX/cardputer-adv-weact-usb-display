// Minimal ESP32-S3 USB Host example for the WeAct FS V1 0.96-inch panel.
// Install EspUsbHost 2.10.1 alongside WeActUSBDisplay.
#include <Arduino.h>
#include <EspUsbHost.h>
#include <WeActDisplay.h>
#include <EspUsbHostTransport.h>
#include <atomic>

static EspUsbHost host;
static EspUsbHostCdcSerial cdc(host);
static weact::EspUsbHostTransport transport(host, cdc);
static weact::Driver panel(transport);
static std::atomic<bool> removed{false};
static bool hostReady = false;
static uint32_t lastScan = 0;
static uint32_t lastColor = 0;
static bool blue = false;

static void scanForPanel() {
  if (!hostReady || panel.state() != weact::State::Disconnected) return;
  EspUsbHostDeviceInfo devices[ESP_USB_HOST_MAX_DEVICES]{};
  const size_t count = host.getDevices(devices, ESP_USB_HOST_MAX_DEVICES);
  for (size_t i = 0; i < count; ++i) {
    const auto &device = devices[i];
    if (device.vid == 0x1a86 && device.pid == 0xfe0c &&
        host.serialReady(device.address, 0)) {
      panel.connect({device.vid, device.pid, device.address, true});
      return;
    }
  }
}

void setup() {
  host.onDeviceDisconnected([](const EspUsbHostDeviceInfo &device) {
    if (device.vid == 0x1a86 && device.pid == 0xfe0c)
      removed.store(true, std::memory_order_relaxed);
  });
  if (cdc.begin(115200)) hostReady = host.begin();
}

void loop() {
  const uint32_t now = millis();
  if (removed.exchange(false, std::memory_order_relaxed)) {
    panel.onRemoved();
    lastColor = 0;
  }
  if (now - lastScan >= 250) {
    lastScan = now;
    scanForPanel();
  }
  if (panel.ready() && (lastColor == 0 || now - lastColor >= 1000)) {
    lastColor = now;
    blue = !blue;
    panel.fillScreen(blue ? 0x001f : 0x07e0);  // RGB565 blue or green
  }
  delay(10);
}
