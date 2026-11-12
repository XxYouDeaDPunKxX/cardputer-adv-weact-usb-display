#pragma once
#include <WeActDisplay.h>
#include "DemoUiModel.h"

namespace weact {

// A single dispatch path shared by the keyboard and the host-side tests.
// "Sent" means the USB transport completed the request, not that an
// unobservable panel-side effect has been independently confirmed.
enum class ActionResult : uint8_t {
 Paused, Running, Waiting, LightSent, OrientationSent, ScreenOnSent, ScreenOffSent,
 Limit, NoPanel, TransportError
};

inline Orientation nextOrientation(Orientation current){
 switch(current){
 case Orientation::Landscape:return Orientation::Portrait;
 case Orientation::Portrait:return Orientation::ReverseLandscape;
 case Orientation::ReverseLandscape:return Orientation::ReversePortrait;
 default:return Orientation::Landscape;
 }
}

inline ActionResult applyAction(UiCommand cmd,DemoUiModel&ui,Driver&panel){
 if(cmd==UiCommand::Animation){
  ui.setPaused(!ui.paused());
  if(ui.paused())return ActionResult::Paused;
  return panel.ready()&&ui.brightness()>0?ActionResult::Running:ActionResult::Waiting;
 }
 if(!panel.ready())return ActionResult::NoPanel;
 if(cmd==UiCommand::Orientation){
  if(!panel.setOrientation(nextOrientation(panel.orientation())))return ActionResult::TransportError;
  ui.setPaused(false);
  return ActionResult::OrientationSent;
 }
 if(cmd==UiCommand::ScreenPower){
  const bool turnOn=ui.brightness()==0;
  if(!(turnOn?panel.screenOn():panel.screenOff()))return ActionResult::TransportError;
  ui.commitBrightness(panel.brightness());
  if(turnOn)ui.setPaused(false);
  return turnOn?ActionResult::ScreenOnSent:ActionResult::ScreenOffSent;
 }
 if(cmd==UiCommand::Brighter&&ui.brightness()==0){
  if(!panel.screenOn())return ActionResult::TransportError;
  ui.commitBrightness(panel.brightness());ui.setPaused(false);
  return ActionResult::ScreenOnSent;
 }
 if(cmd!=UiCommand::Brighter&&cmd!=UiCommand::Dimmer)
  return ActionResult::TransportError;
 uint8_t target=0;
 if(!ui.nextBrightness(cmd==UiCommand::Brighter,target))return ActionResult::Limit;
 if(!panel.setBrightness(target))return ActionResult::TransportError;
 ui.commitBrightness(panel.brightness());
 return ActionResult::LightSent;
}

} // namespace weact
