#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H

#include <Adafruit_ST7789.h>

extern Adafruit_ST7789 tft;
#define COLOR_LIGHT_ORANGE 0xfd00

inline void renderSplashScreen() {
  tft.setTextColor(COLOR_LIGHT_ORANGE, ST77XX_BLACK); 
  tft.setFont(); 
  tft.setTextSize(4);
  tft.setCursor(80, 45); 
  tft.print("BMW E36");

  tft.setTextSize(2);
  tft.setCursor(80, 95);
  tft.print("Initializing");

  for (int i = 0; i < 12; i++) {
    tft.fillRect(238, 95, 45, 20, ST77XX_BLACK);
    tft.setCursor(238, 95);
    
    switch (i % 4) {
      case 1: tft.print(".");   break;
      case 2: tft.print("..");  break;
      case 3: tft.print("..."); break;
    }
    delay(180);
  }
  tft.fillScreen(ST77XX_BLACK);
}

#endif