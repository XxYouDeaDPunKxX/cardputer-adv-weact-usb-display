#pragma once
// Map existing performance telemetry to arbitrary visual controls.
// A stable input yields a repeatable result for each sample; smooth changes
// prevent a new assignment from appearing as a frame glitch.
#include <stdint.h>

namespace weact {
struct TelemetrySignals {
 bool valid=false;
 uint32_t fps=0,renderMs=0,usbMs=0;
 uint8_t usbDevices=0;
 bool panelReady=false;
 TelemetrySignals()=default;
 TelemetrySignals(bool v,uint32_t f,uint32_t r,uint32_t u,uint8_t devices,bool ready):
  valid(v),fps(f),renderMs(r),usbMs(u),usbDevices(devices),panelReady(ready){}
};

struct TelemetryVisuals {
 float shape=0.f,color=0.f,tempo=0.f;
};

class TelemetryAnimation {
 TelemetryVisuals current_{},target_{};
 uint32_t previousMs_=0,sequence_=0;

 static uint32_t scramble(uint32_t x){
  x^=x>>16;x*=0x7feb352du;
  x^=x>>15;x*=0x846ca68bu;
  return x^(x>>16);
 }
 static float unit(uint32_t x){return float(scramble(x)&1023u)/1023.f;}
 static float blend(float a,float b,float fraction){return a+(b-a)*fraction;}
 public:
 void reset(){current_={};target_={};previousMs_=0;sequence_=0;}

 void sample(const TelemetrySignals& data){
  if(!data.valid||!data.panelReady){target_={};return;}
  // Even a one-unit change can drive a different effect; this intentionally
  // has no physical interpretation of FPS or transfer time.
  const float signal[3]={unit(data.fps^0xa52913f1u),
                         unit(data.renderMs^0x68e31da4u),
                         unit(data.usbMs^0x1b56c4e9u)};
  const uint32_t assignment=scramble(sequence_++ ^
      (uint32_t(data.usbDevices)*0x9e3779b9u));
  const unsigned a=assignment%3u,b=(a+1u)%3u,c=(a+2u)%3u;
  target_.shape=.8f*signal[a]+.1f*signal[b]+.1f*signal[c];
  target_.color=.8f*signal[b]+.1f*signal[c]+.1f*signal[a];
  target_.tempo=.8f*signal[c]+.1f*signal[a]+.1f*signal[b];
 }

 TelemetryVisuals value(uint32_t now){
  uint32_t elapsed=0;
  if(previousMs_!=0)elapsed=now-previousMs_;
  previousMs_=now;
  // Long USB stalls must not turn a parameter transition into one flash.
  if(elapsed>200u)elapsed=200u;
  const float amount=float(elapsed)/750.f;
  current_.shape=blend(current_.shape,target_.shape,amount);
  current_.color=blend(current_.color,target_.color,amount);
  current_.tempo=blend(current_.tempo,target_.tempo,amount);
  return current_;
 }
};
} // namespace weact
