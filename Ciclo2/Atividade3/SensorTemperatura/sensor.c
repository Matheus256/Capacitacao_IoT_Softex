#include "sensor.h"

#define NTC_PIN 32

float read_temp(){
  int analogValue = analogRead(NTC_PIN);
  float temperature = 1 / (log(1 / (4095.0 / analogValue - 1)) / 3950.0 + 1.0 / 298.15) - 273.15;
  return temperature;
}