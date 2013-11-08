#ifndef _GPIO_H
#define _GPIO_H

#include "../sys/types.h"

typedef struct __gpio_board _gpio_board;
typedef _u8 _gpio_pin;
typedef enum {GPIO_IN, GPIO_OUT} _gpio_direction;

typedef struct {
 _u32 usleep;
 _priv *priv;
} _gpio;

_gpio*        gpio_new(void);
void          gpio_delete(_gpio *gpio);
_gpio_board*  gpio_new_board(_gpio *gpio, _u8 *gpio_pin_nr,
                _bool *select, _u8 size);
void          gpio_delete_board(_gpio *gpio, _gpio_board *gpio_board);
void          gpio_select_board(_gpio_board *gpio_board);
void          gpio_deselect_board(_gpio_board *gpio_board);
_gpio_pin*    gpio_new_pin(_gpio *gpio, _u8 gpio_pin_nr,
                _gpio_direction gpio_direction);
void          gpio_delete_pin(_gpio *gpio, _gpio_pin *gpio_pin);
void          gpio_write_pin(_gpio_pin *gpio_pin, _bool val);
/*_bool         gpio_read_pin(_gpio_pin *gpio_pin);*/

#endif
