#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>


#include "sd1306.h"
#include "int_i2c.h"
#include "esp_log.h"
#include "board.h"
#include "sensor.h"
#include <math.h>

//uint8_t Temperature;
float Temperature;
float Temps[100];
int counter = 0;

void mean_calculation(float values[]){
  float mean = 0;
  double deviation = 0;

  //Cálculo da Média
  for (int i = 0; i < 100; i++)
    mean = mean + values[i];
  mean = mean/100;

  //Cálculo do desvio padrão
  for (int i = 0; i < 100; i++){
    double v = values[i] - mean;
    deviation = deviation + v*v;
  }
  deviation = sqrt(deviation/99);

  //Armazenamento e leitura do cartão SD
  ssd_card_write(&mean, &deviation);
  ssd_card_read();
}

void task_read_temperature (void *pvParameters){
  while (1) {
    Temperature = read_temp();
    printf("Temperatura: %.2f °C\n",Temperature);

    char text[30];
    sprintf(text, "Temperatura:%.0f ", Temperature);
    ssd1306_display_text(text);

    if (counter == 100){
      counter = 0;
      mean_calculation(Temps);
    }
    else {
      Temps[counter] = Temperature;
      counter++;
    }
    vTaskDelay(10000 / portTICK_RATE_MS);
  }
}

void task_display_text(void *pvParameters) {
  while(1) {
    ssd1306_display_text("Softex 09\n");
    vTaskDelay(2000 / portTICK_RATE_MS);
    ssd1306_display_text("Ciclo 2  \n");
    vTaskDelay(2000 / portTICK_RATE_MS);
  }
}

void app_main(void) {
  board_init();
  //xTaskCreate(&task_display_text, "task_text", 4096, NULL, 5, NULL);
  xTaskCreate(&task_read_temperature, "task_read_temperature", 4096, NULL, 5, NULL);
}