#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/i2c-dev.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ad7746.h"

#define AD7746_STATUS       0x00
#define AD7746_CAP_DATA_H   0x01
#define AD7746_CAP_DATA_M   0x02
#define AD7746_CAP_DATA_L   0x03
#define AD7746_VT_DATA_H    0x04
#define AD7746_VT_DATA_M    0x05
#define AD7746_VT_DATA_L    0x06
#define AD7746_CAP_SETUP    0x07
#define AD7746_VT_SETUP     0x08
#define AD7746_EXC_SETUP    0x09
#define AD7746_CONFIGURE    0x0A
#define AD7746_CAP_DAC_A    0x0B
#define AD7746_CAP_DAC_B    0x0C
#define AD7746_CAP_OFFSET_H 0x0D
#define AD7746_CAP_OFFSET_L 0x0E
#define AD7746_CAP_GAIN_H   0x0F
#define AD7746_CAP_GAIN_L   0x10
#define AD7746_VOLT_GAIN_H  0x11
#define AD7746_VOLT_GAIN_L  0x12

#define AD7746_STATUS_RDYCAP 0x01
#define AD7746_STATUS_RDYVT  0x02
#define AD7746_STATUS_RDY    0x04
#define AD7746_STATUS_EXCERR 0x08

#define AD7746_CAP_SETUP_CAPCHOP 0x01
#define AD7746_CAP_SETUP_CAPDIFF 0x20
#define AD7746_CAP_SETUP_CIN2    0x40
#define AD7746_CAP_SETUP_CAPEN   0x80

#define AD7746_VT_SETUP_VTCHOP  0x01
#define AD7746_VT_SETUP_VTSHORT 0x02
#define AD7746_VT_SETUP_EXTREF  0x10
#define AD7746_VT_SETUP_VTMD0   0x20
#define AD7746_VT_SETUP_VTDM1   0x40
#define AD7746_VT_SETUP_VTEN    0x80

#define AD7746_EXC_SETUP_EXCLVL0 0x01
#define AD7746_EXC_SETUP_EXCLVL1 0x02

#define AD7746_EXC_SETUP_NEXCA   0x04
#define AD7746_EXC_SETUP_EXCA    0x08
#define AD7746_EXC_SETUP_NEXCB   0x10
#define AD7746_EXC_SETUP_EXCB    0x20
#define AD7746_EXC_SETUP_EXCON   0x40
#define AD7746_EXC_SETUP_CLKCTRL 0x80

#define AD7746_CONFIGURE_MD0   0x01
#define AD7746_CONFIGURE_MD1   0x02
#define AD7746_CONFIGURE_MD2   0x04
#define AD7746_CONFIGURE_CAPF0 0x08
#define AD7746_CONFIGURE_CAPF1 0x10
#define AD7746_CONFIGURE_CAPF2 0x20
#define AD7746_CONFIGURE_VTF0  0x40
#define AD7746_CONFIGURE_VTF1  0x80

#define AD7746_CAP_DAC_A_DACA    0x7F
#define AD7746_CAP_DAC_A_DACAENA 0x80

#define AD7746_CAP_DAC_A_DACB    0x7F
#define AD7746_CAP_DAC_A_DACBENB 0x80

/*----------------------------------------------------------------------------*/
#define THIS ((_ad7746_priv *)ad7746->priv)

typedef struct {
  _s32 fd;
} _ad7746_priv;

/*----------------------------------------------------------------------------*/
_ad7746* ad7746_new(char *dev, _u8 addr)
{
  _ad7746 *ad7746;
  _u32 funcs;

  ad7746 = malloc(sizeof(_ad7746));
  ad7746->priv = malloc(sizeof(_ad7746_priv));

  if ((THIS->fd = open(dev, O_RDWR)) < 0) {
    perror("Unable to open I2C control file.\n");
    return NULL;
  }

  ioctl(THIS->fd, I2C_FUNCS, &funcs);
  if (!(funcs & I2C_FUNC_SMBUS_READ_BYTE_DATA)) {
    perror("SMBus read byte-data not supported.\n");
    return NULL;
  }

  ioctl(THIS->fd, I2C_SLAVE, addr);

  return ad7746;
}

/*----------------------------------------------------------------------------*/
void ad7746_delete(_ad7746 *ad7746)
{
  close(THIS->fd);
  free(ad7746->priv);
  free(ad7746);
}

/*----------------------------------------------------------------------------*/
void ad7746_write_setup(_ad7746 *ad7746)
{
  _u8 exc_reg = 0x00;
  _u8 cap_reg = 0x00;

  if (ad7746->exc == AD7746_EXCA) exc_reg |= AD7746_EXC_SETUP_EXCA;
  if (ad7746->exc == AD7746_EXCB) exc_reg |= AD7746_EXC_SETUP_EXCB;
  if (ad7746->excl == AD7746_EXCL_1_OVER_4) exc_reg |= AD7746_EXC_SETUP_EXCLVL0;
  if (ad7746->excl == AD7746_EXCL_3_OVER_8) exc_reg |= AD7746_EXC_SETUP_EXCLVL1;
  if (ad7746->excl == AD7746_EXCL_1_OVER_2)
    exc_reg |= AD7746_EXC_SETUP_EXCLVL0 | AD7746_EXC_SETUP_EXCLVL1;
  i2c_smbus_write_byte_data(THIS->fd, AD7746_EXC_SETUP, exc_reg);

  if (ad7746->cin == AD7746_CIN2) cap_reg |= AD7746_CAP_SETUP_CIN2;
  if (ad7746->cap_diff == TRUE) cap_reg |= AD7746_CAP_SETUP_CAPDIFF;
  cap_reg |= AD7746_CAP_SETUP_CAPEN;
  i2c_smbus_write_byte_data(THIS->fd, AD7746_CAP_SETUP, cap_reg);
}

/*----------------------------------------------------------------------------*/
void ad7746_write_capdac(_ad7746 *ad7746,_u8 capdac)
{
  i2c_smbus_write_byte_data(THIS->fd, AD7746_CAP_DAC_A,
    AD7746_CAP_DAC_A_DACAENA | (capdac & AD7746_CAP_DAC_A_DACA));
}

/*----------------------------------------------------------------------------*/
_u16 ad7746_calibrate(_ad7746 *ad7746,_u8 capdac)
{
  _u16 data;

  i2c_smbus_write_byte_data(THIS->fd, AD7746_CAP_DAC_A,
    AD7746_CAP_DAC_A_DACAENA | (capdac & AD7746_CAP_DAC_A_DACA));
  i2c_smbus_write_byte_data(THIS->fd, AD7746_CONFIGURE,
    AD7746_CONFIGURE_MD2 | AD7746_CONFIGURE_MD0 |
    AD7746_CONFIGURE_CAPF2 | AD7746_CONFIGURE_CAPF1 |
    AD7746_CONFIGURE_CAPF0);
  usleep(200000);
  while (i2c_smbus_read_byte_data(THIS->fd, AD7746_STATUS) &
    AD7746_STATUS_RDYCAP) continue;
  data  = i2c_smbus_read_byte_data(THIS->fd, AD7746_CAP_OFFSET_H) << 0x08;
  data |= i2c_smbus_read_byte_data(THIS->fd, AD7746_CAP_OFFSET_L);

  return data;
}

/*----------------------------------------------------------------------------*/
_u32 ad7746_convert(_ad7746 *ad7746)
{
  _u32 data;

  i2c_smbus_write_byte_data(THIS->fd, AD7746_CONFIGURE,
    AD7746_CONFIGURE_MD1 |
    AD7746_CONFIGURE_CAPF2 | AD7746_CONFIGURE_CAPF1 |
    AD7746_CONFIGURE_CAPF0);
  usleep(200000);
  while (i2c_smbus_read_byte_data(THIS->fd, AD7746_STATUS) &
    AD7746_STATUS_RDYCAP) continue;
  data  = i2c_smbus_read_byte_data(THIS->fd, AD7746_CAP_DATA_H) << 0x0F;
  data |= i2c_smbus_read_byte_data(THIS->fd, AD7746_CAP_DATA_M) << 0x08;
  data |= i2c_smbus_read_byte_data(THIS->fd, AD7746_CAP_DATA_L);

  return data;
}

/*----------------------------------------------------------------------------*/
_bool ad7746_read_excerr(_ad7746 *ad7746)
{
  return ((i2c_smbus_read_byte_data(THIS->fd, AD7746_STATUS) &
    AD7746_STATUS_EXCERR) > 0x00);
}

/*----------------------------------------------------------------------------*/
_u8 ad7746_read_capdac(_ad7746 *ad7746)
{
  return (i2c_smbus_read_byte_data(THIS->fd, AD7746_CAP_DAC_A) &
    AD7746_CAP_DAC_A_DACA);
}
