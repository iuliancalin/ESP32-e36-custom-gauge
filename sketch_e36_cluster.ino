#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// Custom Font Libraries
#include "custom_fonts/DS_Digital_Bold16pt7b.h"
#include "custom_fonts/DS_Digital_Bold12pt7b.h"
#include "custom_fonts/DS_Digital_Bold10pt7b.h"
#include "Fonts/FreeSansBold9pt7b.h"
#include "custom_fonts/Nunito_Regular10pt7b.h"

// --- COLOR CONFIGURATIONS ---
#define COLOR_DARK_ORANGE        0x0800
#define COLOR_LIGHT_ORANGE       0xfa20
#define COLOR_LIGHT_ORANGE_SMALL 0xfc80

// --- FONT CONFIGURATIONS ---
// Change this single line to swap unit fonts ("V" and "C") across the whole sketch:
#define LABEL_FONT DS_Digital_Bold10pt7b

// Modular Segment Architecture
#include "low_beam_icon.h"
#include "Illumination.h"
#include "sensor_parameter.h"
#include "splash_screen.h"
#include "comfort_blinker.h"
#include "cruise_control_icon.h"
#include "cruise_control.h"
#include "battery_icon.h"
#include "oil_icon.h"

// --- HARDWARE PIN ASSIGNMENTS ---
#define TFT_CS        0   // Display CS
#define TFT_RST       16  // Display RES
#define TFT_DC        4   // Display DC
#define TFT_MOSI      21  // Display SDA
#define TFT_SCLK      23  // Display SCL
#define TFT_BL_PIN    2   // Display Backlight
#define CRUISE_IN_PIN 5   // Cruise control input (GPIO 5)

// =========================================================================
// --- PERFECT GRID UI LAYOUT (320x170 DISPLAY) ---
// =========================================================================

struct ElementLayout {
  int boxX, boxY, boxW, boxH;
  int offsetX, offsetY; // Position of text RELATIVE to box top-left corner
};

// --- TOP ROW (2 Boxes + Divider) ---
const ElementLayout batteryUI = {
  .boxX    = 0,   .boxY    = 35,  // Main Box Position
  .boxW    = 155, .boxH    = 56,  // Main Box Size
  .offsetX = 46,  .offsetY = 42   // Offset adjusted right to leave room for the battery icon
};

const ElementLayout oilTempUI = {
  .boxX    = 159, .boxY    = 35,  // Main Box Position
  .boxW    = 155, .boxH    = 56,  // Main Box Size
  .offsetX = 55,  .offsetY = 42   // Offset adjusted right to leave room for the oil icon
};

// Icon Positions (Placed inside top boxes on the left side)
const int batteryIconX = batteryUI.boxX + 5;
const int batteryIconY = batteryUI.boxY + 12;

const int oilIconX     = oilTempUI.boxX + 5;
const int oilIconY     = oilTempUI.boxY + 12;

// Top Center Divider (Centered in the 4px gap between top boxes)
const int dividerX = 156;       // Exactly between X=158 and X=162
const int dividerY = 35;        // Aligned with top box Y position
const int dividerW = 2;         // Divider thickness
const int dividerH = 56;        // Matched to top box height (56px)

// --- BOTTOM ROW (3 Boxes) ---
// Grid Math: 3 (margin) + 88 + 4 (gap) + 130 + 4 (gap) + 88 + 3 (margin) = 320px
const ElementLayout bottomLeftUI  = { .boxX = 0,   .boxY = 105, .boxW = 88,  .boxH = 55, .offsetX = 0, .offsetY = 0 };
const ElementLayout bottomMidUI   = { .boxX = 92,  .boxY = 105, .boxW = 130, .boxH = 55, .offsetX = 0, .offsetY = 0 };
const ElementLayout bottomRightUI = { .boxX = 226, .boxY = 105, .boxW = 88,  .boxH = 55, .offsetX = 0, .offsetY = 0 };

// --- ICON RELATIVE POSITIONS ---
// Dynamically anchored to parent box top-left corners
const int lowBeamIconX = bottomLeftUI.boxX + 21;  // Centered inside 93px box
const int lowBeamIconY = bottomLeftUI.boxY + 2;

const int cruiseIconX  = bottomRightUI.boxX + 21; // Centered inside 93px box
const int cruiseIconY  = bottomRightUI.boxY + 2;

// Degree Symbol offset relative to Oil Temp text baseline Y
const int degreeOffsetY = -29;  // Ring Y relative to text baseline Y
const int degreeOuterR  = 6;    // Degree ring size
const int degreeInnerR  = 3;    // Degree ring hole size
// =========================================================================

// --- SENSOR & SYSTEM GLOBAL STATES ---
bool sensorDisconnected = false;
int lastPwmDuty = -1;

float lastVoltage = -1.0;
float lastOilTemp = -1.0;
bool lastHeadlightsState = false;
bool firstRun = true;

SPIClass *vspi = new SPIClass(VSPI);
Adafruit_ST7789 tft = Adafruit_ST7789(vspi, TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  setupBlinkers();

  pinMode(TFT_BL_PIN, OUTPUT);
  analogWrite(TFT_BL_PIN, 255);
  pinMode(CRUISE_IN_PIN, INPUT_PULLUP);

  vspi->begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.init(170, 320);
  tft.setRotation(1);
  tft.invertDisplay(true);
  tft.fillScreen(ST77XX_BLACK);
  renderSplashScreen();
}

void loop() {
  handleComfortBlinkers();

  bool headlightsOn = false;
  updateClusterIllumination(lastPwmDuty, headlightsOn);

  float voltage = readVoltage();
  float ntcResistance = readNtcResistance();
  float oilTemp = calculateTemperature(ntcResistance);

  if (!sensorDisconnected && oilTemp != -999.0) {
    oilTemp += TEMP_CALIBRATION_OFFSET;
  }

  // --- STATIC DRAW CANVAS ---
  if (firstRun) {
    tft.fillScreen(ST77XX_BLACK);

    // Top Row Boxes + Divider
    tft.fillRect(batteryUI.boxX, batteryUI.boxY, batteryUI.boxW, batteryUI.boxH, COLOR_DARK_ORANGE);
    tft.fillRect(oilTempUI.boxX, oilTempUI.boxY, oilTempUI.boxW, oilTempUI.boxH, COLOR_DARK_ORANGE);
    tft.fillRect(dividerX, dividerY, dividerW, dividerH, COLOR_LIGHT_ORANGE);

    // Draw static icons ONCE at boot
    drawBatteryIcon(batteryIconX, batteryIconY, COLOR_LIGHT_ORANGE);
    drawOilIcon(oilIconX, oilIconY, COLOR_LIGHT_ORANGE);

    firstRun = false;
  }

  // --- DYNAMIC VOLTAGE UPDATE ---
  if (abs(voltage - lastVoltage) >= 0.1) {
    int clearX = batteryUI.boxX + batteryUI.offsetX;
    int clearW = batteryUI.boxW - batteryUI.offsetX;
    tft.fillRect(clearX, batteryUI.boxY, clearW, batteryUI.boxH, COLOR_DARK_ORANGE);

    tft.setFont(&DS_Digital_Bold12pt7b);
    tft.setTextColor(COLOR_LIGHT_ORANGE);

    tft.setCursor(clearX, batteryUI.boxY + batteryUI.offsetY);
    tft.print(voltage, 1);

    tft.setFont(&LABEL_FONT);
    tft.print("V");

    lastVoltage = voltage;
  }

  // --- DYNAMIC TEMPERATURE UPDATE ---
  if (abs(oilTemp - lastOilTemp) >= 1.0) {
    int clearX = oilTempUI.boxX + oilTempUI.offsetX;
    int clearW = oilTempUI.boxW - oilTempUI.offsetX;
    tft.fillRect(clearX, oilTempUI.boxY, clearW, oilTempUI.boxH, COLOR_DARK_ORANGE);

    tft.setFont(&DS_Digital_Bold12pt7b);
    tft.setTextColor(COLOR_LIGHT_ORANGE);

    int textX = clearX;
    int textY = oilTempUI.boxY + oilTempUI.offsetY;

    tft.setCursor(textX, textY);

    // Print temperature or error string
    if (sensorDisconnected || oilTemp == -999.0) {
      tft.print("--");
    } else {
      tft.print(oilTemp, 0);

      // Dynamic X position: automatically locks to the end of printed text
      int degreeX = tft.getCursorX() + 14;
      int degreeY = textY + degreeOffsetY;

      // Draw degree ring
      tft.fillCircle(degreeX, degreeY, degreeOuterR, COLOR_LIGHT_ORANGE);
      tft.fillCircle(degreeX, degreeY, degreeInnerR, COLOR_DARK_ORANGE);

      // Draw 'C' offset from the degree ring
      tft.setCursor(degreeX + 7, textY);
      tft.setFont(&LABEL_FONT);
      tft.print("C");
    }

    lastOilTemp = oilTemp;
  }

  // --- DYNAMIC HEADLIGHT ICON UPDATE ---
  if (headlightsOn != lastHeadlightsState) {
    drawLowBeamIcon(lowBeamIconX, lowBeamIconY, headlightsOn);
    lastHeadlightsState = headlightsOn;
  }

  // --- DYNAMIC CRUISE CONTROL ICON UPDATE ---
  bool cruiseActive = (digitalRead(CRUISE_IN_PIN) == LOW);
  drawCruiseIcon(cruiseIconX, cruiseIconY, cruiseActive);

  delay(10);
}