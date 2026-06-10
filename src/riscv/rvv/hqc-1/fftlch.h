#ifndef FFTLCH_H
#define FFTLCH_H

#include "gf32v.h"

void fafft_fftlch(gf32v_array *out, const gf32v_array *in);
void fafft_inverse_fftlch(gf32v_array *out, const gf32v_array *in);

#endif