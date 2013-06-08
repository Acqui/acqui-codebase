#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "ad7745.h"

#define I2C_DEV      "/dev/i2c-3"
#define AD7745_ADDR  0x48
#define CAPDAC       0x00
#define N_SMPLS      10
#define N_PORT       4646

/*----------------------------------------------------------------------------*/
_s32 main(int argc, char *argv[])
{
  _s32  fd;
  _u8   capdac = CAPDAC;
  _u32  cap_hex;
  _u32  cap[N_SMPLS];
  _u8  n;
  _s32 sock_fd = 0, conn_fd = 0;
  struct sockaddr_in serv_addr;

  if (argc==2) sscanf(argv[1],"%x",(_u32 *)&capdac);

  fd = ad7745_init(I2C_DEV, AD7745_ADDR);

  ad7745_write_capdac(fd, capdac);
  capdac = ad7745_read_capdac(fd);
  printf("CAPDAC:     0x%02X = %.2f pF\n", capdac,
    ((float)capdac/0x7F)*21.0);

  if (ad7745_read_excerr(fd))
    printf("\e[1;31mEXCERR:     1\e[0m\n");
  else
    printf("EXCERR:     0\n");

  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&serv_addr, 0x00, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(N_PORT);

  bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

  listen(sock_fd, 1);
  printf("\e[1;34mAwaiting TCP/IP connection...\e[0m\n");
  conn_fd = accept(sock_fd, (struct sockaddr*)NULL, NULL);
  do {
    for(n=0;n<N_SMPLS;n++) {
      cap_hex = ad7745_convert(fd);
      cap[n] = cap_hex;
      printf("\e[1;32mCAP: 0x%02X = %.6f pF\e[0m\n", cap_hex,
        (((float)cap_hex-0x800000)/0xFFFFFF*8.192));
    }
  } while (write(conn_fd, cap, sizeof(float)*N_SMPLS)!=-1);
  printf("\e[1;34mTCP/IP connection lost...\e[0m\n");
  close(conn_fd);
  ad7745_finalize(fd);
  return 0;
}
