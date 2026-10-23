#ifndef OIL_WARMUP_BAR_H
#define OIL_WARMUP_BAR_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

extern Adafruit_ST7789 tft;

// Geometry configuration (anchored inside bottomMidUI: X=92, Y=105, W=130, H=55)
const int WARMUP_BOX_X = 92;
const int WARMUP_BOX_Y = 105;
const int WARMUP_BOX_W = 130;
const int WARMUP_BOX_H = 55;

// Centered bold progress bar (18px tall, 110px wide, vertically centered)
const int WARMUP_BAR_W = 110;
const int WARMUP_BAR_H = 18;
const int WARMUP_BAR_X = WARMUP_BOX_X + (WARMUP_BOX_W - WARMUP_BAR_W) / 2; // 102
const int WARMUP_BAR_Y = WARMUP_BOX_Y + (WARMUP_BOX_H - WARMUP_BAR_H) / 2; // 123

// Temperature thresholds
const float TEMP_COLD_MAX   = 30.0; // Below 30°C: RED (Cold)
const float TEMP_WARM_MAX   = 60.0; // 30°C - 60°C: YELLOW/AMBER (Warming up)
const float TEMP_FULL_READY = 75.0; // 75°C: Progress bar reaches 100% full
const float TEMP_OVERHEAT   = 125.0; // 125°C+: Red Overheat warning

// Charging threshold: alternator active voltage is typically > 13.2V
const float CHARGING_THRESHOLD = 13.0f;

enum WarmupState {
  STATE_UNKNOWN,
  STATE_COLD,
  STATE_WARMING,
  STATE_READY,
  STATE_HOT,
  STATE_DISCONNECTED
};

inline void drawOilWarmupBar(float currentTemp, bool isDisconnected, float batteryVoltage) {
  static float lastDrawnTemp = -999.0;
  static WarmupState currentState = STATE_UNKNOWN;
  static WarmupState lastDrawnState = STATE_UNKNOWN;
  static bool firstClearDone = false;
  static bool wasActive = false;

  // --- ENGINE NOT RUNNING / NOT CHARGING CHECK ---
  // If the engine is off (voltage resting below ~13.0V), hide/clear the warmup bar box
  if (batteryVoltage < CHARGING_THRESHOLD) {
    if (wasActive || !firstClearDone) {
      tft.fillRect(WARMUP_BOX_X, WARMUP_BOX_Y, WARMUP_BOX_W, WARMUP_BOX_H, ST77XX_BLACK);
      wasActive = false;
      firstClearDone = true;
    }
    return; // Exit early, keeping the box black and hidden
  }

  wasActive = true;

  // Clear widget area once on first run when active
  if (!firstClearDone) {
    tft.fillRect(WARMUP_BOX_X, WARMUP_BOX_Y, WARMUP_BOX_W, WARMUP_BOX_H, ST77XX_BLACK);
    firstClearDone = true;
  }

  // Determine State with Hysteresis (prevents rapid color flickering at boundary temps)
  if (isDisconnected || currentTemp <= -900.0) {
    currentState = STATE_DISCONNECTED;
  } else {
    switch (currentState) {
      case STATE_DISCONNECTED:
      case STATE_UNKNOWN:
        if (currentTemp < TEMP_COLD_MAX) currentState = STATE_COLD;
        else if (currentTemp < TEMP_WARM_MAX) currentState = STATE_WARMING;
        else if (currentTemp < TEMP_OVERHEAT) currentState = STATE_READY;
        else currentState = STATE_HOT;
        break;

      case STATE_COLD:
        if (currentTemp >= TEMP_COLD_MAX) currentState = STATE_WARMING;
        break;

      case STATE_WARMING:
        if (currentTemp < (TEMP_COLD_MAX - 1.5)) currentState = STATE_COLD;
        else if (currentTemp >= TEMP_WARM_MAX) currentState = STATE_READY;
        break;

      case STATE_READY:
        if (currentTemp < (TEMP_WARM_MAX - 1.5)) currentState = STATE_WARMING;
        else if (currentTemp >= TEMP_OVERHEAT) currentState = STATE_HOT;
        break;

      case STATE_HOT:
        if (currentTemp < (TEMP_OVERHEAT - 2.0)) currentState = STATE_READY;
        break;
    }
  }

  // Set colors based on stable state
  uint16_t barColor;
  switch (currentState) {
    case STATE_COLD:
      barColor = ST77XX_RED;
      break;
    case STATE_WARMING:
      barColor = 0xbd83; // Warm amber
      break;
    case STATE_READY:
      barColor = 0x0400; // OEM green
      break;
    case STATE_HOT:
      barColor = 0x9041; // Dark red warning
      break;
    case STATE_DISCONNECTED:
    default:
      barColor = 0x4208; // Dim gray
      break;
  }

  // Redraw only when temp changes by >= 1°C or when state flips
  if (abs(currentTemp - lastDrawnTemp) < 1.0 && currentState == lastDrawnState) {
    return;
  }
  lastDrawnTemp = currentTemp;
  lastDrawnState = currentState;

  // Draw Bar Outline with rounded corners
  tft.drawRoundRect(WARMUP_BAR_X, WARMUP_BAR_Y, WARMUP_BAR_W, WARMUP_BAR_H, 4, barColor);

  // Calculate Bar Fill (maps 20°C -> 75°C to 0% -> 100%)
  int innerW = WARMUP_BAR_W - 4;
  int innerH = WARMUP_BAR_H - 4;
  int fillW = 0;

  if (!isDisconnected && currentTemp > -900.0) {
    float progress = (currentTemp - 20.0) / (TEMP_FULL_READY - 20.0);
    if (progress < 0.0) progress = 0.0;
    if (progress > 1.0) progress = 1.0;
    fillW = (int)(progress * innerW);
  }

  // Draw active filled portion
  if (fillW > 0) {
    tft.fillRect(WARMUP_BAR_X + 2, WARMUP_BAR_Y + 2, fillW, innerH, barColor);
  }
  // Clear unfilled portion to pure black
  if (innerW - fillW > 0) {
    tft.fillRect(WARMUP_BAR_X + 2 + fillW, WARMUP_BAR_Y + 2, innerW - fillW, innerH, ST77XX_BLACK);
  }
}

#endif
