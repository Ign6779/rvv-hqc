#ifndef BASIS_CVT_H
#define BASIS_CVT_H

#include <stdint.h>

void fafft_basis_cvt(uint64_t *out, const uint64_t *in);
void fafft_inverse_basis_cvt(uint64_t *out, const uint64_t *in);

void fafft_basis_cvt_inplace(uint64_t *poly);
void fafft_inverse_basis_cvt_inplace(uint64_t *poly);

#endif