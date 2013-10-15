#ifndef _LTC2493_H
#define _LTC2493_H

#include "../sys/types.h"

typedef struct {
  enum {LTC2493_CHNL_0_1, LTC2493_CHNL_2_3,
        LTC2493_CHNL_1_0, LTC2493_CHNL_3_2,
        LTC2493_CHNL_0, LTC2493_CHNL_2,
        LTC2493_CHNL_1, LTC2493_CHNL_3,
        LTC2493_CHNL_TEMP} chnl_sel;
  enum {LTC2493_RATE_SINGLE,
        LTC2493_RATE_DOUBLE} out_rate;
  enum {LTC2493_REJECT_50HZ,
        LTC2493_REJECT_60HZ,
        LTC2493_REJECT_50HZ_60HZ} reject;
  _priv *priv;
} _ltc2493;

_ltc2493* ltc2493_new(char *dev, _u8 addr);
void      ltc2493_delete(_ltc2493 *ltc2493);
void      ltc2493_write_setup(_ltc2493 *ltc2493);
_s32      ltc2493_convert(_ltc2493 *ltc2493);

#endif
