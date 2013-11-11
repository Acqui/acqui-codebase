#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../sys/net.h"
#include "../../sys/gpio.h"
#include "../../dev/ad7746.h"

#define I2C_DEV      "/dev/i2c-1"
#define AD7746_ADDR  0x48
#define CAPDAC       0x00
#define N_SMPLS      10
#define MAX_SMPLS    250
#define GPIO_PIN     48
#define N_PORT       4646

/*----------------------------------------------------------------------------*/
_s32 main(int argc, char *argv[])
{
  _ad7746  *ad7746 = ad7746_new(I2C_DEV, AD7746_ADDR);
  _u8   capdac = CAPDAC;
  _u32  cap_hex_1, cap_hex_2;
  _u8   board = GPIO_PIN;
  _bool select = TRUE;
  _gpio *gpio = gpio_new();
  _gpio_board *gpio_board;
  float *cap_1;
  float *cap_2;
  _u8  n;
  _bool dual_chnl = FALSE;
  _bool board_once = FALSE;
  _net *net = net_new(N_PORT);
  _s32 opt;

  if (argc==2 && strcmp(argv[1], "--help") == 0) {
    printf("usage: server [--help] [-b] [-d] "
      "[-c CAPDAC] [-n SAMPLES]\n\n");
    printf("  --help,      display this help and exit\n");
    printf("  -c CAPDAC,   set hexdecimal CAPDAC value\n");
    printf("               [ 0x%02X = default ]\n", CAPDAC);
    printf("  -n SAMPLES,  number of samples\n");
    printf("               [ %d = default ]\n", N_SMPLS);
    printf("  -d,          select dual channel\n");
    printf("  -b           keep board selected\n");
    if (ad7746 != NULL) ad7746_delete(ad7746);
    return 0;
  }

  while ((opt = getopt(argc, argv, "bdc:n:")) != -1) {
    switch (opt) {
      case 'd':
        dual_chnl = TRUE;
        break;
      case 'c':
        sscanf(optarg, "%x", (_u32 *)&capdac);
        break;
      case 'n':
        sscanf(optarg, "%u", (_u32 *)&n_smpls);
        if (n_smpls == 0) n_smpls = N_SMPLS;
        if (n_smpls > MAX_SMPLS) n_smpls = MAX_SMPLS;
        break;
      case 'b':
        board_once = TRUE;
        break;
      case '?':
        ad7746_delete(ad7746);
        net_delete(net);
        return 1;
      default:
        ad7746_delete(ad7746);
        net_delete(net);
        return 1;
    }
  }

  gpio_board = gpio_new_board(gpio, &board, &select, 1);
  if (gpio_board == NULL) {
    gpio_delete(gpio);
    ad7746_delete(ad7746);
    net_delete(net);
    return 1;
  }
  if (board_once == FALSE) ad7746->gpio_board = gpio_board;
  else gpio_select_board(gpio_board);

  cap_1 = malloc(sizeof(float)*n_smpls);
  if (dual_chnl == TRUE) cap_2 = malloc(sizeof(float)*n_smpls);

  ad7746->cin = AD7746_CIN2;
  ad7746->exc = AD7746_EXCB;
  ad7746->excl = AD7746_EXCL_1_OVER_2;
  ad7746_write_setup(ad7746);

  ad7746_write_capdac(ad7746, capdac);
  capdac = ad7746_read_capdac(ad7746);
  printf("CAPDAC:     0x%02X = %.2f pF\n", capdac,
    ((float)capdac/0x7F)*21.0);

  if (ad7746_read_excerr(ad7746))
    printf("\e[1;31mEXCERR:     1\e[0m\n");
  else
    printf("EXCERR:     0\n");

  printf("\e[1;34mAwaiting TCP/IP connection.\e[0m\n");
  net_accept(net);
  printf("\e[1;34mStreaming data.\e[0m\n");

  do {
    for(n=0; n<N_SMPLS; n++) {
      ad7746->cin = AD7746_CIN2;
      ad7746->exc = AD7746_EXCB;
      ad7746->excl = AD7746_EXCL_1_OVER_2;
      ad7746_write_setup(ad7746);
      cap_hex_1 = ad7746_acquire(ad7746);
      cap_1[n] = ad7746_convert_to_capacitance(cap_hex_1);

      if (dual_chnl == TRUE) {
        ad7746->cin = AD7746_CIN1;
        ad7746->exc = AD7746_EXCA;
        ad7746->excl = AD7746_EXCL_1_OVER_2;
        ad7746_write_setup(ad7746);
        cap_hex_2 = ad7746_acquire(ad7746);
        cap_2[n] = ad7746_convert_to_capacitance(cap_hex_2);
      }

      if (dual_chnl == TRUE) {
        printf("\e[1;32mCAP_1: 0x%02X = %.6f pF\t\t"
          "CAP_2: 0x%02X = %.6f pF\e[0m\n", cap_hex_1, cap_1[n]*1E12,
          cap_hex_2, cap_2[n]*1E12);
      }
      else {
        printf("\e1;32mCAP: 0x%02X = %.6f pF\e[0m\n", cap_hex_1, cap_1[n]*1E12);
      }
    }
  } while (net_write(net, cap, sizeof(float)*N_SMPLS*2));

  printf("\e[1;34mTCP/IP connection lost...\e[0m\n");

  net_delete(net);
  ad7746_idle(ad7746);
  ad7746_delete(ad7746);
  gpio_delete_board(gpio, gpio_board);
  gpio_delete(gpio);
  return 0;
}
