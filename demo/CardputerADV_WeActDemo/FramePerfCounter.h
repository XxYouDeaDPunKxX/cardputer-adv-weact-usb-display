#pragma once
// Production: rolling averages of completed WeAct full frames.
// No USB implementation, heap, NVS, or serial logging.
#include <stdint.h>
namespace weact {
struct FramePerfSample {
 uint32_t count=0;
 uint32_t renderAvgMs=0,usbAvgMs=0;
};
class FramePerfCounter {
 uint64_t renderSumUs_=0,usbSumUs_=0;
 uint32_t count_=0;
 static uint32_t roundedMs(uint64_t micros){
  return uint32_t((micros+500u)/1000u);
 }
 public:
 void reset(){
  renderSumUs_=usbSumUs_=0;
  count_=0;
 }
 void add(uint32_t renderUs,uint32_t usbUs){
  renderSumUs_+=renderUs;usbSumUs_+=usbUs;
  ++count_;
 }
 FramePerfSample take(){
  FramePerfSample s{};
  if(count_){
   s.count=count_;
   s.renderAvgMs=roundedMs(renderSumUs_/count_);
   s.usbAvgMs=roundedMs(usbSumUs_/count_);
  }
  reset();
  return s;
 }
};
} // namespace weact
