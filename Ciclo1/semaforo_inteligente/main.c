#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>

#include "ultrasonic.h"

#define LED_RED GPIO_NUM_4
#define LED_YELLOW GPIO_NUM_2
#define LED_GREEN GPIO_NUM_15
#define GPIO_OUTPUT_PIN_SEL ((1ULL<<LED_RED) | (1ULL<<LED_YELLOW) | (1ULL<<LED_GREEN))


#define ECHO_GPIO 12
#define TRIGGER_GPIO 13
#define MAX_DISTANCE_CM 500 // Maximum of 5 meters

static TaskHandle_t xtask_handle_leds = NULL;
uint8_t state = 1;
int delay_time;

void ultrasonic_test(void *pvParameters)
{
    float distance;
    uint8_t pause=0;

    ultrasonic_sensor_t sensor = {
        .trigger_pin = TRIGGER_GPIO,
        .echo_pin = ECHO_GPIO
    };

    ultrasonic_init(&sensor);

    while (true) {
        esp_err_t res = ultrasonic_measure(&sensor, MAX_DISTANCE_CM, &distance);

        if (res == ESP_OK) {
            printf("Distancia: %0.04f m\n", distance);

            if(distance<=1 && pause==0){
              gpio_set_level(LED_RED,1);
              gpio_set_level(LED_YELLOW,0);
              gpio_set_level(LED_GREEN,0);
              vTaskSuspend(xtask_handle_leds);
              pause=1;
            }
            else if (distance>1 && pause==1){
              state=1;
              pause=0;
              vTaskResume(xtask_handle_leds);
            }
        } // Print error
        else {
            printf("Error %d: ", res);
            switch (res) {
                case ESP_ERR_ULTRASONIC_PING:
                    printf("Cannot ping (device is in invalid state)\n");
                    break;
                case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
                    printf("Ping timeout (no device found)\n");
                    break;
                case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
                    printf("Echo timeout (i.e. distance too big)\n");
                    break;
                default:
                    printf("%s\n", esp_err_to_name(res));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void leds_init(){
  gpio_config_t led_conf = {};
  led_conf.intr_type = GPIO_INTR_DISABLE;
  led_conf.mode = GPIO_MODE_OUTPUT;
  led_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
  led_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  led_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&led_conf);
}

void leds_control(void *pvParameters){
  while(true){
    printf("State: %d\n",state);
    gpio_set_level(LED_RED,(state>>0)&1);
    gpio_set_level(LED_YELLOW,(state>>1)&1);
    gpio_set_level(LED_GREEN,(state>>2)&1);
    if(state==2){
      state=1;
      delay_time=1000;
    }
    else if(state==4){
      state=2;
      delay_time=10000;
    }
    else {
      state=4;
      delay_time=5000;
    }
    vTaskDelay( delay_time / portTICK_RATE_MS);
  }
}

void app_main()
{
    leds_init();

    xTaskCreate(
    ultrasonic_test,
    "ultrasonic_test",
    configMINIMAL_STACK_SIZE * 3,
    NULL,
    5,
    NULL);

    xTaskCreate(
      leds_control,
      "leds_control",
      2048,
      NULL,
      5,
      &xtask_handle_leds);
}
