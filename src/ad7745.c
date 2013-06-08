#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/i2c-dev.h>

#include "ad7745.h"

#define AD7745_STATUS       0x00
#define AD7745_CAP_DATA_H   0x01
#define AD7745_CAP_DATA_M   0x02
#define AD7745_CAP_DATA_L   0x03
#define AD7745_VT_DATA_H    0x04
#define AD7745_VT_DATA_M    0x05
#define AD7745_VT_DATA_L    0x06
#define AD7745_CAP_SETUP    0x07
#define AD7745_VT_SETUP     0x08
#define AD7745_EXC_SETUP    0x09
#define AD7745_CONFIGURE    0x0A
#define AD7745_CAP_DAC_A    0x0B
#define AD7745_CAP_DAC_B    0x0C
#define AD7745_CAP_OFFSET_H 0x0D
#define AD7745_CAP_OFFSET_L 0x0E
#define AD7745_CAP_GAIN_H   0x0F
#define AD7745_CAP_GAIN_L   0x10
#define AD7745_VOLT_GAIN_H  0x11
#define AD7745_VOLT_GAIN_L  0x12

#define AD7745_STATUS_RDYCAP 0x01
#define AD7745_STATUS_RDYVT  0x02
#define AD7745_STATUS_RDY    0x04
#define AD7745_STATUS_EXCERR 0x08

#define AD7745_CAP_SETUP_CAPCHOP 0x01
#define AD7745_CAP_SETUP_CAPDIFF 0x20
#define AD7745_CAP_SETUP_CIN2    0x40
#define AD7745_CAP_SETUP_CAPEN   0x80

#define AD7745_VT_SETUP_VTCHOP  0x01
#define AD7745_VT_SETUP_VTSHORT 0x02
#define AD7745_VT_SETUP_EXTREF  0x10
#define AD7745_VT_SETUP_VTMD0   0x20
#define AD7745_VT_SETUP_VTDM1   0x40
#define AD7745_VT_SETUP_VTEN    0x80

#define AD7745_EXC_SETUP_EXCLVL0 0x01
#define AD7745_EXC_SETUP_EXCLVL1 0x02
#define AD7745_EXC_SETUP_NEXCA   0x04
#define AD7745_EXC_SETUP_EXCA    0x08
#define AD7745_EXC_SETUP_NEXCB   0x10
#define AD7745_EXC_SETUP_EXCB    0x20
#define AD7745_EXC_SETUP_EXCON   0x40
#define AD7745_EXC_SETUP_CLKCTRL 0x80

#define AD7745_CONFIGURE_MD0   0x01
#define AD7745_CONFIGURE_MD1   0x02
#define AD7745_CONFIGURE_MD2   0x04
#define AD7745_CONFIGURE_CAPF0 0x08
#define AD7745_CONFIGURE_CAPF1 0x10
#define AD7745_CONFIGURE_CAPF2 0x20
#define AD7745_CONFIGURE_VTF0  0x40
#define AD7745_CONFIGURE_VTF1  0x80

#define AD7745_CAP_DAC_A_DACA    0x7F
#define AD7745_CAP_DAC_A_DACAENA 0x80

#define AD7745_CAP_DAC_A_DACB    0x7F
#define AD7745_CAP_DAC_A_DACBENB 0x80

/*----------------------------------------------------------------------------*/
_s32 ad7745_init(const char *dev, _u8 addr)
{
  _s32 fd;
  _u32 funcs;

  if ((fd = open(dev, O_RDWR)) < 0) {
    perror("Unable to open I2C control file.\n");
    exit(1);
  }

  ioctl(fd, I2C_FUNCS, &funcs);
  if (!(funcs & I2C_FUNC_SMBUS_READ_BYTE_DATA)) {
    printf("SMBus read byte data not supported.\n");
    exit(1);
  }

  ioctl(fd, I2C_SLAVE, addr);

  return fd;
}


/*----------------------------------------------------------------------------*/
void ad7745_write_capdac(_s32 fd, _u8 capdac)
{
  i2c_smbus_write_byte_data(fd, AD7745_CAP_DAC_A,
    AD7745_CAP_DAC_A_DACAENA | (capdac & AD7745_CAP_DAC_A_DACA));
  i2c_smbus_write_byte_data(fd, AD7745_EXC_SETUP,
    AD7745_EXC_SETUP_EXCA | AD7745_EXC_SETUP_EXCLVL0 |
    AD7745_EXC_SETUP_EXCLVL1);
  i2c_smbus_write_byte_data(fd, AD7745_CAP_SETUP,
    AD7745_CAP_SETUP_CAPEN);
  i2c_smbus_write_byte_data(fd, AD7745_CAP_DAC_A,
    AD7745_CAP_DAC_A_DACAENA | (capdac & AD7745_CAP_DAC_A_DACA));
}

/*----------------------------------------------------------------------------*/
_u16 ad7745_calibrate(_s32 fd, _u8 capdac)
{
  _u16 data;

  i2c_smbus_write_byte_data(fd, AD7745_CAP_DAC_A,
    AD7745_CAP_DAC_A_DACAENA | (capdac & AD7745_CAP_DAC_A_DACA));
  i2c_smbus_write_byte_data(fd, AD7745_EXC_SETUP,
    AD7745_EXC_SETUP_EXCA | AD7745_EXC_SETUP_EXCLVL0 |
    AD7745_EXC_SETUP_EXCLVL1 );
  i2c_smbus_write_byte_data(fd, AD7745_CAP_SETUP,
    AD7745_CAP_SETUP_CAPEN);
  i2c_smbus_write_byte_data(fd, AD7745_CONFIGURE,
    AD7745_CONFIGURE_MD2 | AD7745_CONFIGURE_MD0 |
    AD7745_CONFIGURE_CAPF2 | AD7745_CONFIGURE_CAPF1 |
    AD7745_CONFIGURE_CAPF0);
  usleep(200000);
  while (i2c_smbus_read_byte_data(fd, AD7745_STATUS) &
    AD7745_STATUS_RDYCAP) continue;
  data  = i2c_smbus_read_byte_data(fd, AD7745_CAP_OFFSET_H) << 0x08;
  data |= i2c_smbus_read_byte_data(fd, AD7745_CAP_OFFSET_L);

  return data;
}

/*----------------------------------------------------------------------------*/
_u32 ad7745_convert(_s32 fd)
{
  _u32 data;

  i2c_smbus_write_byte_data(fd, AD7745_CONFIGURE,
    AD7745_CONFIGURE_MD1 |
    AD7745_CONFIGURE_CAPF2 | AD7745_CONFIGURE_CAPF1 |
    AD7745_CONFIGURE_CAPF0);
  usleep(200000);
  while (i2c_smbus_read_byte_data(fd, AD7745_STATUS) &
    AD7745_STATUS_RDYCAP) continue;
  data  = i2c_smbus_read_byte_data(fd, AD7745_CAP_DATA_H) << 0x0F;
  data |= i2c_smbus_read_byte_data(fd, AD7745_CAP_DATA_M) << 0x08;
  data |= i2c_smbus_read_byte_data(fd, AD7745_CAP_DATA_L);

  return data;
}

/*----------------------------------------------------------------------------*/
_bool ad7745_read_excerr(_s32 fd)
{
  return ((i2c_smbus_read_byte_data(fd, AD7745_STATUS) &
    AD7745_STATUS_EXCERR) > 0x00);
}

/*----------------------------------------------------------------------------*/
_u8 ad7745_read_capdac(_s32 fd)
{
  return (i2c_smbus_read_byte_data(fd, AD7745_CAP_DAC_A) &
    AD7745_CAP_DAC_A_DACA);
}

/*----------------------------------------------------------------------------*/
void ad7745_finalize(_s32 fd)
{
  close(fd);
}
