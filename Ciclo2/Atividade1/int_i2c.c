#include "int_i2c.h"
#include "driver/i2c.h"
#include <freertos/FreeRTOS.h>
#include "esp_err.h"
#include "board.h"


void i2c_master_init(void) {
  	i2c_config_t i2c_config = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = SDA_PIN,
		.scl_io_num = SCL_PIN,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = 1000000
	};
	i2c_param_config(I2C_NUM_0, &i2c_config);
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

int i2c_write(uint8_t *data, uint8_t size) {
 	esp_err_t espRc;

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
  uint8_t i;
  for(i = 0; i < size; i++) {
    i2c_master_write_byte(cmd, data[i], true);
  }

	i2c_master_stop(cmd);

	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);

	if (espRc == ESP_OK) {
		return 0;
	}
	else {
		return -1;
	}
}