#pragma once
// ESP32-S3 implementation of WeAct ByteTransport using the EspUsbHost library.
// All protocol policy and visuals live in separate pure-C++ modules.
#include <Arduino.h>
#include <EspUsbHost.h>
#include "WeActDisplay.h"
namespace weact {
class EspUsbHostTransport final: public ByteTransport {
 EspUsbHost&host_;
 EspUsbHostCdcSerial&port_;
 uint8_t address_=0;
 bool queue_=false;
 uint64_t beforeBytes_=0,expected_=0;
 uint32_t beforeErrors_=0;
 public:
 EspUsbHostTransport(EspUsbHost&h,EspUsbHostCdcSerial&p):host_(h),port_(p){}
 bool open(uint8_t address)override{
  if(address_!=0)close();
  if(!address||!host_.serialReady(address,0))return false;
  port_.setAddress(address);port_.setPort(0);
  if(!port_.connected()){port_.clearAddress();return false;}
  (void)port_.setDtr(true);(void)port_.setRts(true);
  while(port_.available()>0)(void)port_.read();
  if(!host_.setSerialBaudRate(115200,address,0)){
   port_.clearAddress();return false;
  }
  if(!host_.serialWriteQueueBegin(3,1024,address,0)){
   port_.clearAddress();return false;
  }
  address_=address;queue_=true;expected_=0;
  return true;
 }
 bool connected()const override{
  return address_!=0&&host_.serialReady(address_,0)&&port_.connected();
 }
 size_t write(const uint8_t*data,size_t length)override{
  if(!connected()||!data)return 0;
  if(expected_==0){
   const auto counters=host_.serialWriteStats(address_,0);
   beforeBytes_=counters.bytes;
   beforeErrors_=counters.errors;
  }
  const size_t accepted=port_.write(data,length);
  expected_+=accepted;
  return accepted;
 }
 bool flush(size_t expectedBytes)override{
  if(!connected()||expected_!=expectedBytes)return false;
  if(!host_.serialWriteFlush(2500,address_,0))return false;
  const auto counters=host_.serialWriteStats(address_,0);
  const bool success=(counters.bytes>=beforeBytes_ &&
                      counters.bytes-beforeBytes_==expectedBytes &&
                      counters.errors==beforeErrors_);
  if(success)expected_=0;
  return success;
 }
 bool readExact(uint8_t*dst,size_t n,uint32_t waitMs)override{
  if(!dst||!connected())return false;
  size_t at=0;const uint32_t started=millis();
  while(at<n&&connected()&&millis()-started<waitMs){
   if(port_.available()>0){int v=port_.read();if(v>=0)dst[at++]=uint8_t(v);}
   else delay(2);
  }
  return at==n;
 }
 void close()override{
  if(queue_&&address_)host_.serialWriteQueueEnd(address_,0);
  if(address_)port_.clearAddress();
  queue_=false;address_=0;expected_=0;
 }
};
} // namespace weact
