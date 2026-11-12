#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include "WeActDisplay.h"

static unsigned checks = 0;
#define CHECK(expr) do { ++checks; if (!(expr)) { \
 std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); return 1; \
} } while (0)

struct FakeTransport : weact::ByteTransport {
 bool live = false;
 std::vector<std::vector<uint8_t>> writes;
 std::vector<size_t> flushes;
 bool open(uint8_t address) override { live = address != 0; return live; }
 bool connected() const override { return live; }
 size_t write(const uint8_t* bytes, size_t n) override {
  writes.emplace_back(bytes, bytes + n);
  return n;
 }
 bool flush(size_t expected) override { flushes.push_back(expected); return true; }
 bool readExact(uint8_t* data, size_t n, uint32_t) override {
  if (n != 19) return false;
  std::memset(data, 0, n);
  data[0] = 0xc2;
  std::memcpy(data + 1, "V1.0.0.0_d7b870c8", 17);
  data[18] = 0x0a;
  return true;
 }
 void close() override { live = false; }
};

int main() {
 using namespace weact;
 CHECK(orientationWidth(Orientation::Portrait) == 80);
 CHECK(orientationHeight(Orientation::Portrait) == 160);
 CHECK(orientationWidth(Orientation::ReversePortrait) == 80);
 CHECK(orientationHeight(Orientation::ReversePortrait) == 160);
 CHECK(orientationWidth(Orientation::Landscape) == 160);
 CHECK(orientationHeight(Orientation::Landscape) == 80);
 CHECK(orientationWidth(Orientation::ReverseLandscape) == 160);
 CHECK(orientationHeight(Orientation::ReverseLandscape) == 80);

 FakeTransport io;
 Driver display(io);
 CHECK(display.connect({0x1a86, 0xfe0c, 7, true}));
 CHECK(std::strcmp(display.firmwareVersion(), "V1.0.0.0_d7b870c8") == 0);
 CHECK(display.orientation() == Orientation::Landscape);
 CHECK(display.width() == 160 && display.height() == 80);
 CHECK(display.setOrientation(Orientation::Portrait));
 CHECK(io.writes.back() == std::vector<uint8_t>({0x02, 0x00, 0x0a}));
 CHECK(display.width() == 80 && display.height() == 160);
 const uint16_t pixel = 0xf81f;
 CHECK(display.drawRect(79, 159, 1, 1, &pixel));
 CHECK(io.writes[4] == std::vector<uint8_t>({0x05,79,0,159,0,79,0,159,0,0x0a}));
 CHECK(io.writes[5] == std::vector<uint8_t>({0x1f,0xf8}));
 const auto beforeBounds = io.writes.size();
 CHECK(!display.drawRect(80, 0, 1, 1, &pixel));
 CHECK(display.lastError() == Error::Bounds && display.ready());
 CHECK(io.writes.size() == beforeBounds);

 CHECK(display.setOrientation(Orientation::ReversePortrait));
 CHECK(io.writes.back() == std::vector<uint8_t>({0x02,0x01,0x0a}));
 CHECK(display.setOrientation(Orientation::ReverseLandscape));
 CHECK(io.writes.back() == std::vector<uint8_t>({0x02,0x03,0x0a}));
 CHECK(display.setOrientation(Orientation::Landscape));
 CHECK(io.writes.back() == std::vector<uint8_t>({0x02,0x02,0x0a}));
 const size_t beforeInvalidOrientation = io.writes.size();
 CHECK(!display.setOrientation(static_cast<Orientation>(4)));
 CHECK(display.lastError() == Error::Bounds && display.ready());
 CHECK(io.writes.size() == beforeInvalidOrientation);

 CHECK(display.setBrightness(25, 500));
 CHECK(io.writes.back() == std::vector<uint8_t>({0x03,0x3f,0xf4,0x01,0x0a}));
 CHECK(display.brightness() == 25);
 CHECK(display.screenOff());
 CHECK(io.writes.back() == std::vector<uint8_t>({0x03,0x00,0xe8,0x03,0x0a}));
 CHECK(display.brightness() == 0);
 CHECK(display.screenOn());
 CHECK(display.brightness() == 25);
 CHECK(io.writes.back() == std::vector<uint8_t>({0x03,0x3f,0xe8,0x03,0x0a}));

 CHECK(display.fillScreen(0xf81f));
 CHECK(io.writes.back() == std::vector<uint8_t>({0x04,0,0,0,0,159,0,79,0,0x1f,0xf8,0x0a}));
 CHECK(display.fillRect(5,6,10,20,0x1234));
 CHECK(io.writes.back() == std::vector<uint8_t>({0x04,5,0,6,0,14,0,25,0,0x34,0x12,0x0a}));
 const size_t beforeInvalidFill = io.writes.size();
 CHECK(!display.fillRect(159,79,2,1,0x1234));
 CHECK(display.lastError() == Error::Bounds && io.writes.size() == beforeInvalidFill);
 CHECK(display.setOrientation(Orientation::Portrait));
 CHECK(display.fillScreen(0x07e0));
 CHECK(io.writes.back() == std::vector<uint8_t>({0x04,0,0,0,0,79,0,159,0,0xe0,0x07,0x0a}));
 CHECK(display.fillRect(70,150,10,10,0xf800));
 CHECK(io.writes.back() == std::vector<uint8_t>({0x04,70,0,150,0,79,0,159,0,0,0xf8,0x0a}));

 const uint8_t encodedA[] = {0xaa, 0xbb, 0xcc};
 const uint8_t encodedB[] = {0xdd, 0xee};
 const FastLzChunk chunks[] = {{encodedA, 3, 4}, {encodedB, 2, 4}};
 const size_t beforeFastLz = io.writes.size();
 CHECK(display.drawRectFastLz(1, 2, 2, 2, chunks, 2));
 CHECK(io.writes[beforeFastLz] == std::vector<uint8_t>({0x15,1,0,2,0,2,0,3,0,0x0a}));
 CHECK(io.writes[beforeFastLz+1] == std::vector<uint8_t>({4,0,3,0}));
 CHECK(io.writes[beforeFastLz+2] == std::vector<uint8_t>({0xaa,0xbb,0xcc}));
 CHECK(io.writes[beforeFastLz+3] == std::vector<uint8_t>({4,0,2,0}));
 CHECK(io.writes[beforeFastLz+4] == std::vector<uint8_t>({0xdd,0xee}));
 CHECK(io.flushes.back() == 4 + 3 + 4 + 2);
 const size_t beforeInvalidChunks = io.writes.size();
 const FastLzChunk invalid[] = {{encodedA, 3, 2}};
 CHECK(!display.drawRectFastLz(1, 2, 2, 2, invalid, 1));
 CHECK(display.lastError() == Error::Payload && display.ready());
 CHECK(io.writes.size() == beforeInvalidChunks);
 CHECK(display.sendFreeCommand());
 CHECK(io.writes.back() == std::vector<uint8_t>({0x07, 0x0a}));
 display.onRemoved();
 CHECK(std::strcmp(display.firmwareVersion(), "") == 0);
 std::printf("PASS WeAct protocol API: %u checks\n", checks);
 return 0;
}
