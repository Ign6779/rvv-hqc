#ifndef GF32V_TOWER_H
#define GF32V_TOWER_H

#include <stdint.h>
#include "fafft_layout.h"

void gf32v_mul(gf32v_array *out, const gf32v_array *a, const gf32v_array *b);
void gf32v_mul_scalar(gf32v_array *out, const gf32v_array *a, uint32_t scalar);
void gf32v_square(gf32v_array *out, const gf32v_array *a);

/* Specific cheap mults */
void gf32v_mul_0x2(gf32v_array *out, const gf32v_array *a);
void gf32v_mul_0x5(gf32v_array *out, const gf32v_array *a);

#endif