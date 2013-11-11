#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>

#include <string.h>

#include "net.h"

/*----------------------------------------------------------------------------*/
#define THIS ((_net_priv *)net->priv)

typedef struct {
  _s32 sock_fd;
  _s32 conn_fd;
  struct sockaddr_in serv_addr;
} _net_priv;

/*----------------------------------------------------------------------------*/
_net* net_new(_u16 port)
{
  _net *net;

  net = malloc(sizeof(_net));
  net->priv = malloc(sizeof(_net_priv));

  THIS->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&THIS->serv_addr, 0x00, sizeof(THIS->serv_addr));
  THIS->serv_addr.sin_family = AF_INET;
  THIS->serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  THIS->serv_addr.sin_port = htons(port);
  signal(SIGPIPE, SIG_IGN);
  bind(THIS->sock_fd, (struct sockaddr*)&THIS->serv_addr,
   sizeof(THIS->serv_addr));
  listen(THIS->sock_fd, 1);

  return net;
}

/*----------------------------------------------------------------------------*/
void net_delete(_net *net)
{
  close(THIS->conn_fd);
  free(net->priv);
  free(net);
}

/*----------------------------------------------------------------------------*/
void net_accept(_net *net)
{
  THIS->conn_fd = accept(THIS->sock_fd, (struct sockaddr*)NULL, NULL);
}

/*----------------------------------------------------------------------------*/
_bool net_write(_net *net, const void *buf, size_t size)
{
 return (write(THIS->conn_fd, buf, size)!=-1);
}

