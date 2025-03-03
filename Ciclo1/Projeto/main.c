#include <stdio.h>
#include <string.h>    //strlen
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "math.h"

#include "freertos/semphr.h"
SemaphoreHandle_t xSemaphore = NULL;
static TaskHandle_t task_tcp_client = NULL;

#include "HD44870.h"
#define LCD_ADDR 0x27
#define SDA_PIN  21
#define SCL_PIN  22
#define LCD_COLS 16
#define LCD_ROWS 2

#define SSID "Wokwi-GUEST"
#define PASSPHARSE ""
#define TCPServerIP "159.203.79.141"
#define PORT 50000

#define LED_0 GPIO_NUM_19
#define GPIO_OUTPUT_PIN_SEL (1ULL<<LED_0) 

//Para o sensor de temperatura
#define NTC_PIN GPIO_NUM_32
//const float BETA = 3950;
float Temperature;

static const char *id = "43567130870";

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;
static const char *TAG="tcp_client";


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

void tcp_client(void *pvParam){
    printf("tcp_client task started \n");
    char rx_buffer[128];
    char tx_buffer[128];
    char host_ip[] = TCPServerIP;
    int addr_family = 0;
    int ip_protocol = 0;
    struct sockaddr_in tcpServerAddr;
    tcpServerAddr.sin_addr.s_addr = inet_addr(TCPServerIP);
    tcpServerAddr.sin_family = AF_INET;
    tcpServerAddr.sin_port = htons(PORT);
    int s, r;
    
    xEventGroupWaitBits(wifi_event_group,CONNECTED_BIT,false,true,portMAX_DELAY);

    int sock =  socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return ;
    }
    ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, PORT);

    int err = connect(sock, (struct sockaddr *)&tcpServerAddr, sizeof(tcpServerAddr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        return;
    }
    
    ESP_LOGI(TAG, "Successfully connected");

    err = send(sock, id, strlen(id), 0);
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }
    else {
      int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
      // Error occurred during receiving
      if (len < 0) {
          ESP_LOGE(TAG, "recv failed: errno %d", errno);
      }
      // Data received
      else {
          rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
          if(strcmp(rx_buffer,"ok") == 0){
            printf("Login realizado com sucesso\n");
            while(1) {
              if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE){
                printf("Enviando %.2f\n",Temperature);
                sprintf(tx_buffer,"%.2f", Temperature);
                int err = send(sock, tx_buffer, strlen(tx_buffer), 0);
                if (err < 0) {
                  ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                }
                else {
                  printf("Envio realizado com sucesso\n");
                }
              }
              //else {
              //  vTaskSuspend(NULL);
              //}
              //vTaskDelay(5000 / portTICK_PERIOD_MS);
            }
          }
          else {
            printf("Falha no login");
          }
      }
    }
    if (sock != -1) {
        ESP_LOGE(TAG, "Shutting down socket...");
        shutdown(sock, 0);
        close(sock);
    }
    vTaskDelete(NULL);
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

void sensor(void *pvParameters){
  while(1) {
    if (xSemaphoreGive(xSemaphore) == pdTRUE) {
      int analogValue = analogRead(NTC_PIN);
      Temperature = 1 / (log(1 / (4095.0 / analogValue - 1)) / 3950.0 + 1.0 / 298.15) - 273.15;
      printf("Temperatura: %.2f °C\n",Temperature);
      
      char num[20];
      sprintf(num, "%.0f", Temperature);
      LCD_setCursor(12, 0);
      LCD_writeStr(num);
      //LCD_setCursor(14,0);
      LCD_writeStr("\xDF");
      LCD_writeStr("C");

      //vTaskResume(task_tcp_client);
    }
    LCD_setCursor(12,1);
    LCD_writeStr(".");
    vTaskDelay(500 / portTICK_RATE_MS);
    LCD_writeStr(".");
    vTaskDelay(500 / portTICK_RATE_MS);
    LCD_writeStr(".");
    LCD_setCursor(12,1);
    LCD_writeStr(" ");
    vTaskDelay(500 / portTICK_RATE_MS);
    LCD_writeStr(" ");
    vTaskDelay(500 / portTICK_RATE_MS);
    LCD_writeStr(" ");
    vTaskDelay(5000 / portTICK_RATE_MS);
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
}

void app_main() {	
    setup();
    xSemaphore = xSemaphoreCreateBinary();

    LCD_init(LCD_ADDR, SDA_PIN, SCL_PIN, LCD_COLS, LCD_ROWS);
    LCD_home();
    LCD_clearScreen();
    LCD_setCursor(0, 0);
    LCD_writeStr("Temperatura:");
    LCD_setCursor(0,1);
    LCD_writeStr(" Funcionando");

    xTaskCreate(&blink,"blink led",1024,NULL,5,NULL);
    xTaskCreate(&tcp_client,"tcp_client",4096,NULL,5,&task_tcp_client);
    xTaskCreate(&sensor,"sensor",4096,NULL,5,NULL);
}