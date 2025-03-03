#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h" //Para semáforos
#include "esp_system.h"
#include "driver/gpio.h"
#include <esp_err.h>

#define LED_0 GPIO_NUM_21
#define LED_1 GPIO_NUM_19
#define LED_2 GPIO_NUM_18
#define GPIO_OUTPUT_PIN_SEL ((1ULL<<LED_0) | (1ULL<<LED_1) | (1ULL<<LED_2))

static TaskHandle_t task_producer = NULL;
SemaphoreHandle_t xSemaphore = NULL;
uint8_t counter=0;

void init_leds(){
  gpio_config_t led_conf = {};
  led_conf.intr_type = GPIO_INTR_DISABLE;
  led_conf.mode = GPIO_MODE_OUTPUT;
  led_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
  led_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  led_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&led_conf);
}

void upgrade_leds(){
  printf("Contador = %d\n", counter);
  if(counter==1){
    gpio_set_level(LED_0,1);
    gpio_set_level(LED_1,0);
    gpio_set_level(LED_2,0);
  }
  else if(counter==2){
    gpio_set_level(LED_0,1);
    gpio_set_level(LED_1,1);
    gpio_set_level(LED_2,0);
  }
  else if(counter==3){
    gpio_set_level(LED_0,1);
    gpio_set_level(LED_1,1);
    gpio_set_level(LED_2,1);
  }
  else {
    gpio_set_level(LED_0,0);
    gpio_set_level(LED_1,0);
    gpio_set_level(LED_2,0);
  }
}

void producer(void *pvParameters){
  while(true){
    if (xSemaphoreGive(xSemaphore) == pdTRUE){
      counter = counter + 1;
      upgrade_leds();
    }
    else {
      vTaskSuspend(NULL);
    }
    vTaskDelay( 1000 / portTICK_RATE_MS);
  }
}

void consumer(void *pvParameters){
  while(true){
    if (xSemaphoreTake(xSemaphore, 5000 / portTICK_RATE_MS) == pdTRUE){
      counter = counter - 1;
      upgrade_leds();
    }
    else {
      vTaskResume(task_producer);
    }
    vTaskDelay( 1200 / portTICK_RATE_MS);
  }
}

extern "C" void app_main(){
  init_leds();
  xSemaphore = xSemaphoreCreateCounting(3,0);
  xTaskCreate(
    producer,
    "producer",
    2048,
    NULL,
    5,
    &task_producer);

  xTaskCreate(
    consumer,
    "consumer",
    2048,
    NULL,
    5,
    NULL);  
}