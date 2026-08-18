#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H

#include <Adafruit_ST7789.h>

extern Adafruit_ST7789 tft;

inline void renderSplashScreen() {
  tft.setTextColor(COLOR_LIGHT_ORANGE, ST77XX_BLACK); 
  tft.setFont(); 
  tft.setTextSize(4);
  tft.setCursor(80, 65); 
  tft.print("BMW E36");

  tft.setTextSize(2);
  tft.setCursor(60, 115);
  tft.print("Reading sensors");

  for (int i = 0; i < 12; i++) {
    tft.fillRect(248, 115, 45, 20, ST77XX_BLACK);
    tft.setCursor(248, 115);
    
    switch (i % 4) {
      case 1: tft.print(".");   break;
      case 2: tft.print("..");  break;
      case 3: tft.print("..."); break;
    }
    delay(150);
  }
  tft.fillScreen(ST77XX_BLACK);
}

#endif