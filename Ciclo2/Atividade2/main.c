#include <stdio.h>
#include <string.h>    //strlen
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "mqtt_client.h" //provides important functions to connect with MQTT
//#include "protocol_examples_common.h" //important for running different protocols in code
#include "esp_event.h" //managing events of mqtt

#include "esp_log.h"

#include "sensor.h"
#include "HD44870.h"
#define LCD_ADDR 0x27
#define SDA_PIN  21
#define SCL_PIN  22
#define LCD_COLS 16
#define LCD_ROWS 2

#define SSID "Wokwi-GUEST"
#define PASSPHARSE ""

#define LED_0 GPIO_NUM_19
#define GPIO_OUTPUT_PIN_SEL (1ULL<<LED_0) 

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;
static const char *TAG="LOG_MQTT";

esp_mqtt_client_handle_t client;

void wifi_connect() {
    wifi_config_t cfg = {
        .sta = {
            .ssid = SSID,
            .password = PASSPHARSE,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg));
    ESP_ERROR_CHECK(esp_wifi_connect());
}

static esp_err_t event_handler(void *ctx, system_event_t *event) {
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void) {
    esp_log_level_set("wifi", ESP_LOG_NONE); // disable wifi driver logging
    tcpip_adapter_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void mqtt_event_handler(esp_mqtt_event_handle_t event){ //here esp_mqtt_event_handle_t is a struct which receieves struct event from mqtt app start funtion
    esp_mqtt_client_handle_t client = event->client; //making obj client of struct esp_mqtt_client_handle_t and assigning it the receieved event client
    if(event->event_id == MQTT_EVENT_CONNECTED) {
      ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
      printf("conectado... mqtt\n");
    }
    else if(event->event_id == MQTT_EVENT_DISCONNECTED)
    {
      ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED"); //if disconnected
    }
    else if(event->event_id == MQTT_EVENT_SUBSCRIBED)
    {
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");
    }
    else if(event->event_id == MQTT_EVENT_UNSUBSCRIBED) //when subscribed
    {
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED");
    }
    else if(event->event_id == MQTT_EVENT_DATA)//when unsubscribed
    {
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("Mensagem recebida -> %s\n", event->data);
    }
    else if(event->event_id == MQTT_EVENT_ERROR)//when any error
    {
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    }
}
static void mqtt_initialize(void) {/*Depending on your website or cloud there could be more parameters in mqtt_cfg.*/
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);
    const esp_mqtt_client_config_t mqtt_cfg = {
      .host= "test.mosquitto.org",//"broker.hivemq.com", //Uniform Resource Identifier includes path,protocol
      .port = 1883,
      .transport = MQTT_TRANSPORT_OVER_TCP,
      .event_handle=mqtt_event_handler //described above event handler
    };
    client=esp_mqtt_client_init(&mqtt_cfg); //sending struct as a parameter in init client function
    esp_mqtt_client_start(client); //starting the process
}

void blink(void *pvParameter) {
    while(1) {
        /* Blink off (output low) */
        gpio_set_level(LED_0, 0);
        vTaskDelay(1000 / portTICK_RATE_MS);
        /* Blink on (output high) */
        gpio_set_level(LED_0, 1);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void send_data(void *pvParameter){
  while(1){
    //Captura a temperatura
    float Temperature = read_temp();
    char str_data[20];
    sprintf(str_data,"%.2f",Temperature);
    
    //Mostrar no LCD
    LCD_setCursor(12, 1);
    char num[5];
    sprintf(num,"%.0f",Temperature);
    LCD_writeStr(num);
    LCD_writeStr("\xDF");
    LCD_writeStr("C");
    
    //Envio da Temperatura
    printf("Enviando -> '%s'\n",str_data);
    int msg_pub_id = esp_mqtt_client_publish(client, "topic/exemplo/matheussilva", str_data, 0, 0, 0);
    vTaskDelay( 8000 / portTICK_RATE_MS);
  }
}

void setup() {
  //Configura o LED
  gpio_config_t led_conf = {};
  led_conf.intr_type = GPIO_INTR_DISABLE;
  led_conf.mode = GPIO_MODE_OUTPUT;
  led_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
  led_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  led_conf.pull_up_en = GPIO_PULLDOWN_DISABLE;
  gpio_config(&led_conf);

  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_event_group = xEventGroupCreate();
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    initialise_wifi();
  xEventGroupWaitBits(wifi_event_group,CONNECTED_BIT,false,true,portMAX_DELAY);
  mqtt_initialize();

  LCD_init(LCD_ADDR, SDA_PIN, SCL_PIN, LCD_COLS, LCD_ROWS);

}

void app_main() {	
    setup();

    LCD_home();
    LCD_clearScreen();
    LCD_setCursor(0, 1);
    LCD_writeStr("Temperatura:");
    LCD_setCursor(0,0);
    LCD_writeStr("    Bom dia!");

    xTaskCreate(&blink,"blink led",1024,NULL,5,NULL);
    xTaskCreate(&send_data, "send data",4096,NULL,5,NULL);
    //int msg_sub_id = esp_mqtt_client_subscribe(client, "topic/exemplo2", 0);
    //int msg_pub_id = esp_mqtt_client_publish(client, "topic/exemplo", "datateste", 0, 0, 0);
}