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
const float TEMP_WARM_MAX   = 70.0; // 30°C - 70°C: YELLOW (Warming up)
const float TEMP_FULL_READY = 85.0; // 85°C: Progress bar reaches 100% full
const float TEMP_OVERHEAT   = 125.0; // 125°C+: Red Overheat warning

inline void drawOilWarmupBar(float currentTemp, bool isDisconnected) {
  static float lastDrawnTemp = -999.0;
  static bool lastDisconnected = false;
  static uint16_t lastDrawnColor = 0;

  // Determine current color state
  uint16_t barColor;
  const char* label;

  if (isDisconnected || currentTemp <= -900.0) {
    barColor = 0x4208; // Dim gray
    label = "--";
  } else if (currentTemp < TEMP_COLD_MAX) {
    barColor = ST77XX_RED;
    label = "COLD";
  } else if (currentTemp < TEMP_WARM_MAX) {
    barColor = ST77XX_YELLOW;
    label = "WARMING";
  } else if (currentTemp < TEMP_OVERHEAT) {
    barColor = ST77XX_GREEN;
    label = "READY";
  } else {
    barColor = ST77XX_RED;
    label = "HOT!";
  }

  // Redraw only when temp changes by >= 1°C, disconnect status flips, or color state changes
  if (abs(currentTemp - lastDrawnTemp) < 1.0 && isDisconnected == lastDisconnected && barColor == lastDrawnColor) {
    return;
  }
  lastDrawnTemp = currentTemp;
  lastDisconnected = isDisconnected;
  lastDrawnColor = barColor;

  // Clear only the text area above the bar
  tft.fillRect(WARMUP_BOX_X, WARMUP_BOX_Y, WARMUP_BOX_W, 28, ST77XX_BLACK);

  // Draw Status Label centered
  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(barColor);
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(WARMUP_BOX_X + (WARMUP_BOX_W - w) / 2, WARMUP_BOX_Y + 20);
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
