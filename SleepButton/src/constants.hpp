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

constexpr uint8_t clock_digit_level = 217;
constexpr uint8_t clock_colon_level = 58;

// daily window when the button is active; it may cross midnight
constexpr int active_start_hour = 7;
constexpr int active_start_minute = 0;
constexpr int active_end_hour = 8;
constexpr int active_end_minute = 0;

constexpr unsigned long clock_sync_interval_ms = 60UL * 60 * 1000;
constexpr unsigned long wifi_timeout_ms = 15000;

enum Alignment {
  LEFT_ALIGN,
  CENTER_ALIGN,
  RIGHT_ALIGN
};

#endif