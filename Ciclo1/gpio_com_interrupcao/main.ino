#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define BUTTON_0 GPIO_NUM_34
#define BUTTON_1 GPIO_NUM_32
#define LED_0 GPIO_NUM_5
#define LED_1 GPIO_NUM_16
#define LED_2 GPIO_NUM_17
#define LED_3 GPIO_NUM_4

#define GPIO_OUTPUT_PIN_SEL ((1ULL<<LED_0) | (1ULL<<LED_1) | (1ULL<<LED_2) | (1ULL<<LED_3))

#define GPIO_INPUT_PIN_SEL ((1ULL<<BUTTON_0) | (1ULL<<BUTTON_1))

static void IRAM_ATTR gpio0_isr_handle(void* arg) {
  uint8_t *v = (uint8_t*) arg;
  *v=1;
}

static void IRAM_ATTR gpio1_isr_handle(void* arg) {
  uint8_t *v = (uint8_t*) arg;
  *v=0;
}

uint8_t  valor=0;

void setup() {
  gpio_config_t led_conf ={};
  led_conf.intr_type = GPIO_INTR_DISABLE;
  led_conf.mode = GPIO_MODE_OUTPUT;
  led_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
  led_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  led_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&led_conf);

  gpio_config_t button_conf = {};
  button_conf.intr_type = GPIO_INTR_LOW_LEVEL;
  button_conf.mode = GPIO_MODE_INPUT;
  button_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
  button_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  button_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&button_conf);
}

void loop() {
 
  //instalar gpio isr service
  gpio_install_isr_service(ESP_INTR_FLAG_LOWMED);
  //hook isr handle for especific gpio pin
  gpio_isr_handler_add(BUTTON_0, gpio0_isr_handle, (void*) &valor);
  gpio_isr_handler_add(BUTTON_1, gpio1_isr_handle, (void*) &valor);

  gpio_set_level(LED_0,valor);
  gpio_set_level(LED_1,valor);
  gpio_set_level(LED_2,valor);
  gpio_set_level(LED_3,valor);
  printf("Valor = %d\n", valor);
}