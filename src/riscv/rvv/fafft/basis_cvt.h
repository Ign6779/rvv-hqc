#ifndef BASIS_CVT_H
#define BASIS_CVT_H

#include "fafft_types.h"

void fafft_basis_cvt(f2 *out, const f2 *in, unsigned n);
void fafft_inverse_basis_cvt(f2 *out, const f2 *in, unsigned n);

#endif