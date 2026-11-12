#pragma once
// Stateful three-core luminous fluid for landscape or portrait RGB565 frames.
// All interaction is local: real IMU and keyboard impulse; no microphone.
// Independent of M5/USB: one small interface for native verification.
#include <stdint.h>
#include <math.h>
#include "RenderTypes.h"
#include "TelemetryAnimation.h"

namespace weact {
class ReactiveLuminousFluid {
 struct Pulse {
  uint32_t born=0;float power=0.f;bool active=false;
  Pulse()=default;
  Pulse(uint32_t b,float p,bool a):born(b),power(p),active(a){}
 };
 Pulse waves_[4]{};
 unsigned next_=0;
 float previousKey_=0.f;
 static float clip(float f,float a=0.f,float b=1.f){return f<a?a:(f>b?b:f);}
 static uint16_t blend565(uint16_t a,uint16_t b){
  return uint16_t((((unsigned(a>>11&31)+unsigned(b>>11&31))/2u)<<11)|
    (((unsigned(a>>5&63)+unsigned(b>>5&63))/2u)<<5)|
    ((unsigned(a&31)+unsigned(b&31))/2u));
 }
 void launch(uint32_t now,float power){waves_[next_]={now,clip(power),true};next_=(next_+1u)%4u;}
 public:
 void reset(){for(auto& p:waves_)p={};next_=0;previousKey_=0.f;}
 // Capture a frame in the orientation's logical coordinates. Portrait turns
 // the fluid scene before rasterization; the WeAct performs display rotation,
 // so never rotate the completed RGB565 buffer a second time.
 bool render(uint16_t* out,uint32_t now,const Motion& imu,float offsetX,float offsetY,
             float keyPulse,uint16_t width=WIDTH,uint16_t height=HEIGHT,
             TelemetryVisuals telemetry={}){
  if(!out||!((width==WIDTH&&height==HEIGHT)||
             (width==HEIGHT&&height==WIDTH)))return false;
  // Draw the fluid in its native 160x80 pixel space. In portrait, turn the
  // scene a quarter turn instead of independently scaling X by .5 and Y by 2.
  // Both normal and reverse orientations receive the same logical frame;
  // the WeAct hardware applies the requested display direction.
  const bool portrait=width==HEIGHT;
  const float sx=1.f,sy=1.f;
  const float t=float(now)*.001f;
  const float key=clip(keyPulse);
  if(key>.32f&&key>previousKey_+.26f)launch(now,key);
  previousKey_=key;
  const float spin=imu.valid?clip(imu.gz,-5.f,5.f):0.f;
  const float torque=spin*.16f;
  const float shape=clip(telemetry.shape),color=clip(telemetry.color);
  const float tempo=clip(telemetry.tempo);
  const float centerX=float(WIDTH)*.5f+clip(offsetX,-64.f,64.f)*.70f*sx+
                      2.f*sx*sinf(t*.37f);
  const float centerY=float(HEIGHT)*.5f+clip(offsetY,-30.f,30.f)*.65f*sy+
                      2.f*sy*sinf(t*.23f);
  const float energy=.20f+.80f*shape;
  const float swing=6.f+13.f*tempo;
  const float xs[3]={centerX+sx*(-29.f+swing*sinf(t*(.65f+.42f*tempo)+torque)),
                      centerX+sx*(7.f+11.f*shape)*sinf(t*(.46f+.5f*tempo)+2.3f),
                      centerX+sx*(29.f+(7.f+12.f*tempo)*cosf(t*(.59f+.3f*tempo)+torque*2.f))};
  const float ys[3]={centerY+sy*(8.f+(6.f+9.f*shape)*sinf(t*.78f+1.3f)*(.4f+fabsf(torque))),
                      centerY+sy*(-10.f+(5.f+10.f*tempo)*cosf(t*.72f+torque)),
                      centerY+sy*(7.f-(6.f+8.f*shape)*sinf(t*.8f+2.f)*(.4f+fabsf(torque)))};
  const float radiiX[3]={sx*(20.f+energy*4.f),sx*(23.f+energy*4.f),sx*(20.f+energy*4.f)};
  const float radiiY[3]={sy*(14.f+energy*3.f),sy*(16.f+energy*3.f),sy*(14.f+energy*3.f)};
  float light[WIDTH],bend[WIDTH];
  for(int x=0;x<int(WIDTH);++x){
   float l=0.f,b=0.f;
   for(auto &pulse:waves_){
    if(!pulse.active)continue;
    const uint32_t elapsed=now-pulse.born;
    if(elapsed>2100u){pulse.active=false;continue;}
    const float age=float(elapsed)*.001f;
    const float front=centerX+sx*(-29.f+age*(88.f+fabsf(spin)*3.f));
    const float distance=float(x)-front;
    const float q=distance*.10f/sx;
    const float decay=(1.f-.37f*age);
    const float band=clip(pulse.power*decay)/(1.f+q*q*q*q);
    l+=band;
    b+=band*distance*.16f/sx;
   }
   light[x]=clip(l,0.f,1.5f);
   bend[x]=clip(b,-1.f,1.f);
  }
  auto exactPixel=[&](int x,int y)->uint16_t{
   const int sceneX=portrait?y:x;
   const float xx=float(sceneX),yy=portrait?float(width-1-x):float(y);
   const float deformation=bend[sceneX]*5.f*sy;
   float density=0.f,top=0.f,spec=0.f;
   for(int k=0;k<3;++k){
    const float dx=(xx-xs[k])/radiiX[k];
    const float dy=(yy-ys[k]-deformation)/radiiY[k];
    const float dist=dx*dx+dy*dy;
    const float v=(k==1?1.17f:1.06f)/(1.f+1.45f*dist+1.1f*dist*dist);
    density+=v;top+=v*clip(.5f-.5f*dy);
    const float hx=(xx-xs[k]+5.f*sx)*.10f/sx;
    const float hy=(yy-ys[k]+5.f*sy)*.29f/sy;
    const float hs=hx*hx+hy*hy;
    spec+=clip(1.f-hs)*v;
   }
   const float halo=clip((density-.06f)*1.6f);
   const float outer=clip((density-.13f)*2.2f);
   const float solid=clip((density-.61f)*3.3f);
   const float edge=clip(1.f-fabsf(density-.83f)*3.9f);
   const float internal=light[sceneX]*clip((density-.29f)*2.5f);
   const float highlight=clip((density-.74f)*.82f)*clip(top);
   const float glow=clip(spec*.9f);
   const float red=2.f+7.f*halo+11.f*outer+8.f*edge+solid*(10.f+36.f*highlight+28.f*glow+125.f*color)+internal*65.f;
   const float green=6.f+26.f*halo+35.f*outer+41.f*edge+solid*(20.f+122.f*highlight+75.f*glow+55.f*shape)+internal*100.f;
   const float blue=17.f+48.f*halo+81.f*outer+57.f*edge+solid*(24.f+113.f*highlight+70.f*glow-55.f*color)+internal*94.f;
   return rgb(uint8_t(clip(red,0.f,255.f)),uint8_t(clip(green,0.f,255.f)),uint8_t(clip(blue,0.f,255.f)));
  };
  // Q/4 sampling keeps the expensive field evaluation near 3200 pixels.
  // The final row and column are exact to avoid RGB565 edge seams.
  for(int y=0;y<int(height);y+=(y==int(height)-2?1:2)){
   for(int x=0;x<int(width);x+=(x==int(width)-2?1:2))out[size_t(y)*width+x]=exactPixel(x,y);
   for(int x=1;x<int(width)-1;x+=2)out[size_t(y)*width+x]=blend565(out[size_t(y)*width+x-1],out[size_t(y)*width+x+1]);
  }
  for(int y=1;y<int(height)-1;y+=2){
   for(int x=0;x<int(width);++x)out[size_t(y)*width+x]=blend565(out[size_t(y-1)*width+x],out[size_t(y+1)*width+x]);
   out[size_t(y)*width+width-1]=exactPixel(int(width)-1,y);
  }
  for(int x=1;x<int(width)-1;x+=2)out[size_t(height-1)*width+x]=exactPixel(x,int(height)-1);
  return true;
 }
};

// Small logical-corner markers make reverse orientations observable on the
// hardware without mixing software pixel rotation into the animation.
inline void markFrameCorners(uint16_t*frame,uint16_t w,uint16_t h){
 if(!frame||w<4||h<4)return;
 const uint16_t colors[4]={rgb(255,40,40),rgb(40,255,40),
                           rgb(40,120,255),rgb(255,220,40)};
 for(uint16_t y=0;y<4;++y)for(uint16_t x=0;x<4;++x){
  frame[size_t(y)*w+x]=colors[0];
  frame[size_t(y)*w+(w-1-x)]=colors[1];
  frame[size_t(h-1-y)*w+x]=colors[2];
  frame[size_t(h-1-y)*w+(w-1-x)]=colors[3];
 }
}
}
