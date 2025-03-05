#ifndef INT_I2C_H
#define INT_I2C_H

#include <stdint.h>

void i2c_master_init(void);
int i2c_write(uint8_t *data, uint8_t size);

#endif
