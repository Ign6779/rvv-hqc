#ifndef FAFFT_FFTLCH_H
#define FAFFT_FFTLCH_H

#include "fafft_layout.h"

void fafft_fftlch_inplace(gf32v_array *a);
void fafft_inverse_fftlch_inplace(gf32v_array *a);

void fafft_fftlch(gf32v_array *out, const gf32v_array *in);
void fafft_inverse_fftlch(gf32v_array *out, const gf32v_array *in);

#endif