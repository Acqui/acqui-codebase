#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../sys/net.h"
#include "../../sys/gpio.h"
#include "../../dev/ad7746.h"
#include "../../dev/ltc2493.h"

#define I2C_DEV      "/dev/i2c-1"
#define AD7746_ADDR  0x48
#define LTC2493_ADDR 0x24
#define CAPDAC       0x00
#define N_SMPLS      10
#define MAX_SMPLS    250
#define GPIO_PIN     48
#define N_PORT       4646

typedef struct {
  _bool cap_1;
  _bool cap_2;
  _bool vlt_0;
  _bool vlt_1;
  _bool vlt_2;
  _bool vlt_3;
  _bool vlt_0_1;
  _bool vlt_1_0;
  _bool vlt_2_3;
  _bool vlt_3_2;
} _chnl_select;

/*----------------------------------------------------------------------------*/
_s32 main(int argc, char *argv[])
{
  _ad7746  *ad7746 = ad7746_new(I2C_DEV, AD7746_ADDR);
  _ltc2493 *ltc2493 = ltc2493_new(I2C_DEV, LTC2493_ADDR);
  _net *net = net_new(N_PORT);
  _gpio *gpio = gpio_new();
  _gpio_board *gpio_board;
  _u8   capdac = CAPDAC;
  _u32  n_smpls = N_SMPLS;
  _u32  cap_hex;
  _s32  vlt_hex;
  _u8   board = GPIO_PIN;
  _bool select = TRUE;
  float *packet;
  _u8  n, m, n_chnls = 0;
  char *tok;
  _chnl_select chnl_select;
  _bool board_once = FALSE;
  _bool verbose = FALSE;
  _s32 opt;

  memset(&chnl_select, FALSE, sizeof(_chnl_select));

  if ( (argc==2 && strcmp(argv[1], "--help") == 0) || argc==1) {
    printf("usage: server [--help] [-c [cap-1,cap-2]]\n");
    printf("              [-r [0,1,2,3,0-1,1-0,2-3,3-2]] [-d CAPDAC]\n");
    printf("              [-n SAMPLES] [-b] [-v]\n\n");
    printf("  --help,                       display this help and exit\n");
    printf("  -c [cap-1,cap-2],             AD7746 channel selection\n");
    printf("  -r [0,1,2,3,0-1,1-0,2-3,3-2], LTC2493 channel selection\n");
    printf("  -d CAPDAC,                    set hexdecimal CAPDAC value\n");
    printf("                                [ 0x%02X = default ]\n", CAPDAC);
    printf("  -n SAMPLES,         number of samples per TCP/IP packet\n");
    printf("                      [ %d = default ]\n", N_SMPLS);
    printf("  -b                  keep board selected\n");
    printf("  -v                  verbose\n");
    if (ad7746 != NULL) ad7746_delete(ad7746);
    if (ltc2493 != NULL) ltc2493_delete(ltc2493);
    gpio_delete(gpio);
    net_delete(net);
    if (argc==1) return 1;
    return 0;
  }

  if (ad7746 == NULL|| ltc2493 == NULL) {
    if (ad7746 != NULL) ad7746_delete(ad7746);
    if (ltc2493 != NULL) ltc2493_delete(ltc2493);
    gpio_delete(gpio);
    net_delete(net);
    return 1;
  }

  while ((opt = getopt(argc, argv, "vbd:r:c:n:")) != -1) {
    switch (opt) {
      case 'c':
        tok = strtok(optarg,",");
        while (tok != NULL) {
          if (strcmp(tok,"cap-1") == 0) {chnl_select.cap_1 = TRUE; n_chnls++;}
          if (strcmp(tok,"cap-2") == 0) {chnl_select.cap_2 = TRUE; n_chnls++;}
          tok = strtok(NULL,",");
        }
        break;
      case 'r':
        tok = strtok(optarg,",");
        while (tok != NULL) {
          if (strcmp(tok,"0") == 0) {chnl_select.vlt_0 = TRUE; n_chnls++;}
          if (strcmp(tok,"1") == 0) {chnl_select.vlt_1 = TRUE; n_chnls++;}
          if (strcmp(tok,"2") == 0) {chnl_select.vlt_2 = TRUE; n_chnls++;}
          if (strcmp(tok,"3") == 0) {chnl_select.vlt_3 = TRUE; n_chnls++;}
          if (strcmp(tok,"0-1") == 0) {chnl_select.vlt_0_1 = TRUE; n_chnls++;}
          if (strcmp(tok,"1-0") == 0) {chnl_select.vlt_1_0 = TRUE; n_chnls++;}
          if (strcmp(tok,"2-3") == 0) {chnl_select.vlt_2_3 = TRUE; n_chnls++;}
          if (strcmp(tok,"3-2") == 0) {chnl_select.vlt_3_2 = TRUE; n_chnls++;}
          tok = strtok(NULL,",");
        }
        break;
      case 'd':
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
      case 'v':
        verbose = TRUE;
        break;
      case '?':
        ad7746_delete(ad7746);
        ltc2493_delete(ltc2493);
        net_delete(net);
        return 1;
      default:
        ad7746_delete(ad7746);
        ltc2493_delete(ltc2493);
        net_delete(net);
        return 1;
    }
  }

  gpio_board = gpio_new_board(gpio, &board, &select, 1);
  if (gpio_board == NULL) {
    gpio_delete(gpio);
    ad7746_delete(ad7746);
    ltc2493_delete(ltc2493);
    net_delete(net);
    return 1;
  }
  if (board_once == FALSE) {
    ad7746->gpio_board = gpio_board;
    ltc2493->gpio_board = gpio_board;
  }
  else gpio_select_board(gpio_board);

  packet = malloc(sizeof(float)*n_smpls*n_chnls);

  if (chnl_select.cap_1 || chnl_select.cap_2) {
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
  }

  printf("\e[1;34mAwaiting TCP/IP connection.\e[0m\n");
  net_accept(net);
  printf("\e[1;34mStreaming data.\e[0m\n");

  do {
    for(n=0; n<n_smpls; n++) {
      m = 0;
      if (chnl_select.cap_1 == TRUE) {
        ad7746->cin = AD7746_CIN1;
        ad7746->exc = AD7746_EXCA;
        ad7746->excl = AD7746_EXCL_1_OVER_2;
        ad7746_write_setup(ad7746);
        cap_hex = ad7746_acquire(ad7746);
        *(packet + (n*n_chnls + m)) =
          net_float_swap(ad7746_convert_to_capacitance(cap_hex));
        m++;
        if (verbose) printf("\e[1;32mCAP-1: 0x%02X = %.6f pF\e[0m", cap_hex,
          ad7746_convert_to_capacitance(cap_hex)*1E12);
      }
      if (chnl_select.cap_1 && chnl_select.cap_2 && verbose) printf("\t\t");
      if (chnl_select.cap_2 == TRUE) {
        ad7746->cin = AD7746_CIN2;
        ad7746->exc = AD7746_EXCB;
        ad7746->excl = AD7746_EXCL_1_OVER_2;
        ad7746_write_setup(ad7746);
        cap_hex = ad7746_acquire(ad7746);
        *(packet + (n*n_chnls + m)) =
          net_float_swap(ad7746_convert_to_capacitance(cap_hex));
        m++;
        if (verbose) printf("\e[1;32mCAP-2: 0x%02X = %.6f pF\e[0m", cap_hex,
          ad7746_convert_to_capacitance(cap_hex)*1E12);
      }
      if ((chnl_select.cap_1 || chnl_select.cap_2) && verbose) printf("\n");
      if (chnl_select.vlt_0 == TRUE) {
        ltc2493->chnl_sel = LTC2493_CHNL_0;
        ltc2493_write_setup(ltc2493);
        vlt_hex = ltc2493_acquire(ltc2493);
        *(packet + (n*n_chnls + m)) =
          net_float_swap(ltc2493_convert_to_voltage(vlt_hex));
        m++;
        if (ltc2493_out_of_range(vlt_hex)) {
          if (verbose) printf("\e[1;31mVLT-0: OUT OF RANGE\e[0m");
        } else
          if (verbose) printf("\e[1;32mVLT-0: %X = %f V\e[0m",
            vlt_hex, ltc2493_convert_to_voltage(vlt_hex));
      }
      if (chnl_select.vlt_0 && chnl_select.vlt_1 && verbose) printf("\t\t");
      if (chnl_select.vlt_1 == TRUE) {
        ltc2493->chnl_sel = LTC2493_CHNL_1;
        ltc2493_write_setup(ltc2493);
        vlt_hex = ltc2493_acquire(ltc2493);
        *(packet + (n*n_chnls + m)) =
          net_float_swap(ltc2493_convert_to_voltage(vlt_hex));
        m++;
        if (ltc2493_out_of_range(vlt_hex)) {
          if (verbose) printf("\e[1;31mVLT-1: OUT OF RANGE\e[0m");
        } else
          if (verbose) printf("\e[1;32mVLT-1: %X = %f V\e[0m",
            vlt_hex, ltc2493_convert_to_voltage(vlt_hex));
      }
      if ((chnl_select.vlt_0 || chnl_select.vlt_1) && verbose) printf("\n");
      if (chnl_select.vlt_2 == TRUE) {
        ltc2493->chnl_sel = LTC2493_CHNL_2;
        ltc2493_write_setup(ltc2493);
        vlt_hex = ltc2493_acquire(ltc2493);
        *(packet + (n*n_chnls + m)) =
          net_float_swap(ltc2493_convert_to_voltage(vlt_hex));
        m++;
        if (ltc2493_out_of_range(vlt_hex)) {
          if (verbose) printf("\e[1;31mVLT-2: OUT OF RANGE\e[0m");
        } else
          if (verbose) printf("\e[1;32mVLT-2: %X = %f V\e[0m",
            vlt_hex, ltc2493_convert_to_voltage(vlt_hex));
      }
      if (chnl_select.vlt_2 && chnl_select.vlt_3 && verbose) printf("\t\t");
      if (chnl_select.vlt_3 == TRUE) {
        ltc2493->chnl_sel = LTC2493_CHNL_3;
        ltc2493_write_setup(ltc2493);
        vlt_hex = ltc2493_acquire(ltc2493);
        *(packet + (n*n_chnls + m)) =
          net_float_swap(ltc2493_convert_to_voltage(vlt_hex));
        m++;
        if (ltc2493_out_of_range(vlt_hex)) {
          if (verbose) printf("\e[1;31mVLT-3: OUT OF RANGE\e[0m");
        } else
          if (verbose) printf("\e[1;32mVLT-3: %X = %f V\e[0m",
            vlt_hex, ltc2493_convert_to_voltage(vlt_hex));
      }
      if ((chnl_select.vlt_2 || chnl_select.vlt_3) && verbose) printf("\n");
      if (chnl_select.vlt_0_1 == TRUE) {
        ltc2493->chnl_sel = LTC2493_CHNL_0_1;
        ltc2493_write_setup(ltc2493);
        vlt_hex = ltc2493_acquire(ltc2493);
        *(packet + (n*n_chnls + m)) =
          net_float_swap(ltc2493_convert_to_voltage(vlt_hex));
        m++;
        if (ltc2493_out_of_range(vlt_hex)) {
          if (verbose) printf("\e[1;31mVLT-0-1: OUT OF RANGE\e[0m");
        } else
          if (verbose) printf("\e[1;32mVLT-0-1: %X = %f V\e[0m",
            vlt_hex, ltc2493_convert_to_voltage(vlt_hex));
      }
      if (chnl_select.vlt_0_1 && chnl_select.vlt_2_3 && verbose) printf("\t\t");
      if (chnl_select.vlt_2_3 == TRUE) {
        ltc2493->chnl_sel = LTC2493_CHNL_2_3;
        ltc2493_write_setup(ltc2493);
        vlt_hex = ltc2493_acquire(ltc2493);
        *(packet + (n*n_chnls + m)) =
          net_float_swap(ltc2493_convert_to_voltage(vlt_hex));
        m++;
        if (ltc2493_out_of_range(vlt_hex)) {
          if (verbose) printf("\e[1;31mVLT-2-3: OUT OF RANGE\e[0m");
        } else
          if (verbose) printf("\e[1;32mVLT-2-3: %X = %f V\e[0m",
            vlt_hex, ltc2493_convert_to_voltage(vlt_hex));
      }
      if ((chnl_select.vlt_0_1 || chnl_select.vlt_2_3) && verbose) printf("\n");
      if (chnl_select.vlt_1_0 == TRUE) {
        ltc2493->chnl_sel = LTC2493_CHNL_1_0;
        ltc2493_write_setup(ltc2493);
        vlt_hex = ltc2493_acquire(ltc2493);
        *(packet + (n*n_chnls + m)) =
          net_float_swap(ltc2493_convert_to_voltage(vlt_hex));
        m++;
        if (ltc2493_out_of_range(vlt_hex)) {
          if (verbose) printf("\e[1;31mVLT-1-0: OUT OF RANGE\e[0m");
        } else
          if (verbose) printf("\e[1;32mVLT-1-0: %X = %f V\e[0m",
            vlt_hex, ltc2493_convert_to_voltage(vlt_hex));
      }
      if (chnl_select.vlt_1_0 && chnl_select.vlt_3_2 && verbose) printf("\t\t");
      if (chnl_select.vlt_3_2 == TRUE) {
        ltc2493->chnl_sel = LTC2493_CHNL_3_2;
        ltc2493_write_setup(ltc2493);
        vlt_hex = ltc2493_acquire(ltc2493);
        *(packet + (n*n_chnls + m)) =
          net_float_swap(ltc2493_convert_to_voltage(vlt_hex));
        m++;
        if (ltc2493_out_of_range(vlt_hex)) {
          if (verbose) printf("\e[1;31mVLT-3-2: OUT OF RANGE\e[0m");
        } else
          if (verbose) printf("\e[1;32mVLT-3-2: %X = %f V\e[0m",
            vlt_hex, ltc2493_convert_to_voltage(vlt_hex));
      }
      if ((chnl_select.vlt_1_0 || chnl_select.vlt_3_2) && verbose) printf("\n");
    }
  } while (net_write(net, packet, sizeof(float)*n_smpls*n_chnls));

  printf("\e[1;34mTCP/IP connection lost...\e[0m\n");

  free(packet);

  net_delete(net);
  ad7746_idle(ad7746);
  ad7746_delete(ad7746);
  gpio_delete_board(gpio, gpio_board);
  gpio_delete(gpio);
  return 0;
}
