// Cardputer ADV + WeAct USB 0.96: three-page demonstrator.
// The menu exposes documented protocol operations and animation controls.
// Experimental WeAct protocol operations remain available in the library.
#include <Arduino.h>
#include <M5Cardputer.h>
#include <EspUsbHost.h>
#include <atomic>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <WeActDisplay.h>
#include <EspUsbHostTransport.h>
#include "RenderTypes.h"
#include "MotionDynamics.h"
#include "ReactiveLuminousRenderer.h"
#include "FramePerfCounter.h"
#include "TelemetryAnimation.h"
#include "DemoUiModel.h"
#include "DemoActions.h"

static EspUsbHost host;
static EspUsbHostCdcSerial cdc(host);
static weact::EspUsbHostTransport transport(host,cdc);
static weact::Driver panel(transport);
static weact::DemoUiModel ui;
static weact::MotionDynamics dynamics;
static weact::ReactiveLuminousFluid luminousFluid;
static weact::FramePerfCounter perfWindow;
static weact::FramePerfSample perfSample{};
static weact::TelemetryAnimation telemetryAnimation;
static weact::Motion currentMotion{};
static uint16_t frame[weact::WIDTH*weact::HEIGHT]{};
static std::atomic<uint32_t> removeEvents{0};
static uint32_t handledRemoval=0;
static bool cdcAttached=false,hostReady=false;
static uint8_t usbDevices=0;
static uint32_t lastScan=0,lastImu=0,lastUi=0,frameWindowAt=0,frameCount=0,fps=0;
static bool wasPressed[56]{},oldEnter=false,oldTab=false,oldUp=false,oldDown=false;
static char lastSignature[200]{};
static uint8_t shownPage=255;
static const char*feedback="";
static uint32_t feedbackUntil=0;

using Grid=weact::CompactScreen;
static constexpr uint16_t UI_BLACK=0x0841;
static constexpr uint16_t UI_RULE=0x39a7;
static constexpr uint16_t UI_WHITE=0xe73c;
static constexpr uint16_t UI_MUTED=0x8c51;
static constexpr uint16_t UI_CYAN=0x3e9e;
static constexpr uint16_t UI_AMBER=0xf5c8;
static constexpr uint16_t UI_RED=0xf288;
static constexpr uint16_t UI_GREEN=0x5dcb;

static void writeText(int x,int y,const char*label,uint16_t color=UI_WHITE){
 auto&d=M5Cardputer.Display;
 d.setTextSize(2);d.setTextColor(color,UI_BLACK);d.setCursor(x,y);d.print(label);
}
static void writeField(int x,int y,const char*value,int chars,uint16_t color=UI_WHITE){
 char buffer[24]{};
 if(chars>18)chars=18;
 snprintf(buffer,sizeof(buffer),"%-*.*s",chars,chars,value);
 writeText(x,y,buffer,color);
}
static const char*errorName(weact::Error error){
 switch(error){
 case weact::Error::Identity:return "IDENTITY";
 case weact::Error::Connect:return "CDC FAIL";
 case weact::Error::Disconnected:return "UNPLUGGED";
 case weact::Error::Version:return "VERSION";
 case weact::Error::Incomplete:return "TRANSFER";
 case weact::Error::Bounds:return "BOUNDS";
 case weact::Error::NotReady:return "NOT READY";
 case weact::Error::Payload:return "PAYLOAD";
 default:return "--";
 }
}
static const char*orientationName(weact::Orientation orientation){
 switch(orientation){
 case weact::Orientation::Portrait:return "PORT";
 case weact::Orientation::ReversePortrait:return "REV-P";
 case weact::Orientation::ReverseLandscape:return "REV-L";
 default:return "LAND";
 }
}
static const char*connectionName(){
 if(!hostReady)return "HOST NO";
 if(panel.state()==weact::State::Faulted)return "ERROR";
 return panel.ready()?"READY":"WAITING";
}
static void drawChrome(){
 auto&d=M5Cardputer.Display;
 d.fillScreen(UI_BLACK);d.drawFastHLine(4,24,232,UI_RULE);
 const auto page=ui.page();
 writeText(8,Grid::TopY,page==weact::UiPage::Dashboard?"WEACT":
           page==weact::UiPage::Actions?"ACTIONS":"INFO",UI_CYAN);
 writeText(202,Grid::TopY,page==weact::UiPage::Dashboard?"1/3":
           page==weact::UiPage::Actions?"2/3":"3/3",UI_MUTED);
 if(page==weact::UiPage::Actions)d.drawFastHLine(4,47,232,UI_RULE);
 if(page==weact::UiPage::Info)writeText(150,Grid::TopY,"ms",UI_MUTED);
}
static void drawDashboard(){
 const bool ready=panel.ready();
 const uint16_t stateColor=panel.state()==weact::State::Faulted||!hostReady?
    UI_RED:ready?UI_GREEN:UI_AMBER;
 writeField(92,Grid::TopY,connectionName(),8,stateColor);
 char value[12];
 writeText(8,Grid::Row1,"ANIM",UI_MUTED);
 writeField(68,Grid::Row1,!ready?"WAIT":ui.brightness()==0?"OFF":
            ui.paused()?"PAUSE":"RUN",5,ready?UI_GREEN:UI_AMBER);
 writeText(140,Grid::Row1,"LUM",UI_MUTED);
 if(ready)snprintf(value,sizeof(value),"%u%%",unsigned(ui.brightness()));
 else strcpy(value,"--");
 writeField(184,Grid::Row1,value,4,UI_AMBER);

 writeText(8,Grid::Row2,"FPS",UI_MUTED);
 snprintf(value,sizeof(value),"%lu",(unsigned long)(fps>999?999:fps));
 writeField(56,Grid::Row2,value,3,ready?UI_WHITE:UI_MUTED);
 writeText(114,Grid::Row2,"USB",UI_MUTED);
 if(perfSample.count)snprintf(value,sizeof(value),"%lu",
    (unsigned long)(perfSample.usbAvgMs>999?999:perfSample.usbAvgMs));
 else strcpy(value,"--");
 writeField(170,Grid::Row2,value,3);writeText(212,Grid::Row2,"ms",UI_MUTED);

 writeText(8,Grid::Row3,"IMU",UI_MUTED);
 writeField(56,Grid::Row3,currentMotion.valid?"OK":"--",2,
            currentMotion.valid?UI_GREEN:UI_AMBER);
 writeText(114,Grid::Row3,"REN",UI_MUTED);
 if(perfSample.count)snprintf(value,sizeof(value),"%lu",
    (unsigned long)(perfSample.renderAvgMs>999?999:perfSample.renderAvgMs));
 else strcpy(value,"--");
 writeField(170,Grid::Row3,value,3);writeText(212,Grid::Row3,"ms",UI_MUTED);

 writeText(8,Grid::Row4,"ORI",UI_MUTED);
 writeField(56,Grid::Row4,ready?orientationName(panel.orientation()):"--",5);
 writeText(124,Grid::Row4,"PX",UI_MUTED);
 if(ready)snprintf(value,sizeof(value),"%ux%u",unsigned(panel.width()),
                   unsigned(panel.height()));
 else strcpy(value,"--");
 writeField(156,Grid::Row4,value,6);
 writeText(8,Grid::Row5,"TAB MENU",UI_CYAN);
 writeText(120,Grid::Row5,ui.paused()?"ENT RUN":"ENT PAUSE",UI_WHITE);
}
static const char*commandLabel(weact::UiCommand command){
 switch(command){
 case weact::UiCommand::Animation:return ui.paused()?"ANIM > RUN":"ANIM > PAUSE";
 case weact::UiCommand::Orientation:
  switch(weact::nextOrientation(panel.orientation())){
  case weact::Orientation::Portrait:return "ROTATE > PORTRAIT";
  case weact::Orientation::ReverseLandscape:return "ROTATE > REV-LAND";
  case weact::Orientation::ReversePortrait:return "ROTATE > REV-PORT";
  default:return "ROTATE > LAND";
  }
 case weact::UiCommand::Brighter:return "LIGHT +10%";
 case weact::UiCommand::Dimmer:return "LIGHT -10%";
 default:return ui.brightness()==0?"SCREEN > ON":"SCREEN > OFF";
 }
}
static void drawActions(uint32_t now){
 auto&d=M5Cardputer.Display;
 d.fillRect(0,27,240,108,UI_BLACK);d.drawFastHLine(4,47,232,UI_RULE);
 const bool hasFeedback=(int32_t)(feedbackUntil-now)>0;
 writeField(8,Grid::StatusY,hasFeedback?feedback:"UP/DN SEL ENTER=OK",18,
            hasFeedback?UI_AMBER:UI_MUTED);
 static constexpr int y[4]={Grid::MenuY0,Grid::MenuY1,Grid::MenuY2,Grid::MenuY3};
 const uint8_t first=ui.firstVisibleCommand();
 for(uint8_t row=0;row<weact::DemoUiModel::VisibleCommands;row++){
  const uint8_t i=first+row;
  const bool selected=i==ui.selectedIndex();
  writeText(8,y[row],selected?">":" ",selected?UI_CYAN:UI_MUTED);
  writeField(28,y[row],commandLabel(static_cast<weact::UiCommand>(i)),17,
             selected?UI_WHITE:UI_MUTED);
 }
}
static void signedTenth(char out[8],float input){
 if(!isfinite(input)){strcpy(out,"--");return;}
 int value=int(input*10.f+(input>=0.f?.5f:-.5f));
 if(value>99)value=99;if(value< -99)value=-99;
 const unsigned magnitude=unsigned(value<0?-value:value);
 snprintf(out,8,"%c%u.%u",value<0?'-':'+',magnitude/10,magnitude%10);
}
static void drawInfo(){
 char value[24];
 writeText(8,Grid::Row1,"HOST",UI_MUTED);
 writeField(68,Grid::Row1,hostReady?"OK":"NO",2,hostReady?UI_GREEN:UI_RED);
 writeText(104,Grid::Row1,"CDC",UI_MUTED);
 writeField(152,Grid::Row1,cdcAttached?"OK":"NO",2,cdcAttached?UI_GREEN:UI_RED);
 writeText(190,Grid::Row1,"D",UI_MUTED);
 snprintf(value,sizeof(value),"%u",unsigned(usbDevices>99?99:usbDevices));
 writeField(207,Grid::Row1,value,2);
 writeText(8,Grid::Row2,"LCD",UI_MUTED);
 const bool ready=panel.ready();
 writeField(56,Grid::Row2,ready?"OK":panel.state()==weact::State::Faulted?
            "!!":"--",2,ready?UI_GREEN:UI_AMBER);
 writeText(104,Grid::Row2,"FW",UI_MUTED);
 writeField(134,Grid::Row2,ready?panel.firmwareVersion():"--",8);
 writeText(8,Grid::Row3,"FPS",UI_MUTED);
 snprintf(value,sizeof(value),"%lu",(unsigned long)(fps>999?999:fps));
 writeField(54,Grid::Row3,value,3);
 writeText(105,Grid::Row3,"R",UI_MUTED);
 if(perfSample.count)snprintf(value,sizeof(value),"%lu",
  (unsigned long)(perfSample.renderAvgMs>999?999:perfSample.renderAvgMs));
 else strcpy(value,"--");
 writeField(128,Grid::Row3,value,3);
 writeText(179,Grid::Row3,"U",UI_MUTED);
 if(perfSample.count)snprintf(value,sizeof(value),"%lu",
  (unsigned long)(perfSample.usbAvgMs>999?999:perfSample.usbAvgMs));
 else strcpy(value,"--");
 writeField(202,Grid::Row3,value,3);
 writeText(8,Grid::Row4,"ORI",UI_MUTED);
 writeField(56,Grid::Row4,ready?orientationName(panel.orientation()):"--",5);
 writeText(124,Grid::Row4,"PX",UI_MUTED);
 if(ready)snprintf(value,sizeof(value),"%ux%u",unsigned(panel.width()),
                   unsigned(panel.height()));
 else strcpy(value,"--");
 writeField(156,Grid::Row4,value,6);
 if(panel.state()==weact::State::Faulted){
  snprintf(value,sizeof(value),"E:%s REPLUG",errorName(panel.lastError()));
  writeField(8,Grid::Row5,value,18,UI_RED);
 }else if(currentMotion.valid){
  char x[8],y[8];
  signedTenth(x,currentMotion.ax);signedTenth(y,currentMotion.ay);
  int g=int(currentMotion.gz+(currentMotion.gz>=0.f?.5f:-.5f));
  if(g>99)g=99;if(g< -99)g=-99;
  snprintf(value,sizeof(value),"X%s Y%s GZ%+d",x,y,g);
  writeField(8,Grid::Row5,value,18);
 }else writeField(8,Grid::Row5,"IMU NO DATA",18,UI_AMBER);
}
static void renderUi(){
 const uint32_t now=millis();
 const bool showingFeedback=(int32_t)(feedbackUntil-now)>0;
 char signature[200];
 if(ui.page()==weact::UiPage::Dashboard){
  snprintf(signature,sizeof(signature),"D/%u/%u/%u/%u/%u/%u/%lu/%lu/%lu/%lu",
           unsigned(panel.state()),unsigned(hostReady),unsigned(currentMotion.valid),
           unsigned(ui.paused()),unsigned(ui.brightness()),unsigned(panel.orientation()),
           (unsigned long)fps,(unsigned long)perfSample.renderAvgMs,
           (unsigned long)perfSample.usbAvgMs,(unsigned long)perfSample.count);
 }else if(ui.page()==weact::UiPage::Actions){
  snprintf(signature,sizeof(signature),"A/%u/%u/%u/%u/%u/%u/%s",
           unsigned(panel.state()),unsigned(ui.selectedIndex()),unsigned(ui.paused()),
           unsigned(ui.brightness()),unsigned(panel.orientation()),unsigned(showingFeedback),
           showingFeedback?feedback:"");
 }else{
  snprintf(signature,sizeof(signature),"I/%u/%u/%u/%u/%u/%u/%lu/%lu/%lu/%lu/%u/%d/%d/%d/%s",
           unsigned(panel.state()),unsigned(hostReady),unsigned(cdcAttached),
           unsigned(usbDevices),unsigned(currentMotion.valid),
           unsigned(panel.orientation()),(unsigned long)fps,
           (unsigned long)perfSample.renderAvgMs,(unsigned long)perfSample.usbAvgMs,
           (unsigned long)perfSample.count,unsigned(panel.lastError()),
           int(currentMotion.ax*10),int(currentMotion.ay*10),
           int(currentMotion.gz),panel.firmwareVersion());
 }
 const uint8_t page=uint8_t(ui.page());
 if(page==shownPage&&strcmp(lastSignature,signature)==0)return;
 M5Cardputer.Display.startWrite();
 if(page!=shownPage){drawChrome();shownPage=page;}
 if(ui.page()==weact::UiPage::Dashboard)drawDashboard();
 else if(ui.page()==weact::UiPage::Actions)drawActions(now);
 else drawInfo();
 M5Cardputer.Display.endWrite();
 snprintf(lastSignature,sizeof(lastSignature),"%s",signature);
}
static const char*actionMessage(weact::ActionResult result){
 switch(result){
 case weact::ActionResult::Paused:return "PAUSED";
 case weact::ActionResult::Running:return "RUNNING";
 case weact::ActionResult::Waiting:return "WAIT USB/SCREEN";
 case weact::ActionResult::LightSent:return "LIGHT SENT";
 case weact::ActionResult::OrientationSent:return "ROTATION SENT";
 case weact::ActionResult::ScreenOnSent:return "SCREEN ON SENT";
 case weact::ActionResult::ScreenOffSent:return "SCREEN OFF SENT";
 case weact::ActionResult::Limit:return "LIGHT LIMIT";
 case weact::ActionResult::NoPanel:return
   panel.state()==weact::State::Faulted?"REPLUG WEACT":"WAIT WEACT USB";
 default:return "USB TRANSFER FAIL";
 }
}
static void inputTick(uint32_t now){
 const auto keys=M5Cardputer.Keyboard.keysState();
 const auto pressed=M5Cardputer.Keyboard.keyList();
 bool down[56]{};bool printable=false,up=false,downKey=false;
 for(const auto&key:pressed){
  if(key.x<14&&key.y<4){
   const unsigned idx=unsigned(key.y)*14u+unsigned(key.x);
   down[idx]=true;if(!wasPressed[idx])printable=true;
  }
  // Actual printed UP and DOWN keys, independent of SHIFT character mapping.
  if(key.x==11&&key.y==2)up=true;
  if(key.x==11&&key.y==3)downKey=true;
 }
 memcpy(wasPressed,down,sizeof(down));
 const bool enter=keys.enter,tab=keys.tab;
 const bool changedPage=tab&&!oldTab;
 if(changedPage){ui.pressTab();feedbackUntil=0;}
 else if(ui.page()==weact::UiPage::Actions){
  const bool moved=(up&&!oldUp)||(downKey&&!oldDown);
  if(up&&!oldUp){ui.selectPrevious();feedbackUntil=0;}
  if(downKey&&!oldDown){ui.selectNext();feedbackUntil=0;}
  if(enter&&!oldEnter&&!moved){
   const auto result=weact::applyAction(ui.selectedCommand(),ui,panel);
   if(result==weact::ActionResult::OrientationSent)ui.markOrientation(now);
   feedback=actionMessage(result);
   feedbackUntil=now+2200;
  }
 }else{
  if(enter&&!oldEnter)ui.pressEnter();
  if(up&&!oldUp)(void)weact::applyAction(weact::UiCommand::Brighter,ui,panel);
  if(downKey&&!oldDown)(void)weact::applyAction(weact::UiCommand::Dimmer,ui,panel);
 }
 if(ui.page()==weact::UiPage::Dashboard&&printable&&!keys.fn&&!enter&&
    !tab&&!up&&!downKey)ui.pulse(now);
 oldEnter=enter;oldTab=tab;oldUp=up;oldDown=downKey;
}
static void pollImu(uint32_t now){
 if(now-lastImu<40)return;
 lastImu=now;currentMotion.valid=false;
 if(!M5.Imu.isEnabled())return;
 M5.Imu.update();
 float ax=0,ay=0,az=0,gx=0,gy=0,gz=0;
 if(!M5.Imu.getAccel(&ax,&ay,&az)||!M5.Imu.getGyro(&gx,&gy,&gz)||
    !isfinite(ax)||!isfinite(ay)||!isfinite(az)||
    !isfinite(gx)||!isfinite(gy)||!isfinite(gz))return;
 currentMotion={true,ax,ay,gz};
}
static void scanUsb(){
 if(!hostReady)return;
 EspUsbHostDeviceInfo devices[ESP_USB_HOST_MAX_DEVICES]{};
 const size_t count=host.getDevices(devices,ESP_USB_HOST_MAX_DEVICES);
 usbDevices=uint8_t(count);
 if(panel.state()!=weact::State::Disconnected)return;
 size_t matches=0;EspUsbHostDeviceInfo selected{};
 for(size_t i=0;i<count;i++)if(devices[i].vid==0x1a86&&devices[i].pid==0xfe0c){
  ++matches;selected=devices[i];
 }
 if(matches!=1||!host.serialReady(selected.address,0))return;
 const weact::Device device{selected.vid,selected.pid,selected.address,true};
 if(panel.connect(device))ui.commitBrightness(panel.brightness());
}
void setup(){
 auto config=M5.config();config.serial_baudrate=0;
 M5Cardputer.begin(config,true);
 M5Cardputer.Display.setRotation(1);
 M5Cardputer.Display.setBrightness(190);
 M5Cardputer.Display.fillScreen(UI_BLACK);
 renderUi();
 host.onDeviceDisconnected([](const EspUsbHostDeviceInfo&info){
  if(info.vid==0x1a86&&info.pid==0xfe0c)
   removeEvents.fetch_add(1,std::memory_order_relaxed);
 });
 cdcAttached=cdc.begin(115200);
 if(cdcAttached)hostReady=host.begin();
 frameWindowAt=millis();renderUi();
}
void loop(){
 M5Cardputer.update();
 const uint32_t now=millis();
 const uint32_t removed=removeEvents.load(std::memory_order_relaxed);
 if(removed!=handledRemoval){
  handledRemoval=removed;panel.onRemoved();luminousFluid.reset();
  telemetryAnimation.reset();
  frameCount=0;fps=0;perfWindow.reset();perfSample={};frameWindowAt=now;
 }
 if(now-lastScan>=250){lastScan=now;scanUsb();}
 inputTick(now);pollImu(now);
 const weact::Position offset=dynamics.update(now,currentMotion);
 if(panel.ready()&&!ui.paused()&&ui.brightness()>0){
  const uint16_t w=panel.width(),h=panel.height();
  const uint32_t renderAt=micros();
  const auto visual=telemetryAnimation.value(now);
  (void)luminousFluid.render(frame,now,currentMotion,offset.x,offset.y,
                             ui.impulseAt(now),w,h,visual);
  if(ui.orientationMarkersAt(now))weact::markFrameCorners(frame,w,h);
  const uint32_t usbAt=micros();
  // One logical frame sized for the hardware orientation; no double rotation.
  const bool sent=panel.drawRect(0,0,w,h,frame);
  const uint32_t doneAt=micros();
  if(sent){++frameCount;perfWindow.add(usbAt-renderAt,doneAt-usbAt);}
 }
 const uint32_t sampleAt=millis();
 if(sampleAt-frameWindowAt>=1000){
  fps=uint32_t(uint64_t(frameCount)*1000u/(sampleAt-frameWindowAt));
  perfSample=perfWindow.take();frameCount=0;frameWindowAt=sampleAt;
  if(perfSample.count&&panel.ready()&&!ui.paused()&&ui.brightness()>0)
   telemetryAnimation.sample({true,fps,perfSample.renderAvgMs,
                              perfSample.usbAvgMs,usbDevices,true});
 }
 if(now-lastUi>=170){lastUi=now;renderUi();}
 delay(1);
}
