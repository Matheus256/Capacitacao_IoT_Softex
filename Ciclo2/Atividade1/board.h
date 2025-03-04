#ifndef BOARD_H
#define BOARD_H

#define SDA_PIN GPIO_NUM_19
#define SCL_PIN GPIO_NUM_18

//Para o cartão SSD
#define PIN_NUM_MISO  GPIO_NUM_12
#define PIN_NUM_MOSI  GPIO_NUM_13
#define PIN_NUM_CLK   GPIO_NUM_14
#define PIN_NUM_CS    GPIO_NUM_15

void board_init(void);

#endif