#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/i2c-dev.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ltc2493.h"

/*----------------------------------------------------------------------------*/
#define THIS ((_ltc2493_priv *)ltc2493->priv)

typedef struct {
 _s32 fd;
} _ltc2493_priv;

/*----------------------------------------------------------------------------*/
_ltc2493* ltc2493_new(char *dev, _u8 addr)
{
  _ltc2493 *ltc2493;
  _u32 funcs;

  ltc2493 = malloc(sizeof(_ltc2493));
  ltc2493->priv = malloc(sizeof(_ltc2493_priv));

  if ((THIS->fd = open("/dev/i2c-3", O_RDWR)) < 0) {
    perror("Unable to open I2C control file.\n");
    return NULL;
  }

  ioctl(THIS->fd, I2C_FUNCS, &funcs);
  if (!(funcs & I2C_FUNC_I2C)) {
    perror("Plain I2C commands not supported.\n");
    return NULL;
  }

  ioctl(THIS->fd, I2C_SLAVE, addr);

  return ltc2493;
}

/*----------------------------------------------------------------------------*/
void ltc2493_delete(_ltc2493 *ltc2493)
{
  close(THIS->fd);
  free(ltc2493->priv);
  free(ltc2493);
}

/*----------------------------------------------------------------------------*/
void ltc2493_write_setup(_ltc2493 *ltc2493)
{
  _u8 buf[2];

  switch (ltc2493->chnl_sel) {
    case LTC2493_CHNL_0_1:
      buf[0] = 0xA0;
      break;
    case LTC2493_CHNL_2_3:
      buf[0] = 0xA1;
      break;
    case LTC2493_CHNL_1_0:
      buf[0] = 0xA8;
      break;
    case LTC2493_CHNL_3_2:
      buf[0] = 0xA9;
      break;
    case LTC2493_CHNL_0:
      buf[0] = 0xB0;
      break;
    case LTC2493_CHNL_2:
      buf[0] = 0xB1;
      break;
    case LTC2493_CHNL_1:
      buf[0] = 0xB8;
      break;
    case LTC2493_CHNL_3:
      buf[0] = 0xB9;
      break;
    case LTC2493_CHNL_TEMP:
      buf[0] = 0xA0;
    default:
      buf[0] = 0x00;
  }

  buf[1] = 0x80;
  if (ltc2493->chnl_sel == LTC2493_CHNL_TEMP) buf[1] |= 0x40;
  if (ltc2493->out_rate == LTC2493_RATE_DOUBLE) buf[1] |= 0x08;
  switch (ltc2493->reject) {
    case LTC2493_REJECT_50HZ:
      buf[1] |= 0x10;
      break;
    case LTC2493_REJECT_60HZ:
      buf[1] |= 0x20;
      break;
    case LTC2493_REJECT_50HZ_60HZ:
      buf[1] |= 0x30;
      break;
    default:
      buf[1] |= 0x30;
  }

  write(THIS->fd, buf, 1);
  usleep(2000000);
}

/*----------------------------------------------------------------------------*/
_s32 ltc2493_convert(_ltc2493 *ltc2493)
{
  _u8  buf[4];
  _s32 conv;

  if(read(THIS->fd, &buf, 4) != 4)
    return (0x03 << 29);
  else {
    printf("%X %X %X %X\n",buf[0],buf[1],buf[2],buf[3]);
    conv  = ((_s32)buf[0] & 0x3F) << 18;
    conv |= ((_s32)buf[1]) << 10;
    conv |= ((_s32)buf[2]) << 2;
    conv |= ((_s32)buf[3]) >> 6;

    if (buf[0] & 0x40) return conv-0xFFFFFF;
    else return conv;
  }
}
