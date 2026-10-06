#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include "constants.hpp"
#include "display_setters.hpp"

/** EXAMPLE **/
void onPress();
void setLights();

Adafruit_NeoPixel numStrip(LIGHTS_PER_ROW * NUM_ROWS, LIGHT_PIN, NEO_GRB + NEO_KHZ800);

bool lastStable = LOW;
bool lastReading = LOW;
unsigned int lastPin = 0;
unsigned long lastChange = 0;


void setup() {
  // put your setup code here, to run once:
  pinMode(2, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(27, INPUT_PULLDOWN);
  Serial.begin(115200);

  numStrip.begin();
  numStrip.setBrightness(50);
  numStrip.clear();

  setLights();
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t in = digitalRead(27);

  if(in != lastReading) {
    lastChange = millis();
    lastReading = in;
  }

  if(millis() - lastChange > DEBOUNCE_MS && in != lastStable) {
    lastStable = in;
    if(lastStable == HIGH) {
      onPress();
    }
  }
  digitalWrite(2, lastStable);
}

/*** EXAMPLE CODE ***/
void onPress() {
  Serial.println("pushed");
  setLights();
  lastPin++;
}

void setLights() {
  for(int i = 0; i < LIGHTS_PER_ROW; i += LIGHTS_PER_ROW/3) {
    int poff = (lastPin + i) % LIGHTS_PER_ROW;
    int pon = (poff + 1) % LIGHTS_PER_ROW;
    numStrip.setPixelColor(poff, 0);
    numStrip.setPixelColor(pon, numStrip.Color(random(256), random(256), random(256)));
  }
  numStrip.show();
}
