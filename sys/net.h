#ifndef _NET_H
#define _NET_H

#include <stdlib.h>

#include "types.h"

typedef struct {
  _priv *priv;
} _net;

_net* net_new(_u16 port);
void  net_delete(_net *net);
void  net_accept(_net *net);
_bool net_write(_net *net, const void *buf, size_t size);
float net_float_swap(float val);

#endif
