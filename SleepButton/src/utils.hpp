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

// returns whether the clock synced; a failed sync keeps the previous time
bool syncClockTime() {
  bool synced = false;

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < wifi_timeout_ms) delay(100);

  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("wifi connected");

    sntp_set_sync_status(SNTP_SYNC_STATUS_RESET);
    configTzTime("EST5EDT,M3.2.0,M11.1.0", "pool.ntp.org");  // POSIX TZ string handles DST

    start = millis();
    // the status reads COMPLETED once and then resets, so it is captured on the read that sees it
    while (!synced && millis() - start < wifi_timeout_ms) {
      synced = sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED;
      if(!synced) delay(100);
    }
    sntp_stop();

    if(!synced) {
      Serial.println("ntp sync timed out");
    }
  }
  else {
    Serial.println("wifi connect timed out");
  }

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);   // WiFi is by far the biggest power draw, so turn it off

  return synced;
}

bool syncClockWithRetries() {
  for(int i = 0; i < clock_sync_attempts; i++) {
    if(syncClockTime()) return true;
  }
  return false;
}

// identifies the day the current active window started on, as year * 1000 + day of year
int32_t activeWindowDay(const struct tm& t, int activeHour) {
  struct tm start = t;
  start.tm_hour = activeHour;
  start.tm_min = 0;
  start.tm_sec = 0;
  start.tm_isdst = -1;
  if(t.tm_hour < activeHour) {
    // the window crosses midnight and started the previous day
    start.tm_mday -= 1;
  }
  mktime(&start);

  return (start.tm_year + 1900) * 1000 + start.tm_yday;
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

// returns the strip index for an x/y position, or -1 when it is off the display
int getPixelIndexXY(int x, int y) {
  if(x >= lights_per_row || x < 0) {
    return -1;
  }
  else if(y >= num_rows || y < 0) {
    return -1;
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

  return index;
}

void setPixelXY(Adafruit_NeoPixel& strip, int x, int y, uint32_t color) {
  int index = getPixelIndexXY(x, y);
  if(index < 0) return;

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