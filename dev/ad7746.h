#ifndef _AD7746_H
#define _AD7746_H

#include "../sys/types.h"

typedef struct {
  enum {AD7746_CIN1, AD7746_CIN2} cin;
  enum {AD7746_EXCA, AD7746_EXCB} exc;
  enum {AD7746_EXCL_1_OVER_8, AD7746_EXCL_1_OVER_4,
        AD7746_EXCL_3_OVER_8, AD7746_EXCL_1_OVER_2} excl;
  _bool cap_diff;
  _priv *priv;
} _ad7746;

_ad7746* ad7746_new(char *dev, _u8 addr);
void     ad7746_delete(_ad7746 *ad7746);
void     ad7746_write_setup(_ad7746 *ad7746);
void     ad7746_write_capdac(_ad7746 *ad7746, _u8 capdac);
_u16     ad7746_calibrate(_ad7746 *ad7746, _u8 capdac);
_u32     ad7746_convert(_ad7746 *ad7746);
_bool    ad7746_read_excerr(_ad7746 *ad7746);
_u8      ad7746_read_capdac(_ad7746 *ad7746);

#endif
