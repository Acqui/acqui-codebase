#ifndef _AD7745_H
#define _AD7745_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

_s32   ad7745_init(const char *dev, _u8 slave);
void   ad7745_write_capdac(_s32 fd, _u8 capdac);
_u16   ad7745_calibrate(_s32 fd, _u8 capdac);
_u32   ad7745_convert(_s32 fd);
_bool  ad7745_read_excerr(_s32 fd);
_u8    ad7745_read_capdac(_s32 fd);
void   ad7745_finalize(_s32 fd);

#endif
