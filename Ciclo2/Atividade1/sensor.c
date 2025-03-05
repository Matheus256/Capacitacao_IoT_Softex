#include "sensor.h"

//Para o sensor de temperatura
#define NTC_PIN 35


float read_temp(){
  int analogValue = analogRead(NTC_PIN);
  float Temperature = 1 / (log(1 / (4095.0 / analogValue - 1)) / 3950.0 + 1.0 / 298.15) - 273.15;
  return Temperature;
}