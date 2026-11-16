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

// a one-pixel-wide arch on both sides of the centered number drawn by setDisplayAsNumber, mirrored, with a
// blank column beside the number; its crest bounces between the number and the screen edge at a constant
// speed, with its height scaled by envelope (0 to 1); its colors come from a rainbow that flows outward
// across the columns faster than the crest moves
void setDisplayAsWave(uint16_t num, Adafruit_NeoPixel& strip, unsigned long elapsedMs, float envelope, uint8_t level, int yOffset = 0) {
  int halfWidth = (digitCount(num) * 4 - 2) / 2;
  int left = lights_per_row / 2 - halfWidth;
  int right = lights_per_row / 2 + halfWidth;

  // columns available on each side past the blank one
  int span = left - 1;
  float oneWayMs = max(span - 1, 1) * wave_column_ms;

  // a triangle wave from 0 to 1 and back, so the crest reverses at each end without slowing
  float phase = fmodf(elapsedMs / oneWayMs, 2);
  float crest = 1 + (span - 1) * (1 - fabsf(phase - 1));

  int bottomRow = text_top_row + glyph_height - 1 + yOffset;
  for(int dist = 1; dist <= span; dist++) {
    float shape = max(0.f, 1 - fabsf(dist - crest) / wave_half_width);
    float height = (glyph_height - 1) * shape * envelope;
    if(height <= 0) continue;

    // split the brightness between the two rows around the exact height so the arch glides between rows;
    // height 0 is the bottom text row, which the arch never lights
    int lower = (int)height;
    float upperWeight = height - lower;
    float hueTurns = (dist - (float)elapsedMs / wave_hue_column_ms) / wave_rainbow_columns;
    uint16_t hue = (uint16_t)(int32_t)(hueTurns * 65536);

    if(lower >= 1) {
      uint32_t color = strip.gamma32(strip.ColorHSV(hue, 255, level * (1 - upperWeight)));
      setPixelXY(strip, left - 1 - dist, bottomRow - lower, color);
      setPixelXY(strip, right + 1 + dist, bottomRow - lower, color);
    }
    if(upperWeight > 0 && lower + 1 <= glyph_height - 1) {
      uint32_t color = strip.gamma32(strip.ColorHSV(hue, 255, level * upperWeight));
      setPixelXY(strip, left - 1 - dist, bottomRow - lower - 1, color);
      setPixelXY(strip, right + 1 + dist, bottomRow - lower - 1, color);
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