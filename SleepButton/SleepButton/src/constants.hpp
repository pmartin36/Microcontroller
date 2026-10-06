#ifndef CONSTANTS
#define CONSTANTS

#include <stdint.h>

#define DEBOUNCE_MS 30
#define LIGHTS_PER_ROW 15
#define NUM_ROWS 5
#define LIGHT_PIN 25

enum Alignment {
  LEFT_ALIGN,
  CENTER_ALIGN,
  RIGHT_ALIGN
};

const uint8_t DIGITS[10][5] = {
  {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
  {0b001, 0b001, 0b001, 0b001, 0b001}, // 1
  {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
  {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
  {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
  {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
  {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
  {0b111, 0b001, 0b001, 0b001, 0b001}, // 7
  {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
  {0b111, 0b101, 0b111, 0b001, 0b111}, // 9
};

#endif