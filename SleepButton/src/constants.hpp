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

// the button is active daily for active_duration_minutes starting at the active hour
constexpr int default_active_hour = 7;
constexpr int active_duration_minutes = 60;

constexpr unsigned long long_press_ms = 2000;
constexpr unsigned long setting_timeout_ms = 5000;
constexpr unsigned long setting_flash_ms = 250;

// light blue
constexpr uint8_t setting_r = 110;
constexpr uint8_t setting_g = 190;
constexpr uint8_t setting_b = 255;

constexpr unsigned long clock_sync_interval_ms = 60UL * 60 * 1000;
constexpr unsigned long wifi_timeout_ms = 15000;

enum Alignment {
  LEFT_ALIGN,
  CENTER_ALIGN,
  RIGHT_ALIGN
};

#endif