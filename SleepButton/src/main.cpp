#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <Preferences.h>
#include "constants.hpp"
#include "display_setters.hpp"
#include "fireworks.hpp"

/** EXAMPLE **/
void onPress();
void onRelease();
void showPushCount();
void enterSettingMode();
void exitSettingMode();
void settingLoop();
void setLights();
void handleButton(int button);
void ledCircleCycle(void* param);
void ledScanCycle(void* param);
void debugLoop();
void onDebugRelease();
void toggleFireworksMode();
void playNextFireworkEffect();
void startFireworkEffect(int effect);
void stopFireworkEffect();
void drawFireworkEffect();
bool dayCounted(const struct tm& t);
uint32_t flashedRingColor(uint32_t color);
void syncClockIfDue();
float activePulse();
void scrollToward(float target);
float scrollOffset();
void drawPushCount(int scrollRows);
void setOuterRing(uint32_t color);

Adafruit_NeoPixel numStrip(total_lights, LIGHT_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel fireworkStrip(total_firework_lights, FIREWORK_PIN, NEO_GRB + NEO_KHZ800);

bool lastStable = LOW;
bool lastReading = LOW;
unsigned int lastPin = 0;
unsigned long lastChange = 0;

bool clockEnabled = false;
unsigned long lastSyncAttempt = 0;
unsigned long showClockTime = 0;
int pushCount = 0;
int shownCount = 0;
unsigned long pressStart = 0;

// the push count sits one blank row above the clock and both scroll down together
constexpr int count_scroll_rows = glyph_height + 1;
float scrollFrom = 0;
float scrollTo = 0;
unsigned long scrollStart = 0;

// the count goes up only on the first press of each day's active window; that press lands on the
// previous count, then reveals the new one in white with a sloshing rainbow wave
bool revealCount = false;
int countBeforePress = 0;
int32_t lastCountedDay = 0;

Preferences prefs;
uint8_t activeHour = default_active_hour;

bool settingMode = false;
uint8_t savedActiveHour = default_active_hour;
unsigned long lastSettingInteraction = 0;
unsigned long lastSettingTap = 0;

bool debugMode = false;

// the counting press plays a random firework effect; debug fireworks mode, which debug starts in and a long
// press toggles, shows only the snake on the matrix and plays the next effect on each press
bool fireworksMode = true;
bool longPressHandled = false;
int nextFireworkEffect = 0;
int playingFireworkEffect = -1;
unsigned long fireworkEffectStart = 0;
uint32_t fireworkEffectSeed = 0;

// a press that does not count flashes the outer ring red until the day is counted, green after
bool ringFlashing = false;
unsigned long ringFlashStart = 0;
uint32_t ringFlashColor = 0;

// the color setOuterRing last sent; ring_color_unknown makes the next call send whatever it is given
constexpr uint32_t ring_color_unknown = 0xFFFFFFFF;
uint32_t shownRingColor = 0;

// firework debug task
TaskHandle_t ledTask;

bool buttonSettled(int button) {
  return digitalRead(button) == lastStable
      && millis() - lastChange > DEBOUNCE_MS;
}

void setup() {
  Serial.begin(115200);
  
  pinMode(2, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(FIREWORK_PIN, OUTPUT);
  pinMode(27, INPUT_PULLDOWN);
  pinMode(DEBUG_PIN, INPUT_PULLUP);
  Serial.print("debug pin: ");
  Serial.println(digitalRead(DEBUG_PIN));

  debugMode = digitalRead(DEBUG_PIN) == LOW;

  prefs.begin("sleepbutton", false);
  activeHour = prefs.getUChar("active_hour", default_active_hour);
  pushCount = prefs.getInt("push_count", 0);
  lastCountedDay = prefs.getInt("counted_day", 0);

  numStrip.begin();
  numStrip.setBrightness(50);
  numStrip.clear();

  if(!debugMode) {
    setDisplayAsClockPlaceholder(numStrip, numStrip.Color(0, 0, clock_digit_level), numStrip.Color(0, 0, clock_colon_level));
    numStrip.show();

    clockEnabled = syncClockWithRetries();
    lastSyncAttempt = millis();
  }

  fireworkStrip.begin();
  fireworkStrip.setBrightness(50);
  fireworkStrip.clear();

  fireworkStrip.show();

  if(debugMode) {
    return;
  }

  setLights();
}

void loop() {
  // button press
  handleButton(INPUT_BUTTON_PIN);
  if(debugMode) {
    debugLoop();
    return;
  }
  if(!buttonSettled(INPUT_BUTTON_PIN)) return;

  if(!settingMode && lastStable == HIGH && millis() - pressStart >= long_press_ms) {
    enterSettingMode();
  }
  if(settingMode) {
    setOuterRing(0);
    settingLoop();
    delay(20);
    return;
  }

  syncClockIfDue();

  scrollToward(millis() < showClockTime ? count_scroll_rows : 0);
  int scrollRows = lroundf(scrollOffset());

  numStrip.clear();
  uint32_t digitColor = numStrip.Color(0, 0, clock_digit_level);
  uint32_t colonColor = numStrip.Color(0, 0, clock_colon_level);

  uint32_t ringColor = 0;

  struct tm t;
  if(clockEnabled && getLocalTime(&t)) {
    if(isActiveTime(t, activeHour)) {
      // the clock and outer ring pulse through the window: red until the day's press, green after
      float pulse = activePulse();
      if(dayCounted(t)) {
        digitColor = numStrip.Color(0, clock_digit_level * pulse, 0);
        colonColor = numStrip.Color(0, clock_colon_level * pulse, 0);
      }
      else {
        digitColor = numStrip.Color(clock_digit_level * pulse, 0, 0);
        colonColor = numStrip.Color(clock_colon_level * pulse, 0, 0);
      }
      ringColor = digitColor;
    }
    else if(dayCounted(t)) {
      // green from the day's press until midnight, blue otherwise
      digitColor = numStrip.Color(0, clock_digit_level, 0);
      colonColor = numStrip.Color(0, clock_colon_level, 0);
    }

    setDisplayAsClockTime(&t, numStrip, digitColor, colonColor, scrollRows);
  }
  else {
    setDisplayAsClockPlaceholder(numStrip, digitColor, colonColor, scrollRows);
  }
  drawPushCount(scrollRows);
  numStrip.show();

  if(playingFireworkEffect >= 0) {
    drawFireworkEffect();
  }
  else {
    // the outer firework ring pulses along with the clock and is dark otherwise
    setOuterRing(flashedRingColor(ringColor));
  }

  delay(20);
}

void syncClockIfDue() {
  if(millis() - lastSyncAttempt < clock_sync_interval_ms) return;

  struct tm t;
  if(getLocalTime(&t, 0) && isActiveTime(t, activeHour)) return;

  lastSyncAttempt = millis();
  if(syncClockWithRetries()) {
    clockEnabled = true;
  }
}

// whether the day's press has happened: through the active window that is the window's day, and after it
// the calendar day, so the day stays counted until midnight
bool dayCounted(const struct tm& t) {
  int32_t day = isActiveTime(t, activeHour) ? activeWindowDay(t, activeHour) : calendarDay(t);
  return day == lastCountedDay;
}

// blends a ring flash, ramping up to ringFlashColor and back, over the ring's own color
uint32_t flashedRingColor(uint32_t color) {
  unsigned long sinceFlash = millis() - ringFlashStart;
  if(!ringFlashing || sinceFlash >= ring_flash_ms) {
    ringFlashing = false;
    return color;
  }

  float flash = 1 - fabsf(2.f * sinceFlash / ring_flash_ms - 1);
  uint8_t blended[3];
  for(int c = 0; c < 3; c++) {
    int shift = 16 - 8 * c;
    float from = (color >> shift) & 0xFF;
    float to = (ringFlashColor >> shift) & 0xFF;
    blended[c] = from + (to - from) * flash;
  }
  return fireworkStrip.Color(blended[0], blended[1], blended[2]);
}

// starts a count_scroll_ms scroll from the current offset; a reversal mid-scroll starts from where it is
void scrollToward(float target) {
  if(target == scrollTo) return;

  scrollFrom = scrollOffset();
  scrollTo = target;
  scrollStart = millis();
}

// halfway between linear and a quadratic ease-out: 1.5t - 0.5t^2
float scrollOffset() {
  float t = min(1.f, (float)(millis() - scrollStart) / count_scroll_ms);
  float eased = 1.5f * t - 0.5f * t * t;
  return scrollFrom + (scrollTo - scrollFrom) * eased;
}

// brightness factor that breathes between 0.7 and 1.0 every 3 seconds
float activePulse() {
  return 0.85f + 0.15f * sinf(2 * PI * (millis() % 3000) / 3000.f);
}

void handleButton(int button) {
  uint8_t in = digitalRead(button);

  if(in != lastReading) {
    lastChange = millis();
    lastReading = in;
  }

  if(millis() - lastChange > DEBOUNCE_MS && in != lastStable) {
    lastStable = in;
    if(debugMode) {
      // center firework LED lights while the button is held
      setFireworkPixel(fireworkStrip, 0, lastStable == HIGH ? fireworkStrip.Color(255, 255, 255) : 0);
      fireworkStrip.show();
      if(lastStable == HIGH) {
        pressStart = millis();
      }
      else {
        onDebugRelease();
      }
    }
    else if(lastStable == HIGH) {
      onPress();
    }
    else {
      onRelease();
    }
  }
  digitalWrite(ONBOARD_LED_PIN, lastStable);
}

void onPress() {
  pressStart = millis();

  if(settingMode) {
    activeHour = (activeHour + 1) % 24;
    lastSettingTap = millis();
  }
}

// a long press enters setting mode before release, so only short presses reach showPushCount
void onRelease() {
  if(!settingMode) {
    showPushCount();
  }
}

void showPushCount() {
  Serial.println("pushed");

  // a press that starts a scroll in decides whether this showing counts; presses while it shows only restart the timer
  if(scrollTo == 0 && debugMode) {
    // debug previews the counting press every time with a temporary count that is never stored
    revealCount = true;
    countBeforePress = pushCount;
    shownCount = pushCount + 1;
  }
  else if(scrollTo == 0) {
    revealCount = false;

    struct tm t;
    if(getLocalTime(&t, 0) && isActiveTime(t, activeHour)) {
      int32_t day = activeWindowDay(t, activeHour);
      if(day != lastCountedDay) {
        revealCount = true;
        countBeforePress = pushCount;
        pushCount++;
        lastCountedDay = day;

        prefs.putInt("push_count", pushCount);
        prefs.putInt("counted_day", lastCountedDay);

        startFireworkEffect(random(firework_effect_count));
      }
    }
    shownCount = pushCount;
  }

  // debug previews only the count; otherwise a press that does not count flashes the outer ring, unless a
  // celebration is playing on the rings, including the one the counting press just started
  if(!debugMode && playingFireworkEffect < 0) {
    struct tm t;
    bool met = getLocalTime(&t, 0) && dayCounted(t);
    ringFlashing = true;
    ringFlashStart = millis();
    ringFlashColor = met ? fireworkStrip.Color(0, 255, 0) : fireworkStrip.Color(255, 0, 0);
  }

  // the display timer starts once the count has scrolled in; presses while it shows restart it
  scrollToward(count_scroll_rows);
  showClockTime = max(millis(), scrollStart + count_scroll_ms) + count_display_ms;
}

// draws the push count above the clock at the current scroll
void drawPushCount(int scrollRows) {
  if(scrollRows <= 0) return;

  int countOffset = scrollRows - count_scroll_rows;
  // the old count shows while scrolling in and for count_reveal_delay_ms after landing
  bool beforeReveal = scrollTo == count_scroll_rows && millis() - scrollStart < count_scroll_ms + count_reveal_delay_ms;
  bool showOld = revealCount && beforeReveal;

  if(!revealCount || showOld) {
    setDisplayAsNumber(showOld ? countBeforePress : shownCount, numStrip, numStrip.Color(clock_digit_level, 0, 0), countOffset);
    return;
  }

  setDisplayAsNumber(shownCount, numStrip, numStrip.Color(clock_digit_level, clock_digit_level, clock_digit_level), countOffset);

  // the wave sloshes until the last wave_settle_ms before the count scrolls away, including when presses extend it
  unsigned long sinceReveal = millis() - (scrollStart + count_scroll_ms + count_reveal_delay_ms);
  if(scrollTo == count_scroll_rows) {
    float envelope = min(1.f, (float)(showClockTime - millis()) / wave_settle_ms);
    setDisplayAsWave(shownCount, numStrip, sinceReveal, envelope, clock_digit_level, countOffset);
  }
}

void enterSettingMode() {
  settingMode = true;
  stopFireworkEffect();
  savedActiveHour = activeHour;
  lastSettingInteraction = millis();
  lastSettingTap = millis() - setting_flash_ms;
}

void exitSettingMode() {
  settingMode = false;
  showClockTime = 0;
  scrollFrom = 0;
  scrollTo = 0;
  if(activeHour != savedActiveHour) {
    prefs.putUChar("active_hour", activeHour);
  }
}

void settingLoop() {
  // the timeout counts from the last release, so holding the button keeps setting mode open
  if(lastStable == HIGH) {
    lastSettingInteraction = millis();
  }

  unsigned long idle = millis() - lastSettingInteraction;
  if(idle >= setting_timeout_ms) {
    exitSettingMode();
    return;
  }

  // each tap ramps the text to white and back over setting_flash_ms
  float flash = 0;
  unsigned long sinceTap = millis() - lastSettingTap;
  if(sinceTap < setting_flash_ms) {
    flash = 1 - fabsf(2.f * sinceTap / setting_flash_ms - 1);
  }

  uint32_t baseColor = numStrip.Color(setting_r, setting_g, setting_b);
  uint32_t textColor = numStrip.Color(
    setting_r + (255 - setting_r) * flash,
    setting_g + (255 - setting_g) * flash,
    setting_b + (255 - setting_b) * flash
  );

  uint8_t remaining = (setting_timeout_ms - idle + 999) / 1000;

  numStrip.clear();
  setDisplayAsHour(activeHour, numStrip, textColor);
  setDisplayAsCountdown(remaining, numStrip, baseColor);
  numStrip.show();
}

// the release that ends a long press only finishes the toggle
void onDebugRelease() {
  if(longPressHandled) {
    longPressHandled = false;
    return;
  }

  if(fireworksMode) {
    playNextFireworkEffect();
  }
  else {
    showPushCount();
  }
}

void toggleFireworksMode() {
  fireworksMode = !fireworksMode;

  // drop any count showing so it does not resume mid-scroll after fireworks mode
  showClockTime = 0;
  scrollFrom = 0;
  scrollTo = 0;

  stopFireworkEffect();
}

void playNextFireworkEffect() {
  startFireworkEffect(nextFireworkEffect);
  nextFireworkEffect = (nextFireworkEffect + 1) % firework_effect_count;
}

void startFireworkEffect(int effect) {
  playingFireworkEffect = effect;
  fireworkEffectStart = millis();
  fireworkEffectSeed = random(1L << 30);
}

// leaves the rings dark, and has the outer ring redrawn on its next update
void stopFireworkEffect() {
  playingFireworkEffect = -1;
  fireworkStrip.clear();
  fireworkStrip.show();
  shownRingColor = ring_color_unknown;
}

// plays the current effect once, then stops it
void drawFireworkEffect() {
  if(playingFireworkEffect < 0) return;

  unsigned long elapsed = millis() - fireworkEffectStart;
  if(elapsed >= celebration_ms) {
    stopFireworkEffect();
    return;
  }
  fireworkEffects[playingFireworkEffect](fireworkStrip, elapsed, fireworkEffectSeed);
  fireworkStrip.show();
}

// debug mode treats every minute as inside the active window, with "0D:BG" in place of the time
void debugLoop() {
  if(lastStable == HIGH && !longPressHandled && millis() - pressStart >= long_press_ms) {
    longPressHandled = true;
    toggleFireworksMode();
  }

  scrollToward(millis() < showClockTime ? count_scroll_rows : 0);
  int scrollRows = lroundf(scrollOffset());

  // redraw everything each frame so the pixel the snake leaves is restored
  numStrip.clear();

  if(fireworksMode) {
    drawFireworkEffect();
  }
  else {
    float pulse = activePulse();
    setDisplayAsDebugClock(numStrip, numStrip.Color(clock_digit_level * pulse, 0, 0), numStrip.Color(clock_colon_level * pulse, 0, 0), scrollRows);
    drawPushCount(scrollRows);
  }

  // the snake steps 15 times a second through the whole strip
  int snakePos = (millis() / (1000 / 15)) % total_lights;
  uint16_t hue = (uint32_t)snakePos * 65536 / total_lights;
  numStrip.setPixelColor(snakePos, numStrip.gamma32(numStrip.ColorHSV(hue)));
  numStrip.show();

  delay(20);
}

void setLights() {
  for(int i = 0; i < total_lights; i += total_lights/3) {
    int poff = (lastPin + i) % total_lights;
    int pon = (poff + 1) % total_lights;
    numStrip.setPixelColor(poff, 0);
    numStrip.setPixelColor(pon, numStrip.Color(random(256), random(256), random(256)));
  }
  numStrip.show();
}

// only sends to the strip when the color changes, so a dark ring costs nothing per loop
void setOuterRing(uint32_t color) {
  if(color == shownRingColor) return;
  shownRingColor = color;

  const auto& ring = fireworksRingInfo[3];
  for(int i = ring.start; i < getFireworkRingEnd(ring); i++) {
    setFireworkPixel(fireworkStrip, i, color);
  }
  fireworkStrip.show();
}

void ledScanCycle(void *param) {
  constexpr int n = total_firework_lights;
  int angle = 0;

  uint32_t c = fireworkStrip.Color(random(256), random(256), random(256));

  fireworkStrip.setPixelColor(0, c);

  uint16_t kNone = UINT16_MAX;
  uint16_t lit[] = {kNone, kNone, kNone};

  while(true) {
    for(int i = 1; i < 4; i++) {
      auto px = getFireworkPixelForAngle(i, angle);
      if(px != lit[i-1]) {
        fireworkStrip.setPixelColor(lit[i-1], 0);
        lit[i-1] = px;
        fireworkStrip.setPixelColor(lit[i-1], c);
      }
    }

    angle += 360 / 24;
    if(angle >= 360) {
      angle -= 360;
      c = fireworkStrip.Color(random(256), random(256), random(256));
    }
    fireworkStrip.show();
    vTaskDelay(pdMS_TO_TICKS(1000 / 24));
  }
}

void ledCircleCycle(void *param) {
  constexpr int n = total_firework_lights;
  int i = 0;

  long head_r = random(256);
  long head_g = random(256);
  long head_b = random(256);
  uint32_t colors[4] = {fireworkStrip.Color(head_r, head_g, head_b), 0 ,0 ,0};
  int colorPropped = 3;

  while(true) {
    if(colorPropped > 0) {
      int i = 4-colorPropped;
      float factor = .75f;
      if(i == 2) factor = 0.32f;
      else if(i == 3) factor = 0.075f;

      colors[i] = fireworkStrip.Color(head_r * factor, head_g * factor, head_b * factor);
      colorPropped--;
    }
    else {
      if(i % 10 == 0) {
        head_r = random(256);
        head_g = random(256);
        head_b = random(256);
        colors[0] = fireworkStrip.Color(head_r, head_g, head_b);
        colorPropped = 3;
      }
    }

    // the tail wraps from the start of the strip back around to the end
    for(int j = 0; j < 4; j++) {
      setFireworkPixel(fireworkStrip, (i - j + n) % n, colors[j]);
    }
    setFireworkPixel(fireworkStrip, (i - 4 + n) % n, 0);
    fireworkStrip.show();

    i = (i+1) % n;
    
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
