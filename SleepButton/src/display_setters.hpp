#ifndef DISPLAY_SETTERS
#define DISPLAY_SETTERS

#include <Adafruit_NeoPixel.h>
#include "utils.hpp"
#include "char_setters.hpp"

void setDisplayAsClockTime(struct tm* t, Adafruit_NeoPixel& strip, uint32_t color, uint32_t colonColor) {
  strip.clear();

  uint8_t h, m;
  getTimeFromTimeStruct(t, &h, &m);

  // columns: tens-of-hours 1 at 0-2, gap 3, hour 4-6, colon 7, minutes 8-10 and 12-14
  if(h >= 10) {
    writeDigitAtPosition(strip, 1, 0, color);
    h-=10;
  }

  writeDigitAtPosition(strip, h, 4, color);

  // colon
  setPixelXY(strip, 7, text_top_row + 1, colonColor);
  setPixelXY(strip, 7, text_top_row + 3, colonColor);

  displayAlignedNumber(strip, m, 8, color, 2, Alignment::LEFT_ALIGN);
}

void setDisplayAsNumber(uint16_t num, Adafruit_NeoPixel& strip, uint32_t color) {
  displayAlignedNumber(strip, num, lights_per_row-1, color, 0, Alignment::RIGHT_ALIGN);
}

void setDisplayAsDebug(Adafruit_NeoPixel& strip, uint32_t color) {
  // "DBG" is 11 columns wide (3 glyphs + 2 gaps), centered in 15
  writeGlyphAtPosition(strip, GLYPH_D, 2, color);
  writeGlyphAtPosition(strip, GLYPH_B, 6, color);
  writeGlyphAtPosition(strip, GLYPH_G, 10, color);
}

#endif