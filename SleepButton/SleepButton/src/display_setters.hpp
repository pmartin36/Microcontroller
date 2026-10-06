#ifndef DISPLAY_SETTERS
#define DISPLAY_SETTERS

#include <Adafruit_NeoPixel.h>
#include "utils.hpp"
#include "char_setters.hpp"

void setDisplayAsClockTime(Adafruit_NeoPixel strip, uint32_t color) {
  strip.clear();

  uint8_t h, m;
  getTime(&h, &m);

  // 1 takes up 1 column, or no columns and we show nothing
  if(h >= 10) {
    for(int i = 0; i < NUM_ROWS; i++) {
      setPixelXY(strip, 0, i, color);
    }
    h-=10;
  }

  writeDigitAtPosition(strip, h, 2, color);

  // colon
  setPixelXY(strip, 6, 1, color);
  setPixelXY(strip, 6, 3, color);

  displayAlignedNumber(strip, m, 8, color, 2, Alignment::LEFT_ALIGN);
}

void setDisplayAsNumber() {

}

#endif