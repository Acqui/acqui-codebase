#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "../../dev/ltc2493.h"

#define I2C_DEV       "/dev/i2c-1"
#define LTC2493_ADDR  0x24
#define N_SMPLS       10
#define MAX_SMPLS     250

/*----------------------------------------------------------------------------*/
_s32 main(int argc, char *argv[])
{
  _ltc2493 *ltc2493 = ltc2493_new(I2C_DEV, LTC2493_ADDR);
  _s32 conv;
  _s32 opt;
  _u32 n_smpls = N_SMPLS;
  float *vlt;
  float vlt_avg = 0.0;
  float vlt_std = 0.0;
  _u32 n;

  if (argc==2 && strcmp (argv[1],"--help") == 0) {
    printf("usage: meas-ltc2493 [--help] [-c CHANNEL] [-n SAMPLES]\n\n");
    printf("  --help,      display this help and exit\n");
    printf("  -c CHANNEL,  select measurement channel(s)\n");
    printf("               [ 0 = default, 1, 2, 3, 0-1, 1-0, 2-3, 3-2 ]\n");
    printf("  -n SAMPLES   number of samples\n");
    printf("               [ %d = default ]\n", N_SMPLS);
    if (ltc2493 != NULL) ltc2493_delete(ltc2493);
    return 0;
  }

  if (ltc2493 == NULL) return 1;
  ltc2493->chnl_sel = LTC2493_CHNL_0;

  while ((opt = getopt(argc, argv, "c:n:")) != -1) {
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
          fprintf(stderr,"Incorrect measurement channel selection.\n");
          ltc2493_delete(ltc2493);
          return 1;
        }
        break;
      case 'n':
        sscanf(optarg,"%u",(_u32 *)&n_smpls);
        if (n_smpls == 0) n_smpls = MAX_SMPLS;
        if (n_smpls > MAX_SMPLS) n_smpls = MAX_SMPLS;
        break;
      case '?':
        ltc2493_delete(ltc2493);
        return 1;
      default:
        ltc2493_delete(ltc2493);
        return 1;
    }
  }

  vlt = malloc(sizeof(float)*n_smpls);
  ltc2493_write_setup(ltc2493);

  for (n=0;n<n_smpls;n++) {
    conv = ltc2493_acquire(ltc2493);
    vlt[n] = ltc2493_convert_to_voltage(conv);
    vlt_avg += vlt[n]/n_smpls;

    printf("\e[1;32mVLT: %X = %f V\e[0m\n", conv, vlt[n]);
  }

  for (n=0;n<n_smpls;n++) {
    vlt_std += ((vlt[n]-vlt_avg)*(vlt[n]-vlt_avg))/n_smpls;
  }
  vlt_std = sqrtf(vlt_std);

  printf("\e[1;33mN %u\e[0m\n", n_smpls);
  printf("\e[1;33mVLT_AVG: %.3f V\e[0m\n", vlt_avg);
  printf("\e[1;33mVLT_STD: %.3f μV\e[0m\n", vlt_std*1E6);

  free(vlt);
  ltc2493_delete(ltc2493);
  return 0;
}
