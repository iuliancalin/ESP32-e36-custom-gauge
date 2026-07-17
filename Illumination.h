#ifndef ILLUMINATION_H
#define ILLUMINATION_H

#include <Arduino.h>
#include <Adafruit_ST7789.h>

// Hardware external linkages
extern Adafruit_ST7789 tft;
extern const uint16_t low_beam_icon[] PROGMEM; 

// Share the dimensions already defined inside low_beam_icon.h
extern const uint16_t LOW_BEAM_WIDTH;
extern const uint16_t LOW_BEAM_HEIGHT;

#define POT_PIN       32
#define TFT_BL_PIN    15

// Processes the headlight state and pot-driven cluster dimming duty cycle
inline void updateClusterIllumination(int &lastPwmDuty, bool &headlightsOn) {
  int potValue = analogRead(POT_PIN);
  int targetPwm = 255; 
  headlightsOn = false;

  if (potValue >= 180) {
    headlightsOn = true;
    targetPwm = map(potValue, 180, 4095, 10, 150); 
  }

  if (targetPwm != lastPwmDuty) {
    analogWrite(TFT_BL_PIN, targetPwm);
    lastPwmDuty = targetPwm;
  }
}

// Renders the low beam headlight icon transparently based on state
inline void drawLowBeamIcon(int x, int y, bool state) {
  static bool lastState = false; // Prevents spamming pixel draws every frame

  // Only redraw if the headlight state actually flipped
  if (state != lastState) {
    if (state) {
      // Draw the green icon pixels, skip the black background
      for (int yIdx = 0; yIdx < LOW_BEAM_HEIGHT; yIdx++) {
        for (int xIdx = 0; xIdx < LOW_BEAM_WIDTH; xIdx++) {
          uint16_t color = pgm_read_word(&low_beam_icon[yIdx * LOW_BEAM_WIDTH + xIdx]);
          if (color != 0x0000) { 
            tft.drawPixel(x + xIdx, y + yIdx, color);
          }
        }
      }
    } else {
      // Target ONLY the spots where the icon was, turning them back to black
      for (int yIdx = 0; yIdx < LOW_BEAM_HEIGHT; yIdx++) {
        for (int xIdx = 0; xIdx < LOW_BEAM_WIDTH; xIdx++) {
          uint16_t color = pgm_read_word(&low_beam_icon[yIdx * LOW_BEAM_WIDTH + xIdx]);
          if (color != 0x0000) { 
            tft.drawPixel(x + xIdx, y + yIdx, ST77XX_BLACK);
          }
        }
      }
    }
    lastState = state; // Save current state
  }
}

#endif // Make sure this is still at the absolute bottom