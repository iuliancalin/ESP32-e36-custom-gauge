#ifndef CRANK_VOLTAGE_H
#define CRANK_VOLTAGE_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// Font declared in main sketch — use extern to avoid redefinition
extern const GFXfont DS_Digital_Bold12pt7b;

extern Adafruit_ST7789 tft;

// --- Tuning Parameters ---
#define CRANK_DROP_TRIGGER     1.3f   // Voltage must drop instantly by this much below baseline
#define CRANK_MAX_ENTRY_V     11.2f   // Absolute ceiling: must be below this to allow a crank trigger
#define CRANK_EXIT_THRESHOLD   11.8f   // Voltage rises above this = engine running / crank finished
#define CRANK_MIN_VALID_V       6.0f   // Below this is ignition-off / 0V (ignored)
#define CRANK_MAX_ACTIVE_MS   5000UL   // Max allowed cranking window safety timeout

// Display duration after crank ends (ms)
#define CRANK_DISPLAY_MS      5000UL

// bottomMidUI geometry (matches sketch_e36_cluster.ino)
#define CRANK_BOX_X  92
#define CRANK_BOX_Y  105
#define CRANK_BOX_W  130
#define CRANK_BOX_H  55

// Colors (must match main sketch palette)
#define CRANK_BG_COLOR   ST77XX_BLACK
#define CRANK_TEXT_COLOR 0xfa20   // COLOR_LIGHT_ORANGE

// -------------------------------------------------------------------------
// State machine
// -------------------------------------------------------------------------
enum CrankState {
  CRANK_IDLE,      // Normal operation — oil bar owns the box
  CRANK_ACTIVE,    // Engine is cranking — recording minimum voltage
  CRANK_SHOWING    // Crank ended — displaying captured voltage for CRANK_DISPLAY_MS
};

static CrankState     _crankState    = CRANK_IDLE;
static float          _minCrankV     = 99.0f;
static float          _baselineV     = 12.0f; // Floating baseline that adapts to slow sags
static unsigned long  _showStart     = 0;
static unsigned long  _activeStart   = 0;   
static float          _lastShownV    = -1.0f; 

// -------------------------------------------------------------------------
// Call once per loop() BEFORE drawOilWarmupBar / drawCrankVoltage.
// Returns true while the crank display should own the bottomMidUI box.
// -------------------------------------------------------------------------
inline bool updateCrankDetection(float voltage) {
  // Initialize baseline on first run
  if (_baselineV < 6.0f || _baselineV > 15.0f) {
    _baselineV = voltage;
  }

  switch (_crankState) {

    case CRANK_IDLE:
      // Smoothly track normal system voltage. 
      // If voltage is drifting down slowly (e.g. music playing), the baseline follows it.
      if (voltage > _baselineV) {
        _baselineV = _baselineV * 0.90f + voltage * 0.10f; // Quick recovery when charging
      } else {
        _baselineV = _baselineV * 0.99f + voltage * 0.01f; // Very slow tracking of downward sags
      }

      // Detect crank entry: Must be a SUDDEN crash below the floating baseline, 
      // below our ceiling, and not a total 0V power loss.
      if ((_baselineV - voltage >= CRANK_DROP_TRIGGER) && 
          (voltage <= CRANK_MAX_ENTRY_V) && 
          (voltage >= CRANK_MIN_VALID_V)) {
        _crankState   = CRANK_ACTIVE;
        _minCrankV    = voltage;
        _activeStart  = millis();
      }
      return false;

    case CRANK_ACTIVE:
      // Safety abort: if voltage drops to 0 (ignition turned off) or cranking takes too long (>5s)
      if (voltage < CRANK_MIN_VALID_V || (millis() - _activeStart > CRANK_MAX_ACTIVE_MS)) {
        tft.fillRect(CRANK_BOX_X, CRANK_BOX_Y, CRANK_BOX_W, CRANK_BOX_H, CRANK_BG_COLOR);
        _crankState = CRANK_IDLE;
        _baselineV  = voltage;
        _minCrankV  = 99.0f;
        _lastShownV = -1.0f;
        return false;
      }

      // Record minimum voltage during cranking window
      if (voltage < _minCrankV) {
        _minCrankV = voltage;
      }
      
      // Crank ends when voltage recovers past exit threshold (alternator kicks in / engine runs)
      if (voltage >= CRANK_EXIT_THRESHOLD) {
        _crankState  = CRANK_SHOWING;
        _showStart   = millis();
        _lastShownV  = -1.0f; // Force redraw
      }
      return true;  // Still cranking — keep box

    case CRANK_SHOWING:
      if (millis() - _showStart >= CRANK_DISPLAY_MS) {
        // Clear the box and hand it back to the oil warmup bar
        tft.fillRect(CRANK_BOX_X, CRANK_BOX_Y, CRANK_BOX_W, CRANK_BOX_H, CRANK_BG_COLOR);
        _crankState  = CRANK_IDLE;
        _baselineV   = voltage; // Reseed baseline
        _minCrankV   = 99.0f;
        _lastShownV  = -1.0f;
        return false;
      }
      return true;  // Still showing — keep box
  }

  return false;
}

// -------------------------------------------------------------------------
// Render the captured crank voltage into the bottomMidUI box.
// Call only when updateCrankDetection() returns true.
// -------------------------------------------------------------------------
inline void drawCrankVoltage() {
  if (_crankState != CRANK_SHOWING) return;

  // Avoid redraw if nothing changed
  if (_minCrankV == _lastShownV) return;
  _lastShownV = _minCrankV;

  // Clear the box
  tft.fillRect(CRANK_BOX_X, CRANK_BOX_Y, CRANK_BOX_W, CRANK_BOX_H, CRANK_BG_COLOR);

  // Measure text width to center it
  tft.setFont(&DS_Digital_Bold12pt7b);
  tft.setTextColor(CRANK_TEXT_COLOR);

  // Build string: "X.XV" e.g. "10.4V"
  char buf[8];
  dtostrf(_minCrankV, 4, 1, buf);
  // Append V
  int len = strlen(buf);
  buf[len]   = 'V';
  buf[len+1] = '\0';

  // Measure to center horizontally & vertically
  int16_t tx, ty;
  uint16_t tw, th;
  tft.getTextBounds(buf, 0, 0, &tx, &ty, &tw, &th);

  int cursorX = CRANK_BOX_X + (CRANK_BOX_W - tw) / 2 - tx;
  int cursorY = CRANK_BOX_Y + (CRANK_BOX_H + th) / 2;

  tft.setCursor(cursorX, cursorY);
  tft.print(buf);
}

#endif // CRANK_VOLTAGE_H

