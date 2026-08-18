#ifndef SENSOR_PARAMETER_H
#define SENSOR_PARAMETER_H

#include <Arduino.h>

// --- UPDATED HARDWARE PINS (ES32C14 BOARD SPEC) ---
#define VOLT_PIN       32   // Voltage Sense 
#define OIL_TEMP_PIN   33   // Oil Temp Sensor

const float SERIES_RESISTOR = 51000.0; 
const float TEMP_CALIBRATION_OFFSET = 3; 

struct NTCPoint {
  float temp;
  float resistance;
};


const int TABLE_SIZE = 20;
NTCPoint table[TABLE_SIZE] = {
  {5, 143000},  
  {10, 102000}, 
  {15, 75000}, 
  {20, 60000}, 
  {25, 48500}, 
  {30, 39000}, 
  {35, 32000}, 
  {40, 25000}, 
  {45, 21000}, 
  {50, 17500},
  {55, 14500},
  {60, 12000},
  {65, 10000},  
  {70, 8500},  
  {75, 7200}, 
  {80, 6200}, 
  {85, 5400},  
  {90, 4200}, 
  {95, 3500},
  {100, 3200}
};

extern bool sensorDisconnected;

// Read processing for hardware ADC pins
inline float readNtcResistance() {
  long sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += analogRead(OIL_TEMP_PIN);
    delay(2);
  }
  float rawAdc = sum / 20.0;
  
  if (rawAdc <= 50 || rawAdc >= 4020) { 
    sensorDisconnected = true;
    return -1.0; 
  } 
  
  sensorDisconnected = false;
  // Formula for 3.3V feed with 20k pull-down to GND
  return SERIES_RESISTOR * ((4095.0 / rawAdc) - 1.0);
}

inline float calculateTemperature(float currentResistance) {
  if (sensorDisconnected || currentResistance <= 0) return -999.0;
  if (currentResistance >= table[0].resistance) return table[0].temp; 
  if (currentResistance <= table[TABLE_SIZE - 1].resistance) return table[TABLE_SIZE - 1].temp;

  for (int i = 0; i < TABLE_SIZE - 1; i++) {
    if (currentResistance <= table[i].resistance && currentResistance >= table[i + 1].resistance) {
      float r0 = table[i].resistance;
      float r1 = table[i + 1].resistance;
      float t0 = table[i].temp;
      float t1 = table[i + 1].temp;
      return t0 + (currentResistance - r0) * ((t1 - t0) / (r1 - r0));
    }
  }
  return -999.0; 
}

inline float readVoltage() {
  long sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += analogRead(VOLT_PIN);
    delay(2);
  }
  float raw = sum / 20.0;
  
  // Single calibrated multiplier for the 15k / 3.3k divider
  // Adjust 0.004818 up or down slightly to match your multimeter!
  return raw * 0.004728; 
}
#endif