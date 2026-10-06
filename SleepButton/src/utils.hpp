#ifndef UTILS
#define UTILS

#include <stdint.h>
#include "constants.hpp"
#include <Adafruit_NeoPixel.h>

void getTime(uint8_t* h, uint8_t* m) {

}

void setPixelXY(Adafruit_NeoPixel& strip, uint8_t x, uint8_t y, uint32_t color) {
  if(x >= LIGHTS_PER_ROW || x < 0) {
    return;
  }
  else if(y >= NUM_ROWS || y < 0) {
    return;
  }

  int index = LIGHTS_PER_ROW * y;
  if(y%2==0) {
    index += x;
  }
  else {
    index += (LIGHTS_PER_ROW - x - 1);
  } 

  strip.setPixelColor(index, color);
}

#endif