#ifndef FIREWORKS
#define FIREWORKS

#include <Adafruit_NeoPixel.h>
#include "utils.hpp"

constexpr int firework_ring_count = sizeof(fireworksRingInfo) / sizeof(fireworksRingInfo[0]);

// each effect draws the whole firework strip for a moment of a celebration_ms run; the seed varies a run
typedef void (*FireworkEffect)(Adafruit_NeoPixel& strip, unsigned long elapsedMs, uint32_t seed);

void setFireworkPixel(Adafruit_NeoPixel& strip, int virtualIndex, uint32_t color) {
  if(virtualIndex < 0 || virtualIndex >= total_firework_lights) return;
  strip.setPixelColor(getPhysicalFireworkPixelForVirtualIndex(virtualIndex), color);
}

// a pixel's position with the rings one unit apart, so ring n sits at radius n
void fireworkPosition(int virtualIndex, float* x, float* y) {
  const auto& ring = *getFireworkRing(virtualIndex);
  int ringNum = &ring - fireworksRingInfo;
  float angle = 2 * PI * (virtualIndex - ring.start) / ring.length;
  *x = ringNum * cosf(angle);
  *y = ringNum * sinf(angle);
}

// the virtual index nearest a radius in ring spacings and an angle in radians
int fireworkIndexAt(float radius, float angle) {
  int ringNum = constrain(lroundf(radius), 0, firework_ring_count - 1);
  const auto& ring = fireworksRingInfo[ringNum];
  int i = lroundf(ring.length * angle / (2 * PI));
  return ring.start + ((i % ring.length) + ring.length) % ring.length;
}

// fills path with the pixels along one arm of a two-arm spiral: the center, then once around each ring
// from the center out, each lap starting at angle radians, with the second arm starting opposite
void spiralArm(int arm, float angle, int path[total_firework_lights]) {
  int length = 0;
  for(int n = 0; n < firework_ring_count; n++) {
    const auto& ring = fireworksRingInfo[n];
    int first = fireworkIndexAt(n, angle + arm * PI) - ring.start;
    for(int i = 0; i < ring.length; i++) {
      path[length++] = ring.start + (first + i) % ring.length;
    }
  }
}

// overlapping light keeps the brightest of each channel
void blendFireworkPixel(uint8_t rgb[total_firework_lights][3], int virtualIndex, uint32_t color) {
  uint8_t channels[3] = {(uint8_t)(color >> 16), (uint8_t)(color >> 8), (uint8_t)color};
  for(int c = 0; c < 3; c++) {
    rgb[virtualIndex][c] = max(rgb[virtualIndex][c], channels[c]);
  }
}

void showBlendedFireworks(Adafruit_NeoPixel& strip, uint8_t rgb[total_firework_lights][3]) {
  strip.clear();
  for(int i = 0; i < total_firework_lights; i++) {
    setFireworkPixel(strip, i, strip.Color(rgb[i][0], rgb[i][1], rgb[i][2]));
  }
}

// a pixel's brightness behind a head running along a path: full at the head, fading to nothing length
// pixels behind it; ramping up over the pixel ahead keeps the head gliding instead of jumping
float tailLevel(float behind, float length) {
  if(behind <= -1 || behind >= length) return 0;
  if(behind < 0) return behind + 1;
  return 1 - behind / length;
}

uint32_t mixBits(uint32_t x) {
  x ^= x >> 16;
  x *= 0x7feb352d;
  x ^= x >> 15;
  x *= 0x846ca68b;
  x ^= x >> 16;
  return x;
}

// the rainbow flows outward one ring every ripple_ring_ms, with each ring's hue sliding continuously
// between steps; the front fades in ring by ring at the start, and the center goes dark early so the
// tail fades out ring by ring as the run ends
void rainbowRippleEffect(Adafruit_NeoPixel& strip, unsigned long elapsedMs, uint32_t seed) {
  float emissions = (float)celebration_ms / ripple_ring_ms - firework_ring_count;
  float step = (float)elapsedMs / ripple_ring_ms;

  strip.clear();
  for(int r = 0; r < firework_ring_count; r++) {
    float emission = step - r;
    float level = min(min(emission + 1, emissions - emission), 1.f);
    if(level <= 0) continue;

    uint16_t hue = seed + (int32_t)(emission * ripple_hue_step);
    uint32_t color = strip.gamma32(strip.ColorHSV(hue, 255, 255 * level));
    const auto& ring = fireworksRingInfo[r];
    for(int i = ring.start; i < getFireworkRingEnd(ring); i++) {
      setFireworkPixel(strip, i, color);
    }
  }
}

// a random ring by burst_ring_weights, then a random pixel on it
int randomBurstCenter(uint32_t h) {
  int total = 0;
  for(uint8_t w : burst_ring_weights) total += w;

  int pick = h % total;
  int n = 0;
  while(pick >= burst_ring_weights[n]) {
    pick -= burst_ring_weights[n];
    n++;
  }
  const auto& ring = fireworksRingInfo[n];
  return ring.start + (h / total) % ring.length;
}

// bursts launch about every burst_interval_ms at random spots, so two or three overlap; each spreads out
// from its dot, lighting pixels as it reaches them, with the hue shifting the farther a pixel is from the dot
void fireworksEffect(Adafruit_NeoPixel& strip, unsigned long elapsedMs, uint32_t seed) {
  float px[total_firework_lights], py[total_firework_lights];
  for(int i = 0; i < total_firework_lights; i++) {
    fireworkPosition(i, &px[i], &py[i]);
  }

  uint8_t rgb[total_firework_lights][3] = {};

  for(int k = 0; k * burst_interval_ms < celebration_ms; k++) {
    uint32_t h = mixBits(seed + k);
    unsigned long start = k * burst_interval_ms + h % burst_jitter_ms;
    if(start + burst_ms > celebration_ms) continue;
    if(elapsedMs < start || elapsedMs >= start + burst_ms) continue;
    float age = elapsedMs - start;

    int center = randomBurstCenter(mixBits(h + 1));
    uint16_t baseHue = mixBits(h);

    for(int i = 0; i < total_firework_lights; i++) {
      float dist = hypotf(px[i] - px[center], py[i] - py[center]);
      if(dist > burst_radius) continue;

      float reached = dist * burst_step_ms;
      if(age < reached) continue;
      float level = 1 - (age - reached) / burst_fade_ms;
      if(level <= 0) continue;

      uint16_t hue = baseHue + (int32_t)(dist * burst_hue_step);
      blendFireworkPixel(rgb, i, strip.gamma32(strip.ColorHSV(hue, 255, 255 * level)));
    }
  }

  showBlendedFireworks(strip, rgb);
}

// both spiral arms light from the center out to half brightness with a rainbow along the arm, then
// brightening sweeps run out both arms over and over, lifting the pixels they pass toward full
void spiralFillEffect(Adafruit_NeoPixel& strip, unsigned long elapsedMs, uint32_t seed) {
  float angle = 2 * PI * (seed % 360) / 360;
  float levels[total_firework_lights] = {};
  float boosts[total_firework_lights] = {};
  uint16_t hues[total_firework_lights];

  // a new sweep starts at the center as the previous one reaches the end
  float sweepStep = elapsedMs > shape_fill_ms ? (float)(elapsedMs - shape_fill_ms) / spiral_sweep_step_ms : -1;
  int sweep = sweepStep / total_firework_lights;

  for(int arm = 0; arm < 2; arm++) {
    int path[total_firework_lights];
    spiralArm(arm, angle, path);
    for(int k = 0; k < total_firework_lights; k++) {
      float revealMs = (float)k * (shape_fill_ms - shape_reveal_ms) / (total_firework_lights - 1);
      float age = elapsedMs - revealMs;
      if(age > 0) {
        levels[path[k]] += 0.25f * min(1.f, age / shape_reveal_ms);
      }
      // the first arm passes every pixel, so its rainbow colors the whole spiral
      if(arm == 0) hues[path[k]] = seed + k * 65536 / total_firework_lights;

      if(sweepStep < 0) continue;
      for(int run = max(sweep - 1, 0); run <= sweep; run++) {
        float behind = sweepStep - run * total_firework_lights - k;
        boosts[path[k]] = max(boosts[path[k]], tailLevel(behind, spiral_sweep_length));
      }
    }
  }

  float fade = min(1.f, (float)(celebration_ms - elapsedMs) / shape_fade_ms);
  strip.clear();
  for(int i = 0; i < total_firework_lights; i++) {
    float level = (levels[i] + 0.5f * boosts[i]) * fade;
    setFireworkPixel(strip, i, strip.gamma32(strip.ColorHSV(hues[i], 255, 255 * level)));
  }
}

// a head runs out each spiral arm trailing a fading tail, over and over from the center; heads stop
// starting in time for the last one to run off the end as the run ends
void spiralTailEffect(Adafruit_NeoPixel& strip, unsigned long elapsedMs, uint32_t seed) {
  uint8_t rgb[total_firework_lights][3] = {};
  float angle = 2 * PI * (seed % 360) / 360;
  float step = (float)elapsedMs / spiral_tail_step_ms;
  int runs = ((float)celebration_ms / spiral_tail_step_ms - spiral_tail_length) / total_firework_lights;

  for(int arm = 0; arm < 2; arm++) {
    int path[total_firework_lights];
    spiralArm(arm, angle, path);

    for(int run = 0; run < runs; run++) {
      float head = step - run * total_firework_lights;
      for(int k = 0; k < total_firework_lights; k++) {
        float level = tailLevel(head - k, spiral_tail_length);
        if(level <= 0) continue;

        uint16_t hue = seed + k * 65536 / total_firework_lights;
        blendFireworkPixel(rgb, path[k], strip.gamma32(strip.ColorHSV(hue, 255, 255 * level)));
      }
    }
  }

  showBlendedFireworks(strip, rgb);
}

// each ring holds a full rainbow at half brightness that turns, the first ring clockwise and each ring out
// turning the other way, while full brightness ripples out from the center; the center pixel cycles through
// the rainbow
void rotatingRingsEffect(Adafruit_NeoPixel& strip, unsigned long elapsedMs, uint32_t seed) {
  float fade = min(1.f, (float)min(elapsedMs, celebration_ms - elapsedMs) / ring_fade_ms);
  float turns = (float)elapsedMs / ring_turn_ms;

  // the ripple's crest moves from inside the center to past the edge, then starts over
  float rippleRings = firework_ring_count - 1 + 2 * ring_ripple_half_width;
  float crest = fmodf((float)elapsedMs / ring_ripple_ms, rippleRings) - ring_ripple_half_width;

  // gamma32 raises brightness to about the 2.6 power, so the lift blends in that space; blending the value
  // directly leaves two neighboring rings dim while the crest sits between them, and the crest looks to hop
  constexpr float gamma = 2.6f;
  float basePerceived = powf(0.5f, gamma);

  strip.clear();
  for(int n = 0; n < firework_ring_count; n++) {
    const auto& ring = fireworksRingInfo[n];
    float distance = fabsf(n - crest);
    float lift = distance < ring_ripple_half_width ? 0.5f + 0.5f * cosf(PI * distance / ring_ripple_half_width) : 0;
    float level = powf(basePerceived + (1 - basePerceived) * lift, 1 / gamma) * fade;
    // virtual indexes count counterclockwise, so moving the rainbow toward lower indexes turns it clockwise
    float direction = n % 2 == 1 ? -1 : 1;
    for(int i = 0; i < ring.length; i++) {
      uint16_t hue = seed + (int32_t)(((float)i / ring.length - direction * turns) * 65536);
      setFireworkPixel(strip, ring.start + i, strip.gamma32(strip.ColorHSV(hue, 255, 255 * level)));
    }
  }
}

constexpr FireworkEffect fireworkEffects[] = {
  rainbowRippleEffect,
  fireworksEffect,
  spiralTailEffect,
  spiralFillEffect,
  rotatingRingsEffect,
};
constexpr int firework_effect_count = sizeof(fireworkEffects) / sizeof(fireworkEffects[0]);

#endif
