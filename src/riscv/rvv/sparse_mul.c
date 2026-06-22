/**
 * @file sparse_mul.c
 * @brief Sparse-by-dense carry-less multiplication over GF(2) mod X^PARAM_N - 1.
 */

#include "sparse_mul.h"

#include <stdint.h>
#include <string.h>

#include "parameters.h"

/* Doubled buffer holds 2*PARAM_N bits (two copies of a2 back-to-back).
 * +2 words of head-room for the (word + 1) reads in the windowed shift. */
#define SPARSE_B_WORDS (2 * VEC_N_SIZE_64 + 2)

void sparse_mul(uint64_t *o, const uint64_t *a1, const uint64_t *a2) {
    uint64_t B[SPARSE_B_WORDS];

    /* The second copy of a2 begins at bit PARAM_N. PARAM_N is prime, hence not
     * a multiple of 64 for any HQC parameter set, so sb is in [1, 63]. */
    const unsigned sw = PARAM_N >> 6;
    const unsigned sb = PARAM_N & 63;

    /* Build B = [ a2 | a2 ].
     * Low copy: a2 verbatim (its bits above PARAM_N-1 are already zero).
     * High copy: a2 shifted up by PARAM_N bits, OR-ed in (the shift carries
     * across the word boundary at sw, but never collides with the low copy). */
    memset(B, 0, sizeof(B));
    memcpy(B, a2, VEC_N_SIZE_64 * sizeof(uint64_t));
    for (size_t j = 0; j < VEC_N_SIZE_64; j++) {
        B[sw + j]     |= a2[j] << sb;
        B[sw + j + 1] |= a2[j] >> (64 - sb);
    }

    /* Accumulate one cyclic rotation of a2 per set bit of a1. */
    memset(o, 0, VEC_N_SIZE_64 * sizeof(uint64_t));
    for (size_t w = 0; w < VEC_N_SIZE_64; w++) {
        uint64_t word = a1[w];
        while (word) {
            unsigned bit = (unsigned)__builtin_ctzll(word);
            word &= word - 1; /* clear the lowest set bit */

            uint32_t s = (uint32_t)(w * 64u + bit); /* support index in [0, PARAM_N) */

            /* rotate-left by s  ==  read PARAM_N bits of B starting at bit
             * (PARAM_N - s). Split that start offset into word/bit parts. */
            uint32_t off = (uint32_t)PARAM_N - s; /* in [1, PARAM_N] */
            unsigned ws = off >> 6;
            unsigned bo = off & 63;

            if (bo == 0) {
                for (size_t j = 0; j < VEC_N_SIZE_64; j++) {
                    o[j] ^= B[ws + j];
                }
            } else {
                for (size_t j = 0; j < VEC_N_SIZE_64; j++) {
                    o[j] ^= (B[ws + j] >> bo) | (B[ws + j + 1] << (64 - bo));
                }
            }
        }
    }

    /* Single-mask reduction: clear bits above PARAM_N-1 in the top word. */
    o[VEC_N_SIZE_64 - 1] &= BITMASK(PARAM_N, 64);
}