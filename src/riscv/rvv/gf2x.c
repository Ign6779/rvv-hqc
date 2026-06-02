/**
 * @file gf2x.c
 * @brief Implementation of carry-less multiplication of two polynomials over GF(2) mod X^n - 1.
 *
 * @details
 * The temporary buffer requirement for recursive Karatsuba is computed as follows:
 *  - At each recursion level on words of length n, karatsuba_mul needs 8*n words.
 *    (z0, z2, zmid each 2*n, plus ta and tb of n each: total 8*n.)
 *  - Child calls operate on half the length (n/2), needing 8*(n/2) words, placed immediately after.
 *  - Summing across levels n + n/2 + n/4 + ... < 2*n, so total < 8*n * 2 = 16*n words.
 *  - We set TMP_BUFFER_WORDS = 16 * VEC_N_SIZE_64 to guarantee enough space for all recursion levels.
 */

#include "gf2x.h"
#include <stdint.h>
#include <string.h>
#include "parameters.h"

static void fafft_mul(uint64_t *o, const uint64_t *a, const uint64_t *b) {

}

static void reduce_fafft(uint64_t *o, const uint64_t *a) {
    memset(o, 0, VEC_N_SIZE_64 * sizeof(uint64_t));

    for (size_t bit = 0; bit < FAFFT_N_BITS; bit++) {
        uint64_t value = (a[bit >> 6] >> (bit & 63)) & 1ULL;
        size_t dst = bit % PARAM_N;
        o[dst >> 6] ^= value << (dst & 63);
    }

    o[VEC_N_SIZE_64 - 1] &= BITMASK(PARAM_N, 64); 
}

/**
 * @brief Modular reduction of a degree < 2*n polynomial mod (X^n - 1).
 *
 * Folds the high half of the full product back into the low half
 * and masks any excess bits in the last word.
 *
 * @param[out] o  Result buffer, size VEC_N_SIZE_64 words.
 * @param[in]  a  Input buffer, size 2*VEC_N_SIZE_64 words.
 */
static void reduce(uint64_t *o, const uint64_t *a) {
    for (size_t i = 0; i < VEC_N_SIZE_64; i++) {
        uint64_t r = a[i + VEC_N_SIZE_64 - 1] >> (PARAM_N & 0x3F);
        uint64_t carry = a[i + VEC_N_SIZE_64] << (64 - (PARAM_N & 0x3F));
        o[i] = a[i] ^ r ^ carry;
    }
    o[VEC_N_SIZE_64 - 1] &= BITMASK(PARAM_N, 64);
}

/**
 * @brief Carry-less multiplication mod (X^PARAM_N - 1).
 *
 * Computes o = a1 * a2, each operand of VEC_N_SIZE_64 words, then reduces.
 *
 * @param[out] o   Result buffer, size VEC_N_SIZE_64 words.
 * @param[in]  a1  Operand polynomial a(x).
 * @param[in]  a2  Operand polynomial b(x).
 */
void vect_mul(uint64_t *o, const uint64_t *a1, const uint64_t *a2) {
    uint64_t unreduced[FAFFT_N_WORDS];

    fafft_mul(unreduced, a1, a2);

    reduce_fafft(o, unreduced);
}
