#include "board.h"

void board_init() {
  i2c_master_init();
  ssd1306_init();
  ssd_card_init();
}