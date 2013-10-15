#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../sys/types.h"

#define GPIO_PIN  48

/*----------------------------------------------------------------------------*/
_s32 main(int argc, char *argv[])
{
  _s32 fd;
  _u8 buf[10];

  fd = open("/sys/class/gpio/export", 0_WRONLY);
  sprintf(buf, "%d", GPIO_PIN);
  write(fd, buf, strlen(buf));
  close(fd);

  sprintf(buf, "/sys/class/gpio/gpio%d/direction", GPIO_PIN);
  fd = open(buf, 0_WRONLY);
  write(fd, "high", 4);
  close(fd);

  sprintf(buf, "/sys/class/gpio/gpio%d/value", GPIO_PIN);
  fd = open(buf, 0_WRONLY);
  write(fd, "1", 1);
  close(fd);

  fd = open("/sys/class/gpio/unexport", 0_WRONLY);
  sprintf(buf, "%d", GPIO_PIN);
  write(fd, buf, strlen(buf));
  close(fd);

  return 0;
}
