#ifndef UTILS
#define UTILS

#include <stdint.h>
#include <WiFi.h>
#include <Arduino.h>
#include "constants.hpp"
#include "secrets.h"
#include "esp_sntp.h"
#include <Adafruit_NeoPixel.h>

struct FireworksRing {
  uint8_t start;
  uint8_t length;
  uint8_t offset;
  bool isCCW;
};
enum Rounding {
  Round,
  Ceil,
  Floor,
};

constexpr FireworksRing fireworksRingInfo[] = {
  {0,1,0, false},
  {1,6,0, false},
  {7,16,11, true},
  {23,24,12, false}
};

// returns whether the clock holds a valid time; a failed sync keeps the previous time
bool syncClockTime() {
  struct tm t;

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < wifi_timeout_ms) delay(100);

  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("wifi connected");

    sntp_set_sync_status(SNTP_SYNC_STATUS_RESET);
    configTzTime("EST5EDT,M3.2.0,M11.1.0", "pool.ntp.org");  // POSIX TZ string handles DST

    start = millis();
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED && millis() - start < wifi_timeout_ms) delay(100);
    sntp_stop();
  }
  else {
    Serial.println("wifi connect timed out");
  }

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);   // WiFi is by far the biggest power draw, so turn it off

  return getLocalTime(&t, 0);
}

bool isActiveTime(const struct tm& t, int activeHour) {
  int now = t.tm_hour * 60 + t.tm_min;
  int start = activeHour * 60;
  int end = (start + active_duration_minutes) % (24 * 60);

  if(start <= end) {
    return now >= start && now < end;
  }
  return now >= start || now < end;
}

void getTimeFromTimeStruct(struct tm* t, uint8_t* h, uint8_t* m) {
  *h = t->tm_hour % 12;
  if(*h == 0) *h = 12;
  
  *m = t->tm_min;
}

bool getDisplayTime(uint8_t* h, uint8_t* m) {
  struct tm t;
  bool hasTime = getLocalTime(&t);
  if (!hasTime) return false;
  
  getTimeFromTimeStruct(&t, h, m);
  return true;
}

void setPixelXY(Adafruit_NeoPixel& strip, uint8_t x, uint8_t y, uint32_t color) {
  if(x >= lights_per_row || x < 0) {
    return;
  }
  else if(y >= num_rows || y < 0) {
    return;
  }

  // y counts down from the top row; physical rows count up from the bottom
  int row = num_rows - 1 - y;
  int index = lights_per_row * row;
  if(row%2==0) {
    index += x;
  }
  else {
    index += (lights_per_row - x - 1);
  } 

  strip.setPixelColor(index, color);
}

int getFireworkRingEnd(FireworksRing ring) {
  return ring.start + ring.length;
}

const FireworksRing* getFireworkRing(int index, bool wrap = false) {
  if(wrap) index = index % total_firework_lights;

  for(int i = 0; i < 4; i++) {
    if(index < getFireworkRingEnd(fireworksRingInfo[i])) {
      return &fireworksRingInfo[i];
    }
  }
  return nullptr;
}

int getPhysicalFireworkPixelForVirtualIndex(int index) {
  const auto& currentRing = *getFireworkRing(index);
  int i = index - currentRing.start;

  if(currentRing.isCCW) {
    i = currentRing.length - 1 - i;
  }

  return ((i + currentRing.offset) % currentRing.length) + currentRing.start;
}

int getNthPixelFromIndexOnSameRing(int index, int n) {
  const auto& ring = *getFireworkRing(index);
  int i = index - ring.start;
  if(i+n > ring.length) {
    i = i % ring.length;
  }
  while(i+n < 0) {
    i += ring.length;
  }
  i += ring.start;
  return getPhysicalFireworkPixelForVirtualIndex(i);
}

int getNextFireworkPixel(int index) {
  if(index == total_firework_lights-1) return 0;
  return getPhysicalFireworkPixelForVirtualIndex(index+1);
}

int getFireworkPixelForAngle(int ringNum, int angle, Rounding r = Rounding::Round) {
  int px = 0;
  const auto ring = fireworksRingInfo[ringNum];
  float pct = angle / 360.f;
  float led = ring.length * pct;
  if(r == Rounding::Round) {
    px = round(led);
  }
  else if(r == Rounding::Floor) {
    px = floor(led);
  }
  else if(r == Rounding::Ceil) {
    px = ceil(led);
  }

  return getPhysicalFireworkPixelForVirtualIndex((px % ring.length) + ring.start);
}

#endif