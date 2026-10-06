#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define DEBOUNCE_MS 30
#define NUM_PINS 15

void onPress();
void setLights();
Adafruit_NeoPixel numStrip(NUM_PINS, 25, NEO_GRB + NEO_KHZ800);

bool lastStable = LOW;
bool lastReading = LOW;
unsigned int lastPin = 0;
unsigned long lastChange = 0;


void setup() {
  // put your setup code here, to run once:
  pinMode(2, OUTPUT);
  pinMode(25, OUTPUT);
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

// put function definitions here:
void onPress() {
  Serial.println("pushed");
  setLights();
  lastPin++;
}

void setLights() {
  for(int i = 0; i < NUM_PINS; i += NUM_PINS/3) {
    int poff = (lastPin + i) % NUM_PINS;
    int pon = (poff + 1) % NUM_PINS;
    numStrip.setPixelColor(poff, 0);
    numStrip.setPixelColor(pon, numStrip.Color(random(256), random(256), random(256)));
  }
  numStrip.show();
}