#ifndef CRUISE_CONTROL_H
#define CRUISE_CONTROL_H

#include <Arduino.h>
#include <Adafruit_ST7789.h>
#include "cruise_control_icon.h"

extern Adafruit_ST7789 tft;

// Renders the cruise control icon transparently based on state
// This skips 0x0000 (black pixels), so the background remains visible
inline void drawCruiseIcon(int x, int y, bool state) {
  static bool lastState = false; // Prevents spamming pixel draws

  // Only redraw if the cruise state actually flipped
  if (state != lastState) {
    if (state) {
      // Draw the icon pixels, skip the black background
      for (int yIdx = 0; yIdx < CRUISE_HEIGHT; yIdx++) {
        for (int xIdx = 0; xIdx < CRUISE_WIDTH; xIdx++) {
          uint16_t color = pgm_read_word(&Cruise_Control_2[yIdx * CRUISE_WIDTH + xIdx]);
          if (color != 0x0000) { 
            tft.drawPixel(x + xIdx, y + yIdx, color);
          }
        }
      }
    } else {
      // Target ONLY the spots where the icon was, turning them back to black
      for (int yIdx = 0; yIdx < CRUISE_HEIGHT; yIdx++) {
        for (int xIdx = 0; xIdx < CRUISE_WIDTH; xIdx++) {
          uint16_t color = pgm_read_word(&Cruise_Control_2[yIdx * CRUISE_WIDTH + xIdx]);
          if (color != 0x0000) { 
            tft.drawPixel(x + xIdx, y + yIdx, ST77XX_BLACK);
          }
        }
      }
    }
    lastState = state; // Save current state
  }
}

#endif