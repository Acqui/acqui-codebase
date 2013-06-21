#ifndef _AD7746_H
#define _AD7746_H

#include "types.h"

typedef enum {AD7746_CHN1, AD7746_CHNL2} _AD7746_CHNL;

_s32   ad7746_init(const char *dev, _u8 slave);
void   ad7746_write_capdac(_s32 fd, _u8 capdac);
_u16   ad7746_calibrate(_s32 fd, _u8 capdac);
_u32   ad7746_convert(_s32 fd);
_bool  ad7746_read_excerr(_s32 fd);
_u8    ad7746_read_capdac(_s32 fd);
void   ad7746_finalize(_s32 fd);

#endif
