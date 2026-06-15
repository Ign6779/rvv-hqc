#include "reduce.h"

#include <stddef.h>

/*
 * Reduce the unreduced FAFFT product modulo x^PARAM_N - 1.
 * Ported from the HQC reference reduce(); identical bit layout, only the source
 * buffer is the larger FAFFT product. Assumes PARAM_N is not a multiple of 64
 * (true for HQC-1: PARAM_N & 0x3F == 5), so the (64 - shift) below is in [1,63].
 *
 * Scalar for now; the loop is a sliding-window XOR and is straightforward to
 * vectorize with RVV later.
 */
void fafft_reduce(uint64_t *out, const uint64_t *in) {
    const unsigned shift = PARAM_N & 0x3F;

    for (size_t i = 0; i < VEC_N_SIZE_64; i++) {
        uint64_t r     = in[i + VEC_N_SIZE_64 - 1] >> shift;
        uint64_t carry = in[i + VEC_N_SIZE_64] << (64 - shift);
        out[i] = in[i] ^ r ^ carry;
    }

    out[VEC_N_SIZE_64 - 1] &= BITMASK(PARAM_N, 64);
}
