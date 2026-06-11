#include "fftlch.h"

#include <stdint.h>

#include "fftlch_consts.h"
#include "gf32v.h"
#include "gft_mul_vi_gf32v.h"

//might move these to fftlch_consts
#define LOW32   UINT64_C(0x00000000ffffffff)
#define HIGH32  UINT64_C(0xffffffff00000000)
#define FULL64  UINT64_C(0xffffffffffffffff)

