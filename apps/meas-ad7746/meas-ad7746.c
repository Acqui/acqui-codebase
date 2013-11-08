#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "../../dev/ad7746.h"

#define I2C_DEV      "/dev/i2c-1"
#define AD7746_ADDR  0x48
#define CAPDAC       0x00
#define N_SMPLS      10
#define MAX_SMPLS    250

/*----------------------------------------------------------------------------*/
_s32 main(int argc, char *argv[])
{
  _ad7746 *ad7746 = ad7746_new(I2C_DEV, AD7746_ADDR);
  _u8  capdac = CAPDAC;
  _u32 n_smpls = N_SMPLS;
  _u32 cap_hex_1, cap_hex_2;
  float *cap_1;
  float *cap_2;
  float cap_avg_1 = 0.0;
  float cap_avg_2 = 0.0;
  float cap_std_1 = 0.0;
  float cap_std_2 = 0.0;
  _u32  n;
  _bool dual_chnl = FALSE;
  _s32 opt;


  if (argc==2 && strcmp (argv[1],"--help") == 0) {
    printf("usage: meas-ad7746 [--help] [-d] [-c CAPDAC] [-n SAMPLES]\n\n");
    printf("  --help,      display this help and exit\n");
    printf("  -c CAPDAC,   set hexdecimal CAPDAC value\n");
    printf("               [ 0x%02X = default ]\n", CAPDAC);
    printf("  -n SAMPLES,  number of samples\n");
    printf("               [ %d = default ]\n", N_SMPLS);
    printf("  -d           select dual channel\n");
    if (ad7746 != NULL) ad7746_delete(ad7746);
    return 0;
  }

  if (ad7746 == NULL) return 1;

  while ((opt = getopt(argc, argv, "dc:n:")) != -1) {
    switch (opt) {
      case 'd':
        dual_chnl = TRUE;
        break;
      case 'c':
        sscanf(optarg,"%x",(_u32 *)&capdac);
        break;
      case 'n':
        sscanf(optarg,"%u",(_u32 *)&n_smpls);
        if (n_smpls == 0) n_smpls = N_SMPLS;
        if (n_smpls > MAX_SMPLS) n_smpls = MAX_SMPLS;
        break;
      case '?':
        ad7746_delete(ad7746);
        return 1;
      default:
        ad7746_delete(ad7746);
        return 1;
    }
  }

  cap_1 = malloc(sizeof(float)*n_smpls);
  if (dual_chnl == TRUE) cap_2 = malloc(sizeof(float)*n_smpls);

  ad7746->cin = AD7746_CIN2;
  ad7746->exc = AD7746_EXCB;
  ad7746->excl = AD7746_EXCL_1_OVER_2;
  ad7746_write_setup(ad7746);

  ad7746_write_capdac(ad7746, capdac);
/*  cap_offset = ad7746_calibrate(ad7746, capdac);
    printf("CAP_OFFSET: 0x%04X = %.2f pF\n",
      cap_offset, ((float)cap_offset-0x8000)/0xFFFF*2.048); */

  capdac = ad7746_read_capdac(ad7746);
  printf("CAPDAC:     0x%02X = %.2f pF\n", capdac,
    ((float)capdac/0x7F)*21.0);

/*  printf("\e[1;32mCAP_COMM:   %.2f pF\e[0m\n",
     ((float)capdac/0x7F)*21.0+
     ((float)cap_offset-0x8000)/0xFFFF*2.048); */

  if (ad7746_read_excerr(ad7746))
    printf("\e[1;31mEXCERR:     1\e[0m\n");
  else
    printf("EXCERR:     0\n");

  for (n=0;n<n_smpls;n++) {
    ad7746->cin = AD7746_CIN2;
    ad7746->exc = AD7746_EXCB;
    ad7746->excl = AD7746_EXCL_1_OVER_2;
    ad7746_write_setup(ad7746);
    cap_hex_1 = ad7746_acquire(ad7746);
    cap_1[n] = ad7746_convert_to_capacitance(cap_hex_1);
    cap_avg_1 += cap_1[n]/n_smpls;

    if (dual_chnl == TRUE) {
      ad7746->cin = AD7746_CIN1;
      ad7746->exc = AD7746_EXCA;
      ad7746->excl = AD7746_EXCL_1_OVER_2;
      ad7746_write_setup(ad7746);
      cap_hex_2 = ad7746_acquire(ad7746);
      cap_2[n] = ad7746_convert_to_capacitance(cap_hex_2);
      cap_avg_2 += cap_2[n]/n_smpls;
    }

    if (dual_chnl == TRUE) {
      printf("\e[1;32mCAP_1: 0x%02X = %.6f pF\t\t"
        "CAP_2: 0x%02X = %.6f pF\e[0m\n", cap_hex_1, cap_1[n]*1E12,
        cap_hex_2, cap_2[n]*1E12);
    }
    else {
      printf("\e[1;32mCAP: 0x%02X = %.6f pF\e[0m\n", cap_hex_1, cap_1[n]*1E12);
    }
  }

  for (n=0;n<n_smpls;n++) {
    cap_std_1 += ((cap_1[n]-cap_avg_1)*(cap_1[n]-cap_avg_1))/n_smpls;
    if (dual_chnl == TRUE) {
      cap_std_2 += ((cap_2[n]-cap_avg_2)*(cap_2[n]-cap_avg_2))/n_smpls;
    }
  }
  cap_std_1 = sqrtf(cap_std_1);
  if (dual_chnl == TRUE) {
    cap_std_2 = sqrtf(cap_std_2);
  }

  printf("\e[1;33mN: %u\e[0m\n", n_smpls);
  if (dual_chnl == TRUE) {
    printf("\e[1;33mCAP_AVG_1: %.6f pF\t\t"
      "CAP_AVG_2: %.6f pF\e[0m\n", cap_avg_1*1E12, cap_avg_2*1E12);
    printf("\e[1;33mCAP_STD_1: %.3f aF\t\t"
     "CAP_STD_2: %.3f aF\e[0m\n", cap_std_1*1E18, cap_std_2*1E18);
  }
  else {
    printf("\e[1;33mCAP_AVG: %.6f pF\e[0m\n", cap_avg_1*1E12);
    printf("\e[1;33mCAP_STD: %.3f aF\e[0m\n", cap_std_1*1E18);
  }

  free(cap_1);
  if (dual_chnl == TRUE) free(cap_2);

  ad7746_idle(ad7746);
  ad7746_delete(ad7746);
  return 0;
}
