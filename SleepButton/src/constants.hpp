#ifndef CONSTANTS
#define CONSTANTS

#include <stdint.h>

#define DEBOUNCE_MS 30
#define LIGHT_PIN 25
#define INPUT_BUTTON_PIN 27
#define FIREWORK_PIN 32
#define ONBOARD_LED_PIN 2
#define DEBUG_PIN 23

constexpr int lights_per_row = 15;
constexpr int num_rows = 6; // physical rows, row 0 is the bottom
constexpr int total_lights = lights_per_row*num_rows;

constexpr int total_firework_lights = 47;

enum Alignment {
  LEFT_ALIGN,
  CENTER_ALIGN,
  RIGHT_ALIGN
};

#endif