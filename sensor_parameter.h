#ifndef SENSOR_PARAMETER_H
#define SENSOR_PARAMETER_H

#include <Arduino.h>

#define VOLT_PIN      35   
#define OIL_TEMP_PIN  34   

const float SERIES_RESISTOR = 10000.0; 
const float TEMP_CALIBRATION_OFFSET = 4.0; 

struct NTCPoint {
  float temp;
  float resistance;
};

const int TABLE_SIZE = 11;
NTCPoint table[TABLE_SIZE] = {
  {22, 56200}, {45, 21600}, {47, 20200}, {53, 16000}, {62, 11400},
  {71, 8500},  {75, 7200},  {83, 5500},  {87, 4700},  {90, 4500}, {92, 4200}
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
  return SERIES_RESISTOR * ((4095.0 - rawAdc) / rawAdc);
}

inline float calculateTemperature(float currentResistance) {
  if (sensorDisconnected || currentResistance <= 0) return 0;
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
  return 0; 
}

inline float readVoltage() {
  long sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += analogRead(VOLT_PIN);
    delay(2);
  }
  float raw = sum / 20.0;
  float adcVoltage = (raw / 4095.0) * 3.3;
  
  return (4.115 * adcVoltage) + 3.000;
}

#endif
