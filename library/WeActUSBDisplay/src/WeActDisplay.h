#pragma once
// Reusable WeAct FS V1 driver with an injected USB transport.
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "WeActProtocol.h"
namespace weact {
struct Device {uint16_t vid,pid;uint8_t address;bool cdcReady;};
enum class State:uint8_t{Disconnected,Ready,Faulted};
enum class Error:uint8_t{None,Identity,Connect,Disconnected,Version,Incomplete,Bounds,NotReady,Payload};
struct ByteTransport {
 virtual ~ByteTransport()=default;
 virtual bool open(uint8_t address)=0;
 virtual bool connected() const=0;
 virtual size_t write(const uint8_t*bytes,size_t size)=0;
 // Must confirm exact number of payload bytes physically completed since prior flush.
 virtual bool flush(size_t expectedBytes)=0;
 virtual bool readExact(uint8_t*into,size_t size,uint32_t timeoutMs)=0;
 virtual void close()=0;
};
// Caller-supplied FastLZ blocks; no compressor dependency in the core driver.
struct FastLzChunk {
 const uint8_t*data;
 uint16_t compressedBytes;
 uint16_t rawBytes;
};
class Driver{
 ByteTransport &io_;
 State state_=State::Disconnected;
 Error error_=Error::None;
 Orientation orientation_=Orientation::Landscape;
 uint8_t brightness_=60,resumeBrightness_=60;
 char version_[18]={};
 uint32_t frames_=0;
 bool fault(Error code){error_=code;state_=State::Faulted;return false;}
 bool usable(){
  if(state_==State::Ready)return true;
  if(state_!=State::Faulted)error_=Error::NotReady;
  return false;
 }
 bool command(const uint8_t*data,size_t size){
  if(!io_.connected())return fault(Error::Disconnected);
  if(io_.write(data,size)!=size)return fault(Error::Incomplete);
  if(!io_.flush(size))return fault(Error::Incomplete);
  return true;
 }
 bool writePayload(const uint8_t*data,size_t size){
  if(!io_.connected())return fault(Error::Disconnected);
  if(io_.write(data,size)!=size)return fault(Error::Incomplete);
  return true;
 }
 public:
 explicit Driver(ByteTransport&io):io_(io){}
 State state() const{return state_;}
 bool ready()const{return state_==State::Ready;}
 Error lastError()const{return error_;}
 uint8_t brightness()const{return brightness_;}
 uint32_t frames()const{return frames_;}
 Orientation orientation()const{return orientation_;}
 uint16_t width()const{return orientationWidth(orientation_);}
 uint16_t height()const{return orientationHeight(orientation_);}
 const char* firmwareVersion()const{return version_;}
 bool connect(const Device&dev){
  if(state_==State::Faulted)return false;
  if(state_==State::Ready)return false;
  error_=Error::None;
  version_[0]='\0';
  if(dev.vid!=0x1a86||dev.pid!=0xfe0c||!dev.address||!dev.cdcReady){error_=Error::Identity;return false;}
  if(!io_.open(dev.address))return fault(Error::Connect);
  uint8_t request[2];versionQuery(request);
  if(!command(request,2))return false;
  uint8_t response[19]={};
  if(!io_.readExact(response,19,1500)||!versionValid(response,19))return fault(Error::Version);
  memcpy(version_,response+1,17);version_[17]='\0';
  uint8_t rot[3];landscape(rot);
  if(!command(rot,3))return false;
  uint8_t brightness[5];brightnessPacket(brightness_,brightness);
  if(!command(brightness,5))return false;
  orientation_=Orientation::Landscape;
  state_=State::Ready;error_=Error::None;return true;
 }
 bool setOrientation(Orientation orientation){
  if(!usable())return false;
  if(!validOrientation(orientation)){error_=Error::Bounds;return false;}
  uint8_t packet[3];orientationPacket(orientation,packet);
  if(!command(packet,sizeof(packet)))return false;
  orientation_=orientation;error_=Error::None;return true;
 }
 bool setBrightness(uint8_t percent,uint16_t fadeMs=1000){
  if(!usable())return false;
  if(percent>100)percent=100;
  uint8_t commandBytes[5];brightnessPacket(percent,fadeMs,commandBytes);
  if(!command(commandBytes,5))return false;
  if(percent)resumeBrightness_=percent;
  brightness_=percent;error_=Error::None;return true;
 }
 // Screen-off uses brightness 0; no separate screen-off opcode is known.
 bool screenOff(uint16_t fadeMs=1000){return setBrightness(0,fadeMs);}
 bool screenOn(uint16_t fadeMs=1000){return setBrightness(resumeBrightness_,fadeMs);}
 bool fillScreen(uint16_t color){
  if(!usable())return false;
  uint8_t packet[12];fillPacket(orientation_,color,packet);
  if(!command(packet,sizeof(packet)))return false;
  ++frames_;error_=Error::None;return true;
 }
 // Solid-color rectangle using opcode 0x04; partial fills need panel validation.
 bool fillRect(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t color){
  if(!usable())return false;
  uint8_t packet[12];
  if(!fillRectPacket(x,y,w,h,orientation_,color,packet)){
   error_=Error::Bounds;return false;
  }
  if(!command(packet,sizeof(packet)))return false;
  ++frames_;error_=Error::None;return true;
 }
 bool drawRect(uint16_t x,uint16_t y,uint16_t w,uint16_t h,const uint16_t*pixels){
  if(!usable())return false;
  if(!pixels||!fits(x,y,w,h,width(),height())){error_=Error::Bounds;return false;}
  uint8_t head[10];bitmapHeader(x,y,w,h,orientation_,head);
  if(!command(head,10))return false;
  // Protocol receiver expects exactly w*h pixels; bounded 2-row transfers.
  uint8_t buf[WIDTH*4];
  for(uint16_t row=0;row<h;row+=2){
   size_t bytes=0;
   for(uint16_t dy=0;dy<2&&row+dy<h;dy++)
    for(uint16_t col=0;col<w;col++){
     pixel565(pixels[size_t(row+dy)*w+col],buf+bytes);
     bytes+=2;
    }
   if(!writePayload(buf,bytes))return false;
  }
  if(!io_.flush(size_t(w)*h*2))return fault(Error::Incomplete);
  ++frames_;error_=Error::None;return true;
 }
 // Opcode 0x15: blocks already compressed by caller using FastLZ.
 // Chunk payload is [rawSize LE16][compressedSize LE16][FastLZ bytes].
 // The compressed path still needs physical verification on this panel.
 bool drawRectFastLz(uint16_t x,uint16_t y,uint16_t w,uint16_t h,
                     const FastLzChunk*chunks,size_t chunkCount){
  if(!usable())return false;
  if(!fits(x,y,w,h,width(),height())){error_=Error::Bounds;return false;}
  if(!chunks||!chunkCount){error_=Error::Payload;return false;}
  const size_t totalRaw=size_t(w)*h*2;
  size_t checkedRaw=0,sendBytes=0;
  for(size_t i=0;i<chunkCount;i++){
   const FastLzChunk&chunk=chunks[i];
   if(!chunk.data||!chunk.rawBytes||!chunk.compressedBytes||
      chunk.rawBytes>totalRaw-checkedRaw){error_=Error::Payload;return false;}
   checkedRaw+=chunk.rawBytes;
   sendBytes+=4u+chunk.compressedBytes;
  }
  if(checkedRaw!=totalRaw){error_=Error::Payload;return false;}
  uint8_t head[10];bitmapHeader(x,y,w,h,orientation_,head,0x15);
  if(!command(head,sizeof(head)))return false;
  for(size_t i=0;i<chunkCount;i++){
   const FastLzChunk&chunk=chunks[i];
   uint8_t sizes[4]={uint8_t(chunk.rawBytes),uint8_t(chunk.rawBytes>>8),
                     uint8_t(chunk.compressedBytes),uint8_t(chunk.compressedBytes>>8)};
   if(!writePayload(sizes,sizeof(sizes))||
      !writePayload(chunk.data,chunk.compressedBytes))return false;
  }
  if(!io_.flush(sendBytes))return fault(Error::Incomplete);
  ++frames_;error_=Error::None;return true;
 }
 // Reference "Free" command; physical effect still unverified.
 bool sendFreeCommand(){
  if(!usable())return false;
  uint8_t packet[2];freePacket(packet);
  if(!command(packet,sizeof(packet)))return false;
  error_=Error::None;return true;
 }
 void onRemoved(){io_.close();state_=State::Disconnected;error_=Error::None;version_[0]='\0';}
};
} // namespace weact
