#ifndef ILLUMINATION_H
#define ILLUMINATION_H

#include <Arduino.h>
#include <Adafruit_ST7789.h>

// Hardware external linkages
extern Adafruit_ST7789 tft;
extern const uint16_t low_beam_icon[] PROGMEM; 

// Share dimensions defined inside low_beam_icon.h
extern const uint16_t LOW_BEAM_WIDTH;
extern const uint16_t LOW_BEAM_HEIGHT;

// --- HARDWARE PINS (V2 MASTER SPECIFICATION) ---
#define POT_PIN          36  // Vi1 - Standlicht / Potentiometer input for dimming
#define PIN_ABBLENDLICHT 39  // Vi2 - Abblendlicht / Low Beam 12V Trigger
#define TFT_BL_PIN        2  // Display Backlight Control Output

// Lowered threshold to compensate for the inline protection resistor voltage drop
const int TRIGGER_THRESHOLD = 800; 

// Processes the headlight icon state AND park light duty cycle separately
inline void updateClusterIllumination(int &lastPwmDuty, bool &lowBeamOn) {
  // 1. Check Low Beam trigger independently (ONLY for icon logic)
  int abblendlichtVal = analogRead(PIN_ABBLENDLICHT);
  lowBeamOn = (abblendlichtVal >= TRIGGER_THRESHOLD);

  // 2. Dimming logic driven ONLY by Standlicht (POT_PIN / Vi1)
  int potValue = analogRead(POT_PIN);
  int targetPwm = 255; 

  // Standlicht threshold check
  if (potValue >= 180) {
    targetPwm = map(potValue, 180, 4095, 10, 150); 
  }

  // Only update PWM if duty cycle changes by more than 2 steps (prevents ADC flicker)
  if (abs(targetPwm - lastPwmDuty) > 2) {
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

#endif
