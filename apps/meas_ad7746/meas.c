#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../../dev/ad7746.h"

#define I2C_DEV      "/dev/i2c-3"
#define AD7746_ADDR  0x48
#define CAPDAC       0x00
#define N_SMPLS      200

/*----------------------------------------------------------------------------*/
_s32 main(int argc, char *argv[])
{
  _s32 fd;
  _u8  capdac = CAPDAC;
  _u32 cap_hex;
  float cap[N_SMPLS];
  float cap_avg = 0.0;
  float cap_std = 0.0;
  _u8  n;

  if (argc==2) sscanf(argv[1],"%x",(_u32 *)&capdac);

  fd = ad7746_init(I2C_DEV, AD7746_ADDR);

  ad7746_write_capdac(fd, capdac);
//  cap_offset = ad7746_calibrate(fd, capdac);
//  printf("CAP_OFFSET: 0x%04X = %.2f pF\n",
//    cap_offset, ((float)cap_offset-0x8000)/0xFFFF*2.048);

  capdac = ad7746_read_capdac(fd);
  printf("CAPDAC:     0x%02X = %.2f pF\n", capdac,
    ((float)capdac/0x7F)*21.0);

//  printf("\e[1;32mCAP_COMM:   %.2f pF\e[0m\n",
//   ((float)capdac/0x7F)*21.0+
//   ((float)cap_offset-0x8000)/0xFFFF*2.048);

  if (ad7746_read_excerr(fd))
    printf("\e[1;31mEXCERR:     1\e[0m\n");
  else
    printf("EXCERR:     0\n");

  for(n=0;n<N_SMPLS;n++) {
    cap_hex = ad7746_convert(fd);
    cap[n] = (((float)cap_hex-0x800000)/0xFFFFFF*8.192);
    cap_avg += cap[n]/N_SMPLS;
    printf("\e[1;32mCAP: 0x%02X = %.6f pF\e[0m\n", cap_hex, cap[n]);
  }

  for(n=0;n<N_SMPLS;n++) {
    cap_std += ((cap[n]-cap_avg)*(cap[n]-cap_avg))/N_SMPLS;
  }
  cap_std = sqrtf(cap_std);

  printf("\e[1;33mN: %u\e[0m\n", N_SMPLS);
  printf("\e[1;33mCAP_AVG: %.6f pF\e[0m\n", cap_avg);
  printf("\e[1;33mCAP_STD: %.3f aF\e[0m\n", cap_std*1E6);

  ad7746_finalize(fd);
  return 0;
}
