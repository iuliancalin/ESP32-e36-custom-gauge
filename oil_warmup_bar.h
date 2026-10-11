#ifndef OIL_WARMUP_BAR_H
#define OIL_WARMUP_BAR_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Fonts/FreeSansBold9pt7b.h>

extern Adafruit_ST7789 tft;

// Geometry configuration (anchored inside bottomMidUI: X=92, Y=105, W=130, H=55)
const int WARMUP_BOX_X = 92;
const int WARMUP_BOX_Y = 105;
const int WARMUP_BOX_W = 130;
const int WARMUP_BOX_H = 55;

const int WARMUP_BAR_X = 102; // Centered inside 130px box (10px margin)
const int WARMUP_BAR_Y = 136;
const int WARMUP_BAR_W = 110;
const int WARMUP_BAR_H = 12;

// Temperature thresholds
const float TEMP_COLD_MAX   = 30.0; // Below 30°C: RED (Cold)
const float TEMP_WARM_MAX   = 60.0; // 30°C - 60°C: YELLOW (Warming up)
const float TEMP_FULL_READY = 75.0; // 75°C: Progress bar reaches 100% full
const float TEMP_OVERHEAT   = 125.0; // 125°C+: Red Overheat warning

enum WarmupState {
  STATE_UNKNOWN,
  STATE_COLD,
  STATE_WARMING,
  STATE_READY,
  STATE_HOT,
  STATE_DISCONNECTED
};

inline void drawOilWarmupBar(float currentTemp, bool isDisconnected) {
  static float lastDrawnTemp = -999.0;
  static bool lastDisconnected = false;
  static WarmupState currentState = STATE_UNKNOWN;

  // Determine State with Hysteresis (prevents rapid flickering between labels at boundary temps)
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

  // Set colors and labels based on stable state
  uint16_t barColor;
  const char* label;

  switch (currentState) {
    case STATE_COLD:
      barColor = ST77XX_RED;
      label = "COLD";
      break;
    case STATE_WARMING:
      barColor = 0xbd83;
      label = "WARMING";
      break;
    case STATE_READY:
      barColor = 0x0400;
      label = "READY";
      break;
    case STATE_HOT:
      barColor = 0x9041;
      label = "HOT!";
      break;
    case STATE_DISCONNECTED:
    default:
      barColor = 0x4208; // Dim gray
      label = "--";
      break;
  }

  static WarmupState lastDrawnState = STATE_UNKNOWN;

  // Redraw only when temp changes by >= 1°C or when state flips
  if (abs(currentTemp - lastDrawnTemp) < 1.0 && currentState == lastDrawnState) {
    return;
  }
  lastDrawnTemp = currentTemp;
  lastDrawnState = currentState;

  // Fully clear the entire text area from the top of the box down to the progress bar
  tft.fillRect(WARMUP_BOX_X, WARMUP_BOX_Y, WARMUP_BOX_W, WARMUP_BAR_Y - WARMUP_BOX_Y, ST77XX_BLACK);

  // Draw Status Label centered
  tft.setFont(&DS_Digital_Bold10pt7b);
  tft.setTextColor(barColor);
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
  int textX = WARMUP_BOX_X + (WARMUP_BOX_W - w) / 2 - x1;
  int textY = WARMUP_BOX_Y + 20;
  tft.setCursor(textX, textY);
  tft.print(label);

  // Draw Bar Outline
  tft.drawRoundRect(WARMUP_BAR_X, WARMUP_BAR_Y, WARMUP_BAR_W, WARMUP_BAR_H, 3, barColor);

  // Calculate Bar Fill (maps 20°C -> 85°C to 0% -> 100%)
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
  // Clear unfilled portion
  if (innerW - fillW > 0) {
    tft.fillRect(WARMUP_BAR_X + 2 + fillW, WARMUP_BAR_Y + 2, innerW - fillW, innerH, ST77XX_BLACK);
  }
}

#endif
