#ifndef DISPLAY_SETTERS
#define DISPLAY_SETTERS

#include <Adafruit_NeoPixel.h>
#include "utils.hpp"
#include "char_setters.hpp"

void setDisplayAsClockTime(struct tm* t, Adafruit_NeoPixel& strip, uint32_t color, uint32_t colonColor, int yOffset = 0) {
  uint8_t h, m;
  getTimeFromTimeStruct(t, &h, &m);

  // columns: tens-of-hours 0-2, gap 3, hour 4-6, colon 7, minutes 8-10 and 12-14
  writeDigitAtPosition(strip, h / 10, 0, color, yOffset);
  writeDigitAtPosition(strip, h % 10, 4, color, yOffset);

  // colon
  setPixelXY(strip, 7, text_top_row + 1 + yOffset, colonColor);
  setPixelXY(strip, 7, text_top_row + 3 + yOffset, colonColor);

  displayAlignedNumber(strip, m, 8, color, 2, Alignment::LEFT_ALIGN, yOffset);
}

// "--:--" in the clock layout, for when there is no valid time
void setDisplayAsClockPlaceholder(Adafruit_NeoPixel& strip, uint32_t color, uint32_t colonColor, int yOffset = 0) {
  writeGlyphAtPosition(strip, GLYPH_DASH, 0, color, yOffset);
  writeGlyphAtPosition(strip, GLYPH_DASH, 4, color, yOffset);

  setPixelXY(strip, 7, text_top_row + 1 + yOffset, colonColor);
  setPixelXY(strip, 7, text_top_row + 3 + yOffset, colonColor);

  writeGlyphAtPosition(strip, GLYPH_DASH, 8, color, yOffset);
  writeGlyphAtPosition(strip, GLYPH_DASH, 12, color, yOffset);
}

void setDisplayAsNumber(uint16_t num, Adafruit_NeoPixel& strip, uint32_t color, int yOffset = 0) {
  displayAlignedNumber(strip, num, lights_per_row / 2, color, 0, Alignment::CENTER_ALIGN, yOffset);
}

// recolors the lit pixels in rows top through bottom with a rainbow that steps a sixth of the wheel per row from hue
void recolorRowsAsRainbow(Adafruit_NeoPixel& strip, int top, int bottom, uint16_t hue, uint8_t level) {
  for(int y = max(top, 0); y <= bottom && y < num_rows; y++) {
    uint32_t color = strip.gamma32(strip.ColorHSV(hue + (y - top) * (65536 / 6), 255, level));
    for(int x = 0; x < lights_per_row; x++) {
      int index = getPixelIndexXY(x, y);
      if(strip.getPixelColor(index) != 0) {
        strip.setPixelColor(index, color);
      }
    }
  }
}

// shows a 0-23 hour as 12-hour time with an A/P suffix, e.g. "11P", centered
void setDisplayAsHour(uint8_t hour, Adafruit_NeoPixel& strip, uint32_t color) {
  uint8_t h = hour % 12;
  if(h == 0) h = 12;

  uint8_t digits = h >= 10 ? 2 : 1;
  uint8_t width = digits * 4 + 3;
  uint8_t pos = (lights_per_row - width) / 2;

  if(h >= 10) {
    writeDigitAtPosition(strip, 1, pos, color);
    pos += 4;
    h -= 10;
  }
  writeDigitAtPosition(strip, h, pos, color);
  writeGlyphAtPosition(strip, hour >= 12 ? GLYPH_P : GLYPH_A, pos + 4, color);
}

// fills the rightmost column from the bottom of the text rows, one pixel per remaining step
void setDisplayAsCountdown(uint8_t remaining, Adafruit_NeoPixel& strip, uint32_t color) {
  for(int i = 0; i < remaining && i < glyph_height; i++) {
    setPixelXY(strip, lights_per_row - 1, num_rows - 1 - i, color);
  }
}

// "0D:BG" in the clock layout, standing in for the time in debug mode
void setDisplayAsDebugClock(Adafruit_NeoPixel& strip, uint32_t color, uint32_t colonColor, int yOffset = 0) {
  writeDigitAtPosition(strip, 0, 0, color, yOffset);
  writeGlyphAtPosition(strip, GLYPH_D, 4, color, yOffset);

  setPixelXY(strip, 7, text_top_row + 1 + yOffset, colonColor);
  setPixelXY(strip, 7, text_top_row + 3 + yOffset, colonColor);

  writeGlyphAtPosition(strip, GLYPH_B, 8, color, yOffset);
  writeGlyphAtPosition(strip, GLYPH_G, 12, color, yOffset);
}

#endif