#pragma once
#include <stdint.h>

namespace weact {

enum class UiPage : uint8_t { Dashboard, Actions, Info };
enum class UiCommand : uint8_t { Animation, Orientation, Brighter, Dimmer, ScreenPower };

struct CompactScreen {
 static constexpr int Width=240, Height=135;
 static constexpr int GlyphWidth=12, GlyphHeight=16;
 static constexpr int TopY=3, StatusY=29;
 static constexpr int Row1=31, Row2=52, Row3=74, Row4=96, Row5=117;
 static constexpr int MenuY0=51, MenuY1=73, MenuY2=95, MenuY3=117;
 static constexpr bool fits(int x,int y,int chars){
  return x>=0&&y>=0&&x+chars*GlyphWidth<=Width&&y+GlyphHeight<=Height;
 }
};

class DemoUiModel {
 UiPage page_=UiPage::Dashboard;
 uint8_t selected_=0,brightness_=60;
 bool paused_=false;
 uint32_t pulseAt_=0;
 uint32_t orientationAt_=0;
 bool orientationMarked_=false;
 public:
 static constexpr uint8_t CommandCount=5;
 static constexpr uint8_t VisibleCommands=4;
 UiPage page()const{return page_;}
 UiCommand selectedCommand()const{return static_cast<UiCommand>(selected_);}
 uint8_t selectedIndex()const{return selected_;}
 uint8_t firstVisibleCommand()const{
  return selected_<VisibleCommands?0:uint8_t(CommandCount-VisibleCommands);
 }
 uint8_t brightness()const{return brightness_;}
 bool paused()const{return paused_;}
 void setPaused(bool value){paused_=value;}
 void pressTab(){
  page_=page_==UiPage::Dashboard?UiPage::Actions:
        page_==UiPage::Actions?UiPage::Info:UiPage::Dashboard;
 }
 void pressEnter(){if(page_!=UiPage::Actions)paused_=!paused_;}
 void selectNext(){selected_=uint8_t((selected_+1)%CommandCount);}
 void selectPrevious(){selected_=selected_?uint8_t(selected_-1):uint8_t(CommandCount-1);}
 bool nextBrightness(bool increase,uint8_t&next)const{
  if(increase){
   if(brightness_>=100)return false;
   next=brightness_<20?20:uint8_t(brightness_+10);
   return true;
  }
  if(brightness_<=20)return false;
  next=uint8_t(brightness_-10);
  return true;
 }
 void commitBrightness(uint8_t value){brightness_=value;}
 void pulse(uint32_t now){pulseAt_=now;}
 void markOrientation(uint32_t now){orientationAt_=now;orientationMarked_=true;}
 bool orientationMarkersAt(uint32_t now)const{
  return orientationMarked_&&uint32_t(now-orientationAt_)<2200u;
 }
 float impulseAt(uint32_t now)const{
  if(!pulseAt_||now<pulseAt_||now-pulseAt_>=900)return 0.f;
  return 1.f-float(now-pulseAt_)/900.f;
 }
};

} // namespace weact
