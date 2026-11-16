#ifndef GLYPHS
#define GLYPHS

#include <stdint.h>

constexpr int glyph_height = 5;

const uint8_t DIGITS[10][glyph_height] = {
  {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
  {0b010, 0b110, 0b010, 0b010, 0b111}, // 1
  {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
  {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
  {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
  {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
  {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
  {0b111, 0b001, 0b001, 0b001, 0b001}, // 7
  {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
  {0b111, 0b101, 0b111, 0b001, 0b111}, // 9
};

const uint8_t GLYPH_D[glyph_height] = {0b110, 0b101, 0b101, 0b101, 0b110};
const uint8_t GLYPH_B[glyph_height] = {0b110, 0b101, 0b110, 0b101, 0b110};
const uint8_t GLYPH_G[glyph_height] = {0b011, 0b100, 0b101, 0b101, 0b011};
const uint8_t GLYPH_A[glyph_height] = {0b010, 0b101, 0b111, 0b101, 0b101};
const uint8_t GLYPH_P[glyph_height] = {0b110, 0b101, 0b110, 0b100, 0b100};

#endif
