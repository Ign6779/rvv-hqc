#ifndef FAFFT_LAYOUT_H
#define FAFFT_LAYOUT_H

#include "fafft_params.h"
#include "fafft_types.h"

#define GF32V_PLANES      32
#define GF32V_WORD_BITS   64
#define GF32V_NELTS       FAFFT_NP
#define GF32V_WORDS       (GF32V_NELTS / GF32V_WORD_BITS)

#if (GF32V_NELTS % GF32V_WORD_BITS) != 0
    #error "GF32V_NELTS must be divisible by 64"
#endif

typedef struct {
    gf32v_word_t plane[GF32V_PLANES][GF32V_WORDS];
} gf32v_array;

#endif