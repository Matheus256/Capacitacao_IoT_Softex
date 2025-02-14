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
#define LED_4 GPIO_NUM_0
#define LED_5 GPIO_NUM_2
#define LED_6 GPIO_NUM_15
#define LED_7 GPIO_NUM_18

//definindo as mascaras de saída
#define GPIO_OUTPUT_PIN_SEL ((1ULL<<LED_0) | (1ULL<<LED_1) | (1ULL<<LED_2) | (1ULL<<LED_3))
#define GPIO_OUTPUT_PIN_SEL2 ((1ULL<<LED_4) | (1ULL<<LED_5) | (1ULL<<LED_6) | (1ULL<<LED_7))

//definindo as mascaras de entrada
#define GPIO_INPUT_PIN_SEL ((1ULL<<BUTTON_0) | (1ULL<<BUTTON_1))

//Definição das rotinas de interrupção
static void IRAM_ATTR gpio0_isr_handle(void* arg) {
  uint8_t *v = (uint8_t*) arg;
  *v=1;
}

static void IRAM_ATTR gpio1_isr_handle(void* arg) {
  uint8_t *v = (uint8_t*) arg;
  *v=0;
}

void setup() {
  //Setando LEDs
  gpio_config_t led_conf = {};
  led_conf.intr_type = GPIO_INTR_DISABLE;
  led_conf.mode = GPIO_MODE_OUTPUT;
  led_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL | GPIO_OUTPUT_PIN_SEL2;
  led_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  led_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&led_conf);

  //Setando butões
  gpio_config_t button_conf = {};
  button_conf.intr_type = GPIO_INTR_LOW_LEVEL;
  button_conf.mode = GPIO_MODE_INPUT;
  button_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
  button_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  button_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&button_conf);

}

uint8_t valor=1;
uint8_t direcao=0;

void loop() {
  //Instalação do isr service
  gpio_install_isr_service(ESP_INTR_FLAG_LOWMED);
  //Associação das interrupç~pes com os botões
  gpio_isr_handler_add(BUTTON_0, gpio0_isr_handle, (void*) &direcao);
  gpio_isr_handler_add(BUTTON_1, gpio1_isr_handle, (void*) &direcao);

  gpio_set_level(LED_0, (valor>>0)&1);
  gpio_set_level(LED_1, (valor>>1)&1);
  gpio_set_level(LED_2, (valor>>2)&1);
  gpio_set_level(LED_3, (valor>>3)&1);
  gpio_set_level(LED_4, (valor>>4)&1);
  gpio_set_level(LED_5, (valor>>5)&1);
  gpio_set_level(LED_6, (valor>>6)&1);
  gpio_set_level(LED_7, (valor>>7)&1);
  
  printf("Valor = %d\n", valor);
  if (direcao == 0) {
      valor = valor<<1;
    if (valor == 0) {
      valor=1;
    }
  }
  else {
    valor = valor>>1;
    if (valor==0){
      valor=128;
    }
  }
  delay(1000); // this speeds up the simulation
}
