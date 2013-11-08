#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpio.h"

#define GPIO_MAX_PINS  129

/*----------------------------------------------------------------------------*/
#define THIS ((_gpio_priv *)gpio->priv)

typedef enum {GPIO_CFG_NONE, GPIO_CFG_IN,
              GPIO_CFG_OUT, GPIO_CFG_BOARD} _gpio_config;

struct __gpio_board {
  _u8   *gpio_pin_nr;
  _bool *select;
  _u8   size;
};

static void gpio_export(_gpio *gpio, _u8 gpio_pin_nr);
static void gpio_unexport(_gpio *gpio, _u8 gpio_pin_nr);
static void gpio_write_direction(_gpio *gpio, _u8 gpio_pin_nr,
              _gpio_direction gpio_direction);
static void gpio_write_value(_u8 gpio_pin_nr, _bool val);

typedef struct {
  _u8           gpio_allocation[GPIO_MAX_PINS];
  _gpio_config  gpio_config[GPIO_MAX_PINS];
} _gpio_priv;

/*----------------------------------------------------------------------------*/
_gpio* gpio_new(void)
{
  _gpio *gpio;

  gpio = malloc(sizeof(_gpio));
  gpio->priv = malloc(sizeof(_gpio_priv));

  memset(THIS->gpio_allocation, 0, sizeof(_bool)*GPIO_MAX_PINS);
  memset(THIS->gpio_config, GPIO_CFG_NONE, sizeof(_bool)*GPIO_MAX_PINS);

  gpio->usleep = 1000000;

  return gpio;
}

/*----------------------------------------------------------------------------*/
void gpio_delete(_gpio *gpio)
{
  free(gpio->priv);
  free(gpio);
}

/*----------------------------------------------------------------------------*/
_gpio_board* gpio_new_board(_gpio *gpio, _u8 *gpio_pin_nr,
  _bool *select, _u8 size)
{
  _gpio_board *gpio_board;
  _u8 n;

  for (n=0; n<size; n++) {
    if (THIS->gpio_config[gpio_pin_nr[n]] == GPIO_CFG_IN ||
        THIS->gpio_config[gpio_pin_nr[n]] == GPIO_CFG_OUT)
      return NULL;
  }

  gpio_board = malloc(sizeof(_gpio_board));

  gpio_board->gpio_pin_nr = malloc(sizeof(_u8)*size);
  gpio_board->select = malloc(sizeof(_bool)*size);
  gpio_board->size = size;

  memcpy(gpio_board->gpio_pin_nr, gpio_pin_nr, sizeof(_u8)*size);
  memcpy(gpio_board->select, select, sizeof(_bool)*size);

  for (n=0; n<size; n++) {
    THIS->gpio_config[gpio_pin_nr[n]] = GPIO_CFG_BOARD;
    THIS->gpio_allocation[gpio_pin_nr[n]] += 1;
    gpio_export(gpio, gpio_pin_nr[n]);
    gpio_write_direction(gpio, gpio_pin_nr[n], GPIO_OUT);
  }

  return gpio_board;
}

/*----------------------------------------------------------------------------*/
void gpio_delete_board(_gpio *gpio, _gpio_board *gpio_board)
{
  _u8 n;

  for (n=0; n < gpio_board->size; n++) {
    if (THIS->gpio_allocation[gpio_board->gpio_pin_nr[n]] > 0)
      THIS->gpio_allocation[gpio_board->gpio_pin_nr[n]] -= 1;
    if (THIS->gpio_allocation[gpio_board->gpio_pin_nr[n]] == 0 ) {
      THIS->gpio_config[gpio_board->gpio_pin_nr[n]] = GPIO_CFG_NONE;
      /*gpio_unexport(gpio, gpio_board->gpio_pin_nr[n]);*/
    }
  }

  free(gpio_board->gpio_pin_nr);
  free(gpio_board->select);
  free(gpio_board);
}

/*----------------------------------------------------------------------------*/
void gpio_select_board(_gpio_board *gpio_board)
{
  _u8 n;

  for (n=0; n < gpio_board->size; n++) {
    gpio_write_value(gpio_board->gpio_pin_nr[n], gpio_board->select[n]);
  }
}

/*----------------------------------------------------------------------------*/
void gpio_deselect_board(_gpio_board *gpio_board)
{
  _u8 n;

  for (n=0; n < gpio_board->size; n++) {
    gpio_write_value(gpio_board->gpio_pin_nr[n], FALSE);
  }
}

/*----------------------------------------------------------------------------*/
_gpio_pin* gpio_new_pin(_gpio *gpio, _u8 gpio_pin_nr,
  _gpio_direction gpio_direction)
{
  _gpio_pin *gpio_pin;

  if (!(THIS->gpio_config[gpio_pin_nr] == GPIO_CFG_NONE ||
       (THIS->gpio_config[gpio_pin_nr] == GPIO_CFG_IN &&
         gpio_direction == GPIO_IN ) ||
       (THIS->gpio_config[gpio_pin_nr] == GPIO_CFG_IN &&
        gpio_direction == GPIO_OUT)))
    return NULL;

  gpio_pin = malloc(sizeof(_gpio_pin));
  *gpio_pin = gpio_pin_nr;

  if (gpio_direction == GPIO_IN) {
    THIS->gpio_config[gpio_pin_nr] = GPIO_CFG_IN;
    gpio_write_direction(gpio, gpio_pin_nr, GPIO_IN);
  }
  if (gpio_direction == GPIO_OUT) {
    THIS->gpio_config[gpio_pin_nr] = GPIO_CFG_OUT;
    gpio_write_direction(gpio, gpio_pin_nr, GPIO_OUT);
  }
  THIS->gpio_allocation[gpio_pin_nr] += 1;
  gpio_export(gpio, gpio_pin_nr);

  return gpio_pin;
}

/*----------------------------------------------------------------------------*/
void gpio_write_pin(_gpio_pin *gpio_pin, _bool val)
{
  gpio_write_value(*gpio_pin, val);
}

/*----------------------------------------------------------------------------*/
void gpio_delete_pin(_gpio *gpio,_gpio_pin *gpio_pin)
{
  if (THIS->gpio_allocation[*gpio_pin] > 0)
    THIS->gpio_allocation[*gpio_pin] -= 1;
  if (THIS->gpio_allocation[*gpio_pin] == 0 ) {
    THIS->gpio_config[*gpio_pin] = GPIO_CFG_NONE;
    /*gpio_unexport(gpio, *gpio_pin);*/
  }

  free(gpio_pin);
}

/*----------------------------------------------------------------------------*/
static void gpio_export(_gpio *gpio, _u8 gpio_pin_nr)
{
  _s32 fd;
  char buf[50];

  fd = open("/sys/class/gpio/export", O_WRONLY);
  sprintf(buf, "%u", gpio_pin_nr);
  write(fd, buf, strlen(buf));
  close(fd);
  usleep(gpio->usleep);
}

/*----------------------------------------------------------------------------*/
static void gpio_unexport(_gpio *gpio, _u8 gpio_pin_nr)
{
  _s32 fd;
  char buf[50];

  fd = open("/sys/class/gpio/unexport", O_WRONLY);
  sprintf(buf, "%u", gpio_pin_nr);
  write(fd, buf, strlen(buf));
  close(fd);
  usleep(gpio->usleep);
}

/*----------------------------------------------------------------------------*/
static void gpio_write_direction(_gpio *gpio, _u8 gpio_pin_nr,
  _gpio_direction gpio_direction)
{
  _s32 fd;
  char buf[50];

  sprintf(buf, "/sys/class/gpio/gpio%d/direction", gpio_pin_nr);
  fd = open(buf, O_WRONLY);
  if (gpio_direction == GPIO_OUT) write(fd, "out", 3);
  else write(fd, "in", 2);
  close(fd);
  usleep(gpio->usleep);
}

/*----------------------------------------------------------------------------*/
static void gpio_write_value(_u8 gpio_pin_nr, _bool val)
{
  _s32 fd;
  char buf[50];

  sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio_pin_nr);
  fd = open(buf, O_WRONLY);
  if (val == TRUE) write(fd, "1", 1);
  if (val == FALSE) write(fd, "0", 1);
  close(fd);
}
