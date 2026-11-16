#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <Preferences.h>
#include "constants.hpp"
#include "display_setters.hpp"

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
void setFireworkPixel(int virtualIndex, uint32_t color);
void syncClockIfDue();
float activePulse();

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
unsigned long pressStart = 0;

Preferences prefs;
uint8_t activeHour = default_active_hour;

bool settingMode = false;
uint8_t savedActiveHour = default_active_hour;
unsigned long lastSettingInteraction = 0;
unsigned long lastSettingTap = 0;

bool debugMode = false;

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
  if(!debugMode) {
    clockEnabled = syncClockTime();
    lastSyncAttempt = millis();
  }

  numStrip.begin();
  numStrip.setBrightness(50);
  numStrip.clear();

  fireworkStrip.begin();
  fireworkStrip.setBrightness(50);
  fireworkStrip.clear();

  if(debugMode) {
    fireworkStrip.show();
    return;
  }

  xTaskCreatePinnedToCore(
    ledScanCycle, "ledCycle", 4096, NULL, 2, &ledTask, 0
  );

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
    settingLoop();
    delay(20);
    return;
  }

  syncClockIfDue();

  bool showNumber = millis() < showClockTime;
  uint64_t wakeTime = 1000000ULL * 60;
  
  if(clockEnabled) {
    struct tm t;
    bool hasClockTime = getLocalTime(&t);
    if(!hasClockTime) {
      Serial.println("Trying to show time but failing");
      if(!showNumber) {
        // display -:--
      }
    }
    else {
      wakeTime = (60 - t.tm_sec) * 1000000ULL;
      Serial.print("showing time, sleep for");
      Serial.println(wakeTime);

      if(!showNumber) {
        uint32_t digitColor = numStrip.Color(0, 0, clock_digit_level);
        uint32_t colonColor = numStrip.Color(0, 0, clock_colon_level);
        if(isActiveTime(t, activeHour)) {
          float pulse = activePulse();
          digitColor = numStrip.Color(clock_digit_level * pulse, 0, 0);
          colonColor = numStrip.Color(clock_colon_level * pulse, 0, 0);
          // keep waking often enough to animate the pulse
          wakeTime = min(wakeTime, 20000ULL);
        }

        numStrip.clear();
        setDisplayAsClockTime(&t, numStrip, digitColor, colonColor);
        numStrip.show();
      }
    }
  }
  if(showNumber) {
    wakeTime = min((showClockTime - millis()) * 1000ULL, wakeTime);
  }

  esp_sleep_enable_timer_wakeup(wakeTime);
  gpio_wakeup_enable((gpio_num_t)INPUT_BUTTON_PIN, lastStable ? GPIO_INTR_LOW_LEVEL : GPIO_INTR_HIGH_LEVEL);
  esp_sleep_enable_gpio_wakeup();
  Serial.flush();
  //esp_light_sleep_start();
  delay(20);
}

void syncClockIfDue() {
  if(millis() - lastSyncAttempt < clock_sync_interval_ms) return;

  struct tm t;
  if(getLocalTime(&t, 0) && isActiveTime(t, activeHour)) return;

  lastSyncAttempt = millis();
  if(syncClockTime()) {
    clockEnabled = true;
  }
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
      setFireworkPixel(0, lastStable == HIGH ? fireworkStrip.Color(255, 255, 255) : 0);
      fireworkStrip.show();
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
  pushCount++;
  showClockTime = millis() + 4000;
  numStrip.clear();
  setDisplayAsNumber(pushCount, numStrip, numStrip.Color(0, 255, 0));
  numStrip.show();
}

void enterSettingMode() {
  settingMode = true;
  savedActiveHour = activeHour;
  lastSettingInteraction = millis();
  lastSettingTap = millis() - setting_flash_ms;
}

void exitSettingMode() {
  settingMode = false;
  showClockTime = 0;
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

void debugLoop() {
  static unsigned long lastStep = 0;
  static int snakePos = 0;

  if(millis() - lastStep < 1000 / 15) return;
  lastStep = millis();

  // redraw the text every step so the pixel the snake leaves is restored
  numStrip.clear();
  setDisplayAsDebug(numStrip, numStrip.Color(0, 255, 0));

  uint16_t hue = (uint32_t)snakePos * 65536 / total_lights;
  numStrip.setPixelColor(snakePos, numStrip.gamma32(numStrip.ColorHSV(hue)));
  numStrip.show();

  snakePos = (snakePos + 1) % total_lights;
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

void setFireworkPixel(int virtualIndex, uint32_t color) {
  if(virtualIndex < 0 || virtualIndex >= total_firework_lights) return;
  fireworkStrip.setPixelColor(getPhysicalFireworkPixelForVirtualIndex(virtualIndex), color);
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
      setFireworkPixel((i - j + n) % n, colors[j]);
    }
    setFireworkPixel((i - 4 + n) % n, 0);
    fireworkStrip.show();

    i = (i+1) % n;
    
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
