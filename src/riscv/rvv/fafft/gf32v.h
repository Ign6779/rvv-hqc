#ifndef GF32V_H
#define GF32V_H

#include "fafft_layout.h"

void gf32v_zero(gf32v_array *a);
void gf32v_copy(gf32v_array *out, const gf32v_array *in);
void gf32v_add(gf32v_array *out, const gf32v_array *a, const gf32v_array *b);
void gf32v_add_inplace(gf32v_array *a, const gf32v_array *b);

#endif