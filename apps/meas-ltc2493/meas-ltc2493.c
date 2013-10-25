#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../dev/ltc2493.h"

#define I2C_DEV       "/dev/i2c-1"
#define LTC2493_ADDR  0x24

/*----------------------------------------------------------------------------*/
_s32 main(int argc, char *argv[])
{
  _ltc2493 *ltc2493 = ltc2493_new(I2C_DEV, LTC2493_ADDR);
  _s32 conv;
  _s32 opt;


  if (argc==2 && strcmp (argv[1],"--help") == 0) {
    printf("usage: meas-ltc2493 [--help] [-c CHNL]\n\n");
    printf("  --help,   display this help\n");
    printf("  -c CHNL,  select channel\n");
    printf("            [0 = default, 1, 2, 3, 0-1, 1-0, 2-3, 3-2]\n");
    if (ltc2493 != NULL) ltc2493_delete(ltc2493);
    return 0;
  }

  if (ltc2493 == NULL) return 1;
  ltc2493->chnl_sel = LTC2493_CHNL_0;

  while ((opt = getopt(argc, argv, "c:")) != -1) {
    switch (opt) {
      case 'c':
        if (strcmp(optarg,"0") == 0) ltc2493->chnl_sel = LTC2493_CHNL_0;
        else if (strcmp(optarg,"1") == 0) ltc2493->chnl_sel = LTC2493_CHNL_1;
        else if (strcmp(optarg,"2") == 0) ltc2493->chnl_sel = LTC2493_CHNL_2;
        else if (strcmp(optarg,"3") == 0) ltc2493->chnl_sel = LTC2493_CHNL_3;
        else if (strcmp(optarg,"0-1") == 0)
          ltc2493->chnl_sel = LTC2493_CHNL_0_1;
        else if (strcmp(optarg,"1-0") == 0)
          ltc2493->chnl_sel = LTC2493_CHNL_1_0;
        else if (strcmp(optarg,"2-3") == 0)
          ltc2493->chnl_sel = LTC2493_CHNL_2_3;
        else if (strcmp(optarg,"3-2") == 0)
          ltc2493->chnl_sel = LTC2493_CHNL_3_2;
        else {
          fprintf(stderr,"Incorrect channel selection.\n");
          ltc2493_delete(ltc2493);
          return 1;
        }
        break;
      case '?':
        ltc2493_delete(ltc2493);
        return 1;
      default:
        ltc2493_delete(ltc2493);
        return 1;
    }
  }

  ltc2493_write_setup(ltc2493);

  conv = ltc2493_acquire(ltc2493);

  printf("\e[1;32mADC: %X \e[0m\n", conv);
  printf("\e[1;33mVOLT: %f \e[0m\n", (conv/16777216.0)*1.65);

  ltc2493_delete(ltc2493);
  return 0;
}
