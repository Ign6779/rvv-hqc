/**
 * @file sparse_mul.c
 * @brief Sparse-by-dense carry-less multiplication over GF(2) mod X^PARAM_N - 1.
 *
 * In every HQC polynomial multiplication one operand is a low-weight
 * (fixed-weight) vector. Writing a1 = sum_{i in supp(a1)} X^i, the product
 * modulo (X^PARAM_N - 1) is
 *
 *     a1 * a2  =  XOR over i in supp(a1) of ( a2 rotated cyclically left by i ).
 *
 * Each cyclic rotation is read straight out of a doubled buffer
 * B = [ a2 | a2 ] (a2 concatenated with itself at the bit level). Because the
 * rotation is already cyclic, no wrap-around handling is needed and the
 * reduction mod (X^PARAM_N - 1) collapses to a single mask of the top word.
 *
 * This is correct for any weight of a1 (a dense a1 just means more rotations),
 * so it is a drop-in for vect_mul; it is merely *fast* when a1 is sparse.
 *
 * Vectorized: the per-rotation shift-with-carry + XOR, the doubled-buffer
 * build, and the accumulator merge run through VLA RVV kernels in
 * sparse_mul.S. Support extraction (scanning a1 for set bits) stays scalar.
 */

#include "sparse_mul.h"

#include <stdint.h>
#include <string.h>

#include "parameters.h"

/* RVV kernels (sparse_mul.S). All are vector-length agnostic. */
extern void sparse_rvv_rotate_xor(uint64_t *acc, const uint64_t *win, uint64_t bo, uint64_t nwords);
extern void sparse_rvv_or_shl(uint64_t *dst, const uint64_t *src, uint64_t sh, uint64_t nwords);
extern void sparse_rvv_or_shr(uint64_t *dst, const uint64_t *src, uint64_t sh, uint64_t nwords);
extern void sparse_rvv_xor(uint64_t *dst, const uint64_t *src, uint64_t nwords);

/* Doubled buffer holds 2*PARAM_N bits (two copies of a2 back-to-back).
 * +2 words of head-room for the (word + 1) reads in the windowed shift. */
#define SPARSE_B_WORDS (2 * VEC_N_SIZE_64 + 2)

/* Number of independent accumulators used to break the XOR dependency chain
 * across successive rotations (point 3). Must be a power of two. Set to 1 to
 * recover the single-accumulator version for A/B benchmarking. */
#define SPARSE_NACC 4

void sparse_mul(uint64_t *o, const uint64_t *a1, const uint64_t *a2) {
    uint64_t B[SPARSE_B_WORDS];
    uint64_t acc[SPARSE_NACC * VEC_N_SIZE_64];

    /* The second copy of a2 begins at bit PARAM_N. PARAM_N is prime, hence not
     * a multiple of 64 for any HQC parameter set, so sb is in [1, 63]. */
    const unsigned sw = PARAM_N >> 6;
    const unsigned sb = PARAM_N & 63;

    /* Build B = [ a2 | a2 ].
     * Low copy: a2 verbatim (its bits above PARAM_N-1 are already zero).
     * High copy: a2 shifted up by PARAM_N bits, OR-ed in as two shift passes:
     *   B[sw + j]     |= a2[j] << sb
     *   B[sw + j + 1] |= a2[j] >> (64 - sb) */
    memset(B, 0, sizeof(B));
    memcpy(B, a2, VEC_N_SIZE_64 * sizeof(uint64_t));
    sparse_rvv_or_shl(&B[sw],     a2, sb,        VEC_N_SIZE_64);
    sparse_rvv_or_shr(&B[sw + 1], a2, 64u - sb,  VEC_N_SIZE_64);

    /* Accumulate one cyclic rotation of a2 per set bit of a1, round-robined
     * across SPARSE_NACC accumulators. */
    memset(acc, 0, sizeof(acc));
    unsigned t = 0;
    for (size_t w = 0; w < VEC_N_SIZE_64; w++) {
        uint64_t word = a1[w];
        while (word) {
            unsigned bit = (unsigned)__builtin_ctzll(word);
            word &= word - 1; /* clear the lowest set bit */

            uint32_t s = (uint32_t)(w * 64u + bit); /* support index in [0, PARAM_N) */

            /* rotate-left by s == read PARAM_N bits of B starting at bit
             * (PARAM_N - s). Split that start offset into word/bit parts. */
            uint32_t off = (uint32_t)PARAM_N - s; /* in [1, PARAM_N] */
            unsigned ws = off >> 6;
            unsigned bo = off & 63;

            uint64_t *dst = &acc[(t & (SPARSE_NACC - 1)) * VEC_N_SIZE_64];
            sparse_rvv_rotate_xor(dst, &B[ws], bo, VEC_N_SIZE_64);
            t++;
        }
    }

    /* Merge the accumulators into o, then single-mask the reduction. */
    memcpy(o, &acc[0], VEC_N_SIZE_64 * sizeof(uint64_t));
    for (unsigned k = 1; k < SPARSE_NACC; k++) {
        sparse_rvv_xor(o, &acc[k * VEC_N_SIZE_64], VEC_N_SIZE_64);
    }
    o[VEC_N_SIZE_64 - 1] &= BITMASK(PARAM_N, 64);
}
