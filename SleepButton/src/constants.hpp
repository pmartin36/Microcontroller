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
constexpr int default_active_hour = 23;
constexpr int active_duration_minutes = 60;

// the count shows for count_display_ms in the active window and count_display_outside_ms outside it
constexpr unsigned long count_display_ms = 4000;
constexpr unsigned long count_display_outside_ms = count_display_ms / 2;
constexpr unsigned long count_scroll_ms = 250;
constexpr unsigned long count_reveal_delay_ms = 100;
// the wave beside the count sloshes between the count and the screen edge, a column every
// wave_column_ms, and settles flat over the last wave_settle_ms before the count scrolls away
constexpr unsigned long wave_column_ms = 95;
constexpr unsigned long wave_settle_ms = 400;
constexpr float wave_half_width = 3;
// the rainbow along the wave is wave_rainbow_columns long and moves outward a column every wave_hue_column_ms,
// faster than the crest
constexpr float wave_rainbow_columns = 6;
constexpr unsigned long wave_hue_column_ms = 50;

// a celebration effect plays on the firework rings for celebration_ms, slightly longer than the count shows
constexpr unsigned long celebration_ms = 5000;
// rainbow ripple: the colors move out a ring every ripple_ring_ms, shifting ripple_hue_step per ring
constexpr unsigned long ripple_ring_ms = 150;
constexpr uint16_t ripple_hue_step = 65536 / 6;
// fireworks: a burst launches about every burst_interval_ms and spreads out to burst_radius, measured in
// ring spacings, moving one spacing every burst_step_ms; each pixel fades over burst_fade_ms once the burst
// reaches it, and the hue shifts burst_hue_step per spacing out from the burst's center
constexpr unsigned long burst_interval_ms = 300;
constexpr unsigned long burst_jitter_ms = 150;
constexpr unsigned long burst_step_ms = 150;
constexpr float burst_radius = 2.5;
constexpr unsigned long burst_fade_ms = 400;
constexpr uint16_t burst_hue_step = 65536 / 6;
constexpr unsigned long burst_ms = burst_step_ms * burst_radius + burst_fade_ms;
// a burst picks its ring by these weights, center out, then a random pixel on that ring
constexpr uint8_t burst_ring_weights[] = {15, 35, 35, 15};
// spiral: two arms start opposite each other and each go once around every ring from the center out
// the spiral's rainbow spans each arm
// a filling spiral lights outward to half brightness over shape_fill_ms, each arm's pass lighting a pixel
// half way and fading it in over shape_reveal_ms; then brightening sweeps run out the arms like the
// spiral tail, a pixel every spiral_sweep_step_ms, raising spiral_sweep_length pixels toward full; it
// fades out over the last shape_fade_ms
constexpr unsigned long shape_fill_ms = 1500;
constexpr unsigned long shape_reveal_ms = 100;
constexpr unsigned long spiral_sweep_step_ms = 30;
constexpr float spiral_sweep_length = 12;
constexpr unsigned long shape_fade_ms = 400;
// a spiral tail: a head runs out each arm a pixel every spiral_tail_step_ms trailing spiral_tail_length
// pixels, and a new head starts at the center as the previous one reaches the end
constexpr unsigned long spiral_tail_step_ms = 40;
constexpr float spiral_tail_length = 12;
// rotating rings: each ring holds a full rainbow at half brightness turning once every ring_turn_ms,
// neighboring rings in opposite directions, while full brightness ripples out a ring every ring_ripple_ms;
// it fades in and out over ring_fade_ms
constexpr unsigned long ring_turn_ms = 1500;
constexpr unsigned long ring_ripple_ms = 300;
// the crest lifts rings within ring_ripple_half_width rings of it, most at the crest
constexpr float ring_ripple_half_width = 1.5;
constexpr unsigned long ring_fade_ms = 400;

// a press that does not count flashes the outer firework ring up to full and back over ring_flash_ms
constexpr unsigned long ring_flash_ms = 400;

constexpr unsigned long long_press_ms = 2000;
constexpr unsigned long setting_timeout_ms = 5000;
constexpr unsigned long setting_flash_ms = 250;

// light blue
constexpr uint8_t setting_r = 110;
constexpr uint8_t setting_g = 190;
constexpr uint8_t setting_b = 255;

constexpr unsigned long clock_sync_interval_ms = 60UL * 60 * 1000;
constexpr unsigned long wifi_timeout_ms = 15000;
constexpr int clock_sync_attempts = 3;

enum Alignment {
  LEFT_ALIGN,
  CENTER_ALIGN,
  RIGHT_ALIGN
};

#endif