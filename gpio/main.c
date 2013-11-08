#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpio.h"

#define GPIO_PIN  48

/*----------------------------------------------------------------------------*/
_s32 main(int argc, char *argv[])
{
  _gpio       *gpio;
  _gpio_board *gpio_board;
  _u8   board[1] = {48};
  _bool select[1] = {TRUE};

  gpio = gpio_new();
  if (gpio == NULL) return 1;

  gpio_board = gpio_new_board(gpio, board, select, 1);
  if (gpio_board == NULL) {
    gpio_delete(gpio);
    return 1;
  }

  gpio_select_board(gpio_board);
  gpio_deselect_board(gpio_board);


  gpio_delete_board(gpio, gpio_board);
  gpio_delete(gpio);
  return 0;
}
