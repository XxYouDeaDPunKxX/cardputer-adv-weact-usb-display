#pragma once
// Observed WeAct Display FS V1 0.96-inch USB CDC wire protocol.
// Independent of Arduino, ESP32 and the demo.
#include <stdint.h>
#include <stddef.h>
#include <string.h>
namespace weact {
// Standard landscape dimensions for this panel.
static constexpr uint16_t WIDTH=160,HEIGHT=80;
enum class Orientation:uint8_t{
 Portrait=0,ReversePortrait=1,Landscape=2,ReverseLandscape=3
};
inline bool validOrientation(Orientation o){return uint8_t(o)<=3;}
inline uint16_t orientationWidth(Orientation o){
 return o==Orientation::Portrait||o==Orientation::ReversePortrait?80:160;
}
inline uint16_t orientationHeight(Orientation o){
 return o==Orientation::Portrait||o==Orientation::ReversePortrait?160:80;
}
inline bool fits(uint16_t x,uint16_t y,uint16_t w,uint16_t h,
                 uint16_t screenW,uint16_t screenH){
 return w!=0&&h!=0&&uint32_t(x)+w<=screenW&&uint32_t(y)+h<=screenH;
}
inline bool fits(uint16_t x,uint16_t y,uint16_t w,uint16_t h){
 return fits(x,y,w,h,WIDTH,HEIGHT);
}
inline void versionQuery(uint8_t out[2]){out[0]=0xc2;out[1]=0x0a;}
inline bool versionValid(const uint8_t*reply,size_t n){
 return reply&&n==19&&reply[0]==0xc2&&reply[18]==0x0a&&
        memcmp(reply+1,"V1.0.0.0",8)==0;
}
inline void orientationPacket(Orientation orientation,uint8_t out[3]){
 out[0]=0x02;out[1]=uint8_t(orientation);out[2]=0x0a;
}
inline void landscape(uint8_t out[3]){orientationPacket(Orientation::Landscape,out);}
inline void brightnessPacket(uint8_t percent,uint16_t fadeMs,uint8_t out[5]){
 if(percent>100)percent=100;
 out[0]=0x03;out[1]=uint8_t(uint16_t(percent)*255u/100u);
 out[2]=uint8_t(fadeMs);out[3]=uint8_t(fadeMs>>8);out[4]=0x0a;
}
inline void brightnessPacket(uint8_t percent,uint8_t out[5]){
 brightnessPacket(percent,1000,out);
}
inline bool bitmapHeader(uint16_t x,uint16_t y,uint16_t w,uint16_t h,
                         Orientation orientation,uint8_t out[10],uint8_t opcode=0x05){
 if(!validOrientation(orientation)||
    !fits(x,y,w,h,orientationWidth(orientation),orientationHeight(orientation)))return false;
 const uint16_t x1=uint16_t(x+w-1),y1=uint16_t(y+h-1);
 out[0]=opcode;out[1]=uint8_t(x);out[2]=uint8_t(x>>8);
 out[3]=uint8_t(y);out[4]=uint8_t(y>>8);
 out[5]=uint8_t(x1);out[6]=uint8_t(x1>>8);
 out[7]=uint8_t(y1);out[8]=uint8_t(y1>>8);out[9]=0x0a;
 return true;
}
inline bool bitmapHeader(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint8_t out[10]){
 return bitmapHeader(x,y,w,h,Orientation::Landscape,out);
}
inline void pixel565(uint16_t pixel,uint8_t*out){
 out[0]=uint8_t(pixel);out[1]=uint8_t(pixel>>8);
}
inline bool fillRectPacket(uint16_t x,uint16_t y,uint16_t w,uint16_t h,
                           Orientation orientation,uint16_t color,uint8_t out[12]){
 if(!validOrientation(orientation)||
    !fits(x,y,w,h,orientationWidth(orientation),orientationHeight(orientation)))return false;
 const uint16_t x1=uint16_t(x+w-1),y1=uint16_t(y+h-1);
 out[0]=0x04;
 out[1]=uint8_t(x);out[2]=uint8_t(x>>8);
 out[3]=uint8_t(y);out[4]=uint8_t(y>>8);
 out[5]=uint8_t(x1);out[6]=uint8_t(x1>>8);
 out[7]=uint8_t(y1);out[8]=uint8_t(y1>>8);
 pixel565(color,out+9);out[11]=0x0a;
 return true;
}
inline bool fillPacket(Orientation orientation,uint16_t color,uint8_t out[12]){
 if(!validOrientation(orientation))return false;
 return fillRectPacket(0,0,orientationWidth(orientation),orientationHeight(orientation),
                       orientation,color,out);
}
inline void freePacket(uint8_t out[2]){out[0]=0x07;out[1]=0x0a;}
}
