#pragma once
// Minimal primitives shared by the no-microphone renderer and motion model.
#include <stdint.h>
#include "WeActProtocol.h"
namespace weact {
struct Motion {
 bool valid=false;
 float ax=0.f,ay=0.f,gz=0.f;
};
inline uint16_t rgb(uint8_t red,uint8_t green,uint8_t blue){
 return uint16_t((uint16_t(red>>3)<<11)|(uint16_t(green>>2)<<5)|uint16_t(blue>>3));
}
} // namespace weact
