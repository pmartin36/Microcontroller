#ifndef CHAR_SETTERS
#define CHAR_SETTERS

#include <Arduino.h>
#include "utils.hpp"
#include <stdint.h>
#include <Adafruit_NeoPixel.h>

void writeDigitAtPosition(Adafruit_NeoPixel& strip, uint8_t num, uint8_t pos, uint32_t color) {
  if(num > 9) {
    Serial.println("Invalid input - specified num > 9 to single digit num display");
    return;
  }

  for(int y = 0; y < NUM_ROWS; y++) {
    for(int xo = 0; xo < 3; xo++) {
      int x = pos + xo;
      if((DIGITS[num][y] & (4 >> xo)) != 0) {
        setPixelXY(strip, x, y, color);
      }
    }
  }
}

void displayAlignedNumber(Adafruit_NeoPixel& strip, uint16_t num, uint8_t pos, uint32_t color, uint8_t min_digits = 0, Alignment alignment = Alignment::RIGHT_ALIGN) {
  uint8_t digits = 0;
  uint16_t dcount = num;
  while(dcount >= 1) {
    dcount /= 10;
    digits++;
  }
  digits = max(digits, min_digits);
  uint8_t size = digits * 3 + (digits-1); // 3 columns per digit, 1 space between each digit

  uint8_t nextPos = pos;
  if(alignment == Alignment::LEFT_ALIGN) {
    nextPos += size;
  }
  else if(alignment == Alignment::CENTER_ALIGN) {
    nextPos += (size / 2) + 1;
  }
  else {
    nextPos++;
  }

  nextPos -= 3;
  while(digits >= 1) {
    uint8_t digit = num % 10;

    writeDigitAtPosition(strip, digit, nextPos, color);
    nextPos -= 4;
    digits--;

    num /= 10;
  }
}

#endif