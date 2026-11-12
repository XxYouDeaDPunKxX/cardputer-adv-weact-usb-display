#pragma once
// WeAct 160x80 demo: autonomous free-roam plus tilt-controlled inertial bias.
// BMI270 acceleration is NOT absolute position. No physical tracking claimed.
// The blob can traverse the screen even while the Cardputer rests on a table.
#include <stdint.h>
#include <math.h>
#include "RenderTypes.h"
namespace weact {
struct Position {float x=0.f,y=0.f;};
class MotionDynamics {
 Position position_{};
 float tiltX_=0.f,tiltY_=0.f;
 float vx_=0.f,vy_=0.f;
 uint32_t previousMs_=0;
 static float clamp(float v,float lo,float hi){return v<lo?lo:(v>hi?hi:v);}
 public:
 Position position() const{return position_;}
 Position update(uint32_t timeMs,const Motion&imu){
  uint32_t elapsed=0;
  if(previousMs_!=0)elapsed=timeMs-previousMs_;
  previousMs_=timeMs;
  if(elapsed>200)elapsed=200; // do not fling an inertial offset after a long USB wait
  const float targetX=imu.valid?clamp(imu.ax,-1.5f,1.5f)*24.f:0.f;
  const float targetY=imu.valid?clamp(imu.ay,-1.5f,1.5f)*13.f:0.f;
  // Critically damped-ish tilt bias; it can reverse but never forces the
  // autonomous animation back to screen center.
  const int steps=(int(elapsed)+15)/16;
  const float dt=steps>0?float(elapsed)*.001f/float(steps):0.f;
  for(int step=0;step<steps;step++){
   const float accelX=27.f*(targetX-tiltX_)-10.f*vx_;
   const float accelY=27.f*(targetY-tiltY_)-10.f*vy_;
   vx_=clamp(vx_+accelX*dt,-140.f,140.f);
   vy_=clamp(vy_+accelY*dt,-85.f,85.f);
   tiltX_=clamp(tiltX_+vx_*dt,-36.f,36.f);
   tiltY_=clamp(tiltY_+vy_*dt,-19.f,19.f);
  }
  // Two incommensurate periods on each axis produce continuously wandering
  // paths rather than a tiny rotation around the fixed 80,40 center.
  const float t=float(timeMs)*.001f;
  const float wanderX=51.f*sinf(t*.52f)+10.f*sinf(t*.19f+.7f);
  const float wanderY=23.f*sinf(t*.41f+.9f)+5.f*cosf(t*.79f);
  position_.x=clamp(wanderX+tiltX_,-64.f,64.f);
  position_.y=clamp(wanderY+tiltY_,-30.f,30.f);
  return position_;
 }
};
}
