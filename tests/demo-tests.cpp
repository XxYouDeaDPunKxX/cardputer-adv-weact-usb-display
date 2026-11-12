#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <WeActDisplay.h>
#include "../demo/CardputerADV_WeActDemo/DemoUiModel.h"
#include "../demo/CardputerADV_WeActDemo/DemoActions.h"
#include "../demo/CardputerADV_WeActDemo/ReactiveLuminousRenderer.h"
#include "../demo/CardputerADV_WeActDemo/TelemetryAnimation.h"

struct FakeLink : weact::ByteTransport {
 bool online=false, failNextFlush=false;
 std::vector<std::vector<uint8_t>> writes;
 std::vector<size_t> flushes;
 bool open(uint8_t)override{online=true;return true;}
 bool connected()const override{return online;}
 size_t write(const uint8_t*data,size_t n)override{
  if(!online)return 0;
  writes.emplace_back(data,data+n);return n;
 }
 bool flush(size_t n)override{
  flushes.push_back(n);
  if(failNextFlush){failNextFlush=false;return false;}
  return true;
 }
 bool readExact(uint8_t*into,size_t n,uint32_t)override{
  if(n!=19)return false;
  memset(into,0,n);into[0]=0xc2;
  memcpy(into+1,"V1.0.0.0",8);into[18]=0x0a;return true;
 }
 void close()override{online=false;}
};

int main(){
 using namespace weact;
 static_assert(CompactScreen::fits(28,CompactScreen::MenuY3,17),"menu fits 240x135");
 static_assert(CompactScreen::fits(202,CompactScreen::TopY,3),"page number fits");
 static_assert(!CompactScreen::fits(28,CompactScreen::MenuY3,18),"reject clipped labels");
 static_assert(DemoUiModel::CommandCount==5,"rotation is a first-class menu action");
 static_assert(DemoUiModel::VisibleCommands==4,"four readable action rows");

 DemoUiModel ui;
 assert(ui.page()==UiPage::Dashboard&&ui.selectedIndex()==0);
 ui.pressEnter();assert(ui.paused());
 ui.pressTab();assert(ui.page()==UiPage::Actions);
 ui.pressEnter();assert(ui.paused()); // ENTER is dispatched only to selected item.
 ui.selectPrevious();assert(ui.selectedCommand()==UiCommand::ScreenPower);
 ui.selectNext();assert(ui.selectedCommand()==UiCommand::Animation);
 ui.selectNext();assert(ui.selectedCommand()==UiCommand::Orientation);
 ui.selectPrevious();assert(ui.selectedCommand()==UiCommand::Animation);
 ui.selectPrevious();assert(ui.firstVisibleCommand()==1);
 ui.selectNext();assert(ui.firstVisibleCommand()==0);
 ui.markOrientation(1000);
 assert(ui.orientationMarkersAt(1000));
 assert(ui.orientationMarkersAt(3199));
 assert(!ui.orientationMarkersAt(3200));
 ui.pressTab();assert(ui.page()==UiPage::Info);
 ui.pressTab();assert(ui.page()==UiPage::Dashboard);

 FakeLink link;Driver panel(link);
 assert(applyAction(UiCommand::Brighter,ui,panel)==ActionResult::NoPanel);
 assert(ui.brightness()==60);
 assert(panel.connect({0x1a86,0xfe0c,1,true}));
 assert(link.writes[1]==std::vector<uint8_t>({0x02,0x02,0x0a}));

 // Rotate by hardware command only. The renderer receives logical dimensions
 // from the driver; it must NOT rotate the completed pixel buffer again.
 const Orientation order[]={
  Orientation::Portrait, Orientation::ReverseLandscape,
  Orientation::ReversePortrait, Orientation::Landscape
 };
 static uint16_t oriented[WIDTH*HEIGHT];
 ReactiveLuminousFluid orientedFluid;
 Motion still{};
 for(unsigned n=0;n<4;n++){
  ui.setPaused(true);
  const size_t commandAt=link.writes.size();
  assert(nextOrientation(panel.orientation())==order[n]);
  assert(applyAction(UiCommand::Orientation,ui,panel)==ActionResult::OrientationSent);
  assert(!ui.paused()&&panel.orientation()==order[n]);
  assert(link.writes.size()==commandAt+1);
  assert(link.writes[commandAt]==std::vector<uint8_t>({0x02,uint8_t(order[n]),0x0a}));
  const uint16_t w=panel.width(),h=panel.height();
  assert(size_t(w)*h==WIDTH*HEIGHT);
  assert(w==orientationWidth(order[n])&&h==orientationHeight(order[n]));
  assert(orientedFluid.render(oriented,1000+n*100,still,0,0,0,w,h));
  markFrameCorners(oriented,w,h);
  assert(oriented[0]==rgb(255,40,40));
  assert(oriented[w-1]==rgb(40,255,40));
  assert(oriented[size_t(h-1)*w]==rgb(40,120,255));
  assert(oriented[size_t(h)*w-1]==rgb(255,220,40));
  const size_t headerAt=link.writes.size();
  assert(panel.drawRect(0,0,w,h,oriented));
  const std::vector<uint8_t> head={0x05,0,0,0,0,
    uint8_t(w-1),0,uint8_t(h-1),0,0x0a};
  assert(link.writes[headerAt]==head);
  assert(link.flushes.back()==size_t(WIDTH)*HEIGHT*2);
 }
 {
  FakeLink brokenLink;Driver broken(brokenLink);DemoUiModel view;
  assert(applyAction(UiCommand::Orientation,view,broken)==ActionResult::NoPanel);
  assert(broken.connect({0x1a86,0xfe0c,1,true}));
  view.setPaused(true);
  brokenLink.failNextFlush=true;
  assert(applyAction(UiCommand::Orientation,view,broken)==ActionResult::TransportError);
  assert(broken.state()==State::Faulted);
  assert(broken.orientation()==Orientation::Landscape);
  assert(view.paused());
 }

 ui.setPaused(true); // Rotation resumed the animation; reset this test's precondition.
 const size_t start=link.writes.size();
 assert(applyAction(UiCommand::Animation,ui,panel)==ActionResult::Running);
 assert(!ui.paused()&&link.writes.size()==start);
 assert(applyAction(UiCommand::Animation,ui,panel)==ActionResult::Paused);
 assert(ui.paused()&&link.writes.size()==start);

 assert(applyAction(UiCommand::Brighter,ui,panel)==ActionResult::LightSent);
 uint8_t encoded[5];brightnessPacket(70,encoded);
 assert(link.writes.back()==std::vector<uint8_t>(encoded,encoded+5));
 assert(ui.brightness()==70&&panel.brightness()==70);
 assert(applyAction(UiCommand::Dimmer,ui,panel)==ActionResult::LightSent);
 assert(ui.brightness()==60&&panel.brightness()==60);
 assert(applyAction(UiCommand::ScreenPower,ui,panel)==ActionResult::ScreenOffSent);
 brightnessPacket(0,encoded);
 assert(link.writes.back()==std::vector<uint8_t>(encoded,encoded+5));
 assert(ui.brightness()==0);
 assert(applyAction(UiCommand::ScreenPower,ui,panel)==ActionResult::ScreenOnSent);
 assert(ui.brightness()==60&&!ui.paused());
 assert(applyAction(UiCommand::ScreenPower,ui,panel)==ActionResult::ScreenOffSent);
 assert(applyAction(UiCommand::Brighter,ui,panel)==ActionResult::ScreenOnSent);

 for(unsigned i=0;i<4;i++)ui.pressTab(); // dashboard -> actions
 assert(ui.page()==UiPage::Actions);
 uint8_t level=0;
 for(unsigned i=0;i<4;i++){
  assert(ui.nextBrightness(true,level));ui.commitBrightness(level);
 }
 assert(ui.brightness()==100);
 const size_t beforeLimit=link.writes.size();
 assert(applyAction(UiCommand::Brighter,ui,panel)==ActionResult::Limit);
 assert(link.writes.size()==beforeLimit);
 ui.commitBrightness(panel.brightness());

 link.failNextFlush=true;
 assert(applyAction(UiCommand::Brighter,ui,panel)==ActionResult::TransportError);
 assert(panel.state()==State::Faulted);
 assert(ui.brightness()==panel.brightness()); // commit only after USB success
 assert(applyAction(UiCommand::ScreenPower,ui,panel)==ActionResult::NoPanel);
 panel.onRemoved();assert(!panel.ready());
 assert(panel.connect({0x1a86,0xfe0c,1,true})); // recovery requires unplug/replug

 static uint16_t a[WIDTH*HEIGHT],b[WIDTH*HEIGHT];
 ReactiveLuminousFluid fluid;Motion motion{};
 assert(fluid.render(a,1000,motion,0,0,0));
 assert(fluid.render(b,1400,motion,0,0,0));
 unsigned changed=0;
 for(unsigned i=0;i<WIDTH*HEIGHT;i++)if(a[i]!=b[i])++changed;
 assert(changed>100); // the shipped animation is genuinely temporal.
 assert(panel.drawRect(0,0,WIDTH,HEIGHT,b));
 assert(link.flushes.back()==size_t(WIDTH)*HEIGHT*2);

 // Performance readings create bounded, smoothly changing visual parameters.
 // The link being unavailable must not invent telemetry-driven animation.
 TelemetryAnimation reactive;
 const auto idle=reactive.value(1000);
 assert(idle.shape==0.f&&idle.color==0.f&&idle.tempo==0.f);
 reactive.sample({true,24,8,31,1,true});
 reactive.value(1000);
 const auto first=reactive.value(1200);
 const auto settled=reactive.value(2000);
 assert(first.shape>=0.f&&first.shape<=1.f);
 assert(first.color>=0.f&&first.color<=1.f);
 assert(first.tempo>=0.f&&first.tempo<=1.f);
 assert(settled.shape>=0.f&&settled.shape<=1.f);
 assert(settled.color>=0.f&&settled.color<=1.f);
 assert(settled.tempo>=0.f&&settled.tempo<=1.f);
 assert(settled.shape!=0.f||settled.color!=0.f||settled.tempo!=0.f);
 reactive.sample({true,12,4,54,2,true});
 const auto reshuffled=reactive.value(3000);
 assert(reshuffled.shape!=settled.shape||reshuffled.color!=settled.color||
        reshuffled.tempo!=settled.tempo);
 reactive.sample({false,0,0,0,0,false});
 const auto fading=reactive.value(4000);
 assert(fading.shape<=reshuffled.shape&&fading.color<=reshuffled.color&&
        fading.tempo<=reshuffled.tempo);
 weact::TelemetryVisuals off{};
 for(uint32_t t=4200;t<=8000;t+=200)off=reactive.value(t);
 assert(off.shape<.01f&&off.color<.01f&&off.tempo<.01f);

 // At identical time and IMU input, telemetry changes the rendered pixels.
 static uint16_t plain[WIDTH*HEIGHT],modulated[WIDTH*HEIGHT];
 ReactiveLuminousFluid freshPlain,freshModulated;
 assert(freshPlain.render(plain,2200,motion,0,0,0));
 assert(freshModulated.render(modulated,2200,motion,0,0,0,
                              WIDTH,HEIGHT,settled));
 unsigned visualDifference=0;
 for(unsigned i=0;i<WIDTH*HEIGHT;i++)
  if(plain[i]!=modulated[i])++visualDifference;
 assert(visualDifference>100);

 // A small change to any measured performance channel changes the output.
 const auto visualFrom=[](uint32_t fps,uint32_t renderMs,uint32_t usbMs){
  TelemetryAnimation engine;
  engine.sample({true,fps,renderMs,usbMs,1,true});
  engine.value(1000);
  return engine.value(2000);
 };
 const auto reference=visualFrom(24,8,31);
 const auto fpsShift=visualFrom(25,8,31);
 const auto renderShift=visualFrom(24,9,31);
 const auto usbShift=visualFrom(24,8,32);
 assert(reference.shape!=fpsShift.shape||reference.color!=fpsShift.color||
        reference.tempo!=fpsShift.tempo);
 assert(reference.shape!=renderShift.shape||reference.color!=renderShift.color||
        reference.tempo!=renderShift.tempo);
 assert(reference.shape!=usbShift.shape||reference.color!=usbShift.color||
        reference.tempo!=usbShift.tempo);
 static uint16_t portrait[WIDTH*HEIGHT];
 ReactiveLuminousFluid portraitFluid;
 assert(portraitFluid.render(portrait,2200,motion,0,0,0,
                            HEIGHT,WIDTH,settled));

 // Portrait must rearrange the same fluid in the long axis, not squeeze its
 // horizontal extent by 1/2 and stretch its vertical extent by 2. Compare
 // green-channel pixels against a quarter-turn of the landscape rendering;
 // the small allowance accounts for raster interpolation on different axes.
 static uint16_t wideFluid[WIDTH*HEIGHT],tallFluid[WIDTH*HEIGHT];
 ReactiveLuminousFluid wideScene,tallScene;
 assert(wideScene.render(wideFluid,2200,motion,0,0,0,
                         WIDTH,HEIGHT,settled));
 assert(tallScene.render(tallFluid,2200,motion,0,0,0,
                         HEIGHT,WIDTH,settled));
 unsigned long difference=0;
 for(unsigned y=0;y<WIDTH;y++)for(unsigned x=0;x<HEIGHT;x++){
  const unsigned tallGreen=(tallFluid[y*HEIGHT+x]>>5)&63u;
  const unsigned rotatedGreen=(wideFluid[(HEIGHT-1u-x)*WIDTH+y]>>5)&63u;
  difference+=tallGreen>rotatedGreen?tallGreen-rotatedGreen:
              rotatedGreen-tallGreen;
 }
 std::printf("Portrait rotation mean green difference: %.3f\n",
             double(difference)/double(WIDTH*HEIGHT));
 assert(difference<2u*WIDTH*HEIGHT);

 std::puts("PASS demo: actions, rotation, recovery, telemetry-driven visuals");
}
