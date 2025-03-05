#include "sd1306.h"
#include <stdint.h>
#include "driver/i2c.h"
#include "esp_err.h"
#include "font8x8_basic.h"
#include "esp_log.h"

#define tag "SSD1306"

void ssd1306_init(void) {
  uint8_t buffer[7];
  buffer[0] = (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE;
  buffer[1] = OLED_CONTROL_BYTE_CMD_STREAM;
  buffer[2] = OLED_CMD_SET_CHARGE_PUMP;
  buffer[3] = 0x14;
  buffer[4] = OLED_CMD_SET_SEGMENT_REMAP;
  buffer[5] = OLED_CMD_SET_COM_SCAN_MODE;
  buffer[6] = OLED_CMD_DISPLAY_ON;

  int ret = i2c_write(buffer,7);

  if (ret == 0) {
		ESP_LOGI(tag, "OLED configured successfully");
	}
	else {
		ESP_LOGE(tag, "OLED configuration failed");
	}
  
}

void ssd1306_display_text(char *text) {
  uint8_t cur_page = 0;
  uint8_t buffer[10];
  buffer[0] = (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE;
  buffer[1] = OLED_CONTROL_BYTE_CMD_STREAM;
  buffer[2] = 0x00; // reset column
  buffer[3] = 0x10;
  buffer[4] = 0xB0 | cur_page; // reset page
  i2c_write(buffer, 5);
  
  uint8_t text_len = strlen(text);

	for (uint8_t i = 0; i < text_len; i++) {
		if (text[i] == '\n') {
      buffer[0] = (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE;
      buffer[1] = OLED_CONTROL_BYTE_CMD_STREAM;
      buffer[2] = 0x00;
      buffer[3] = 0x10;
      buffer[4] = (0xB0 | ++cur_page);
      i2c_write(buffer, 5);
    }
    else {
      buffer[0] = (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE;
      buffer[1] = OLED_CONTROL_BYTE_DATA_STREAM;
      for(uint8_t j = 2; j < 10; j++) {
        buffer[j] = font8x8_basic_tr[(uint8_t)text[i]][j-2];
      }
			i2c_write(buffer, 10);
    }
  }
}
