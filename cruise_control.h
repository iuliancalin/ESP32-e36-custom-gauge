#ifndef CRUISE_CONTROL_H
#define CRUISE_CONTROL_H

#include <Arduino.h>
#include <Adafruit_ST7789.h>
#include "cruise_control_icon.h"

extern Adafruit_ST7789 tft;

// Timing Configuration
const unsigned long CRUISE_STANDBY_TIMEOUT_MS = 20000; // Hold yellow for 20 seconds after use

enum CruiseDisplayState {
  CRUISE_DISP_OFF,
  CRUISE_DISP_GREEN,
  CRUISE_DISP_YELLOW
};

// Renders the cruise control icon based on display state (Green, Yellow, or Erased Black)
inline void drawCruiseIconState(int x, int y, CruiseDisplayState state) {
  static CruiseDisplayState lastDrawnState = CRUISE_DISP_OFF;

  if (state != lastDrawnState) {
    for (int yIdx = 0; yIdx < CRUISE_HEIGHT; yIdx++) {
      for (int xIdx = 0; xIdx < CRUISE_WIDTH; xIdx++) {
        uint16_t color = pgm_read_word(&Cruise_Control_2[yIdx * CRUISE_WIDTH + xIdx]);
        if (color != 0x0000) { 
          if (state == CRUISE_DISP_GREEN) {
            tft.drawPixel(x + xIdx, y + yIdx, color);
          } else if (state == CRUISE_DISP_YELLOW) {
            // Convert green anti-aliased shades to yellow
            uint16_t g = (color >> 5) & 0x3F;
            uint16_t r = g >> 1;
            uint16_t yellow = (r << 11) | (g << 5);
            tft.drawPixel(x + xIdx, y + yIdx, yellow);
          } else {
            // Off: erase pixels to black
            tft.drawPixel(x + xIdx, y + yIdx, ST77XX_BLACK);
          }
        }
      }
    }
    lastDrawnState = state;
  }
}

// Manages cruise active (green), standby (yellow for 20s), and off states
inline void drawCruiseIcon(int x, int y, bool cruiseActive) {
  static bool wasActive = false;
  static unsigned long turnOffTime = 0;
  
  unsigned long now = millis();
  CruiseDisplayState targetState;

  if (cruiseActive) {
    targetState = CRUISE_DISP_GREEN;
    wasActive = true;
    turnOffTime = 0;
  } else {
    // Cruise is currently inactive
    if (wasActive) {
      // Just switched from ON to OFF! Start 20s yellow timer
      turnOffTime = now;
      wasActive = false;
    }

    if (turnOffTime > 0 && (now - turnOffTime < CRUISE_STANDBY_TIMEOUT_MS)) {
      targetState = CRUISE_DISP_YELLOW;
    } else {
      targetState = CRUISE_DISP_OFF;
      turnOffTime = 0;
    }
  }

  drawCruiseIconState(x, y, targetState);
}

#endif