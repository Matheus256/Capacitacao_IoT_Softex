//Baseado no projeto já existente, refiz para acompanhar o que tava acontecendo

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_RED GPIO_NUM_5
#define LED_BLUE GPIO_NUM_4
#define LED_GREEN GPIO_NUM_2

struct led_task_parameters_t {
  gpio_num_t led_gpio;
  TickType_t blink_time;
};

//Definindo os parâmetros das led_task (tarefa dos leds)
static led_task_parameters_t red_led_gpio = {LED_RED, 2000};
static led_task_parameters_t blue_led_gpio = {LED_BLUE, 1000};
static led_task_parameters_t green_led_gpio = {LED_GREEN, 500};

void led_task(void *pvParameter) {
  gpio_num_t led_gpio = ((led_task_parameters_t *)pvParameter)->led_gpio;
  TickType_t blink_time = ((led_task_parameters_t *)pvParameter)->blink_time;
  uint8_t led_value = 0;
  gpio_reset_pin(led_gpio);
  gpio_set_direction(led_gpio, GPIO_MODE_OUTPUT);

  while(1) {
    gpio_set_level(led_gpio, led_value);
    led_value = !led_value;
    vTaskDelay( blink_time / portTICK_PERIOD_MS); //Se usar delay da errado
  }
  vTaskDelete( NULL );
}

extern "C" void app_main() {
  //Definindo as tasks para cada um dos leds
  xTaskCreate (
    &led_task, // apontar a função que realiza a tarefa
    "red_led_task", // nome da tarefa
    2048, // tamanho da pilha?!
    &red_led_gpio, // apontar os parâmetros da tarefa
    5, // prioridade
    NULL); // ponteiro exterior para a task handle 

  xTaskCreate (
    &led_task,
    "blue_led_task",
    2048,
    &blue_led_gpio,
    5,
    NULL);

  xTaskCreate (
    &led_task,
    "green_led_task",
    2048,
    &green_led_gpio,
    5,
    NULL);
}