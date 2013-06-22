#ifndef _AD7746_H
#define _AD7746_H

#include "../sys/types.h"

typedef enum {AD7746_CIN1, AD7746_CIN2} _ad7746_cin;
typedef enum {AD7746_EXCA, AD7746_EXCB} _ad7746_exc;
typedef enum {AD7746_CIN1_EXCA, AD7746_CIN1_EXB,
              AD7746_CIN2_EXCA, AD7746_CIN2_EXB} _ad7746_cin_exc;

_s32   ad7746_init(const char *dev, _u8 slave);
void   ad7746_write_capdac(_s32 fd, _u8 capdac);
_u16   ad7746_calibrate(_s32 fd, _u8 capdac);
_u32   ad7746_convert(_s32 fd);
_bool  ad7746_read_excerr(_s32 fd);
_u8    ad7746_read_capdac(_s32 fd);
void   ad7746_finalize(_s32 fd);

#endif
