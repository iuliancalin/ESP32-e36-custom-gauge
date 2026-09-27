#ifndef COMFORT_BLINKER_H
#define COMFORT_BLINKER_H

#include <Arduino.h>

// --- HARDWARE PINS (ES32C14 BOARD SPECIFICATION) ---
#define PIN_IN1_LEFT_SW   19 // IN1 Terminal (GPIO 19)
#define PIN_IN2_RIGHT_SW  18 // IN2 Terminal (GPIO 18)

#define PIN_RELAY_LEFT    27 // CH1 Relay Output (GPIO 27)
#define PIN_RELAY_RIGHT   14 // CH2 Relay Output (GPIO 14)

// Relay Logic (NO Contacts: HIGH = ON, LOW = OFF)
#define RELAY_ON  HIGH
#define RELAY_OFF LOW

// Timing Configurations
const unsigned long HOLD_DURATION_MS   = 2800; // Hold relay closed for 2.5 seconds
const unsigned long TAP_MAX_MS         = 300;  // Max trigger pulse duration to register as a "tap"
const unsigned long PRE_RELAY_DELAY_MS = 300;  // Cadence delay after tap before energizing relay

enum BlinkerState { IDLE, WAIT_RELEASE_LEFT, WAIT_RELEASE_RIGHT, DELAY_LEFT, DELAY_RIGHT, HOLD_LEFT, HOLD_RIGHT };
static BlinkerState currentState = IDLE;

inline void setupBlinkers() {
  pinMode(PIN_IN1_LEFT_SW, INPUT_PULLUP);
  pinMode(PIN_IN2_RIGHT_SW, INPUT_PULLUP);
  
  pinMode(PIN_RELAY_LEFT, OUTPUT);
  pinMode(PIN_RELAY_RIGHT, OUTPUT);
  
  digitalWrite(PIN_RELAY_LEFT, RELAY_OFF);
  digitalWrite(PIN_RELAY_RIGHT, RELAY_OFF);
}

inline void handleComfortBlinkers() {
  static unsigned long leftPressStart = 0;
  static unsigned long rightPressStart = 0;
  static unsigned long delayStartTime = 0;
  static unsigned long holdStartTime = 0;

  unsigned long currentMillis = millis();

  // Read NPN Inputs (LOW when grounded/active)
  bool leftIn = (digitalRead(PIN_IN1_LEFT_SW) == LOW);
  bool rightIn = (digitalRead(PIN_IN2_RIGHT_SW) == LOW);

  // --- STATE MACHINE ---
  switch (currentState) {
    case IDLE:
      digitalWrite(PIN_RELAY_LEFT, RELAY_OFF);
      digitalWrite(PIN_RELAY_RIGHT, RELAY_OFF);
      
      if (leftIn) {
        leftPressStart = currentMillis;
        currentState = WAIT_RELEASE_LEFT;
      } else if (rightIn) {
        rightPressStart = currentMillis;
        currentState = WAIT_RELEASE_RIGHT;
      }
      break;

    case WAIT_RELEASE_LEFT:
      // Wait for user to release tap
      if (!leftIn) {
        unsigned long duration = currentMillis - leftPressStart;
        if (duration <= TAP_MAX_MS) {
          // Short tap! Wait for OEM flasher cadence delay before clicking relay
          currentState = DELAY_LEFT;
          delayStartTime = currentMillis;
        } else {
          // Long press / locked lever -> ignore and return to IDLE
          currentState = IDLE;
        }
      }
      break;

    case WAIT_RELEASE_RIGHT:
      // Wait for user to release tap
      if (!rightIn) {
        unsigned long duration = currentMillis - rightPressStart;
        if (duration <= TAP_MAX_MS) {
          // Short tap! Wait for OEM flasher cadence delay before clicking relay
          currentState = DELAY_RIGHT;
          delayStartTime = currentMillis;
        } else {
          // Long press / locked lever -> ignore and return to IDLE
          currentState = IDLE;
        }
      }
      break;

    case DELAY_LEFT:
      // Cancel immediately if opposite stalk tapped or left pressed again
      if (rightIn || leftIn) {
        currentState = IDLE;
        break;
      }
      // Wait for cadence delay before turning relay ON
      if (currentMillis - delayStartTime >= PRE_RELAY_DELAY_MS) {
        currentState = HOLD_LEFT;
        holdStartTime = currentMillis;
        digitalWrite(PIN_RELAY_LEFT, RELAY_ON);
      }
      break;

    case DELAY_RIGHT:
      // Cancel immediately if opposite stalk tapped or right pressed again
      if (leftIn || rightIn) {
        currentState = IDLE;
        break;
      }
      // Wait for cadence delay before turning relay ON
      if (currentMillis - delayStartTime >= PRE_RELAY_DELAY_MS) {
        currentState = HOLD_RIGHT;
        holdStartTime = currentMillis;
        digitalWrite(PIN_RELAY_RIGHT, RELAY_ON);
      }
      break;

    case HOLD_LEFT:
      // Cancel immediately if opposite blinker tapped
      if (rightIn) {
        digitalWrite(PIN_RELAY_LEFT, RELAY_OFF);
        currentState = IDLE;
        break;
      }

      // Check if 2.5 second hold window has elapsed
      if (currentMillis - holdStartTime >= HOLD_DURATION_MS) {
        digitalWrite(PIN_RELAY_LEFT, RELAY_OFF);
        currentState = IDLE;
      }
      break;

    case HOLD_RIGHT:
      // Cancel immediately if opposite blinker tapped
      if (leftIn) {
        digitalWrite(PIN_RELAY_RIGHT, RELAY_OFF);
        currentState = IDLE;
        break;
      }

      // Check if 2.5 second hold window has elapsed
      if (currentMillis - holdStartTime >= HOLD_DURATION_MS) {
        digitalWrite(PIN_RELAY_RIGHT, RELAY_OFF);
        currentState = IDLE;
      }
      break;
  }
}

#endif
