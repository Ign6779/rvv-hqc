#ifndef FAFFT_POINTWISE_H
#define FAFFT_POINTWISE_H

#include "fafft_layout.h"

void fafft_pointwise_mul(gf32v_array *out, const gf32v_array *a, const gf32v_array *b);

#endif