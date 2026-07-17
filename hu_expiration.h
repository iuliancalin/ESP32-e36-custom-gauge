#ifndef HU_EXPIRATION_H
#define HU_EXPIRATION_H

#include <Adafruit_ST7789.h>

// Look, mom! No font includes here anymore.
extern Adafruit_ST7789 tft;

// Link to the centralized coordinates in your main .ino file
extern const int huX;
extern const int huY;

#define COLOR_LIGHT_BLUE   0x053B

inline void handleHuExpiration(bool &huVisible, const unsigned long bootTime, const unsigned long HU_TIMEOUT) {
  if (huVisible) {
    if (millis() - bootTime < HU_TIMEOUT) {
      static bool firstDraw = true;
      if (firstDraw) {
        tft.setFont(&DS_Digital_Bold10pt7b); // The compiler will see this from the main .ino file
        tft.setTextColor(COLOR_LIGHT_BLUE);
        tft.setCursor(huX, huY + 12); 
        tft.print("HU 10/27");
        firstDraw = false;
      }
    } else {
      tft.setFont(&DS_Digital_Bold10pt7b);
      tft.setTextColor(ST77XX_BLACK);
      tft.setCursor(huX, huY + 12);
      tft.print("HU 10/27");
      huVisible = false; 
    }
  }
}

#endif