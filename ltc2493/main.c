#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ltc2493.h"

#define I2C_DEV       "/dev/i2c-3"
#define LTC2493_ADDR  0x24

/*----------------------------------------------------------------------------*/
_s32 main(int argc, char *argv[])
{
  _ltc2493 *ltc2493 = ltc2493_new(I2C_DEV, LTC2493_ADDR);
  _s32 conv;

  ltc2493->chnl_sel = LTC2493_CHNL_3;
  ltc2493->out_rate = LTC2493_RATE_SINGLE;
  ltc2493->reject = LTC2493_REJECT_50HZ_60HZ;
  ltc2493_write_setup(ltc2493);

  conv = ltc2493_convert(ltc2493);

  printf("\e[1;32mADC: %X \e[0m\n", conv);
  printf("\e[1;33mVOLT: %f \e[0m\n", (conv/16777216.0)*1.65);

  ltc2493_delete(ltc2493);
  return 0;
}
