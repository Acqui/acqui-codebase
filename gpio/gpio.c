#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../sys/types.h"

#define GPIO_PIN  48

/*----------------------------------------------------------------------------*/
_s32 main(int argc, char *argv[])
{
  _s32 fd;
  char buf[100];

  fd = open("/sys/class/gpio/export", O_WRONLY);
  sprintf(buf, "%d", GPIO_PIN);
  write(fd, buf, strlen(buf));
  close(fd);

  sprintf(buf, "/sys/class/gpio/gpio%d/direction", GPIO_PIN);
  fd = open(buf, O_WRONLY);
  write(fd, "high", 4);
  close(fd);

  sprintf(buf, "/sys/class/gpio/gpio%d/value", GPIO_PIN);
  fd = open(buf, O_WRONLY);
  write(fd, "1", 1);
  close(fd);

/*  fd = open("/sys/class/gpio/unexport", O_WRONLY);
  sprintf(buf, "%d", GPIO_PIN);
  write(fd, buf, strlen(buf));
  close(fd);
*/
  return 0;
}
