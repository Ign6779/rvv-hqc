/**
 * \file gf2x.c
 * \brief RVV implementation wrapper for Toom-Karatsuba multiplication of two polynomials.
 */

#include "gf2x.h"
#include "parameters.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define GF2X_PAD_WORDS        (PARAM_N_MULT / 64)
#define GF2X_UNREDUCED_WORDS  (2 * GF2X_PAD_WORDS + 1)

/**
 * Implemented in the specific assembly file:
 *
 *   hqc-1/gf2x_toom.S
 *   hqc-3/gf2x_toom.S
 *   hqc-5/gf2x_toom.S
 *
 * @brief Computes tmp = a1 * a2 over GF(2)[X].
 *
 * tmp must have GF2X_UNREDUCED_WORDS words available.
 */
extern void rvv_toom3_mul(uint64_t *tmp, const uint64_t *a1, const uint64_t *a2);

/**
 * Implemented in gf2x_reduce.S.
 *
 * @brief Modular reduction of a degree < 2n polynomial modulo X^n - 1.
 */
// extern void rvv_reduce(uint64_t *o, const uint64_t *tmp);
static void reduce(uint64_t *o, const uint64_t *a) {
    for (size_t i = 0; i < VEC_N_SIZE_64; i++) {
        uint64_t r = a[i + VEC_N_SIZE_64 - 1] >> (PARAM_N & 0x3F);
        uint64_t carry = a[i + VEC_N_SIZE_64] << (64 - (PARAM_N & 0x3F));
        o[i] = a[i] ^ r ^ carry;
    }

    o[VEC_N_SIZE_64 - 1] &= BITMASK(PARAM_N, 64);
}

// void scalar_mul(uint64_t *r, const uint64_t *a, const uint64_t *b, size_t nwords);


/**
 * @brief Multiply two polynomials modulo X^n - 1.
 *
 * This function multiplies two dense binary polynomials without using sparsity.
 *
 * @param[out] o  Result polynomial, VEC_N_SIZE_64 words.
 * @param[in] a1  First input polynomial, VEC_N_SIZE_64 words.
 * @param[in] a2  Second input polynomial, VEC_N_SIZE_64 words.
 */
void vect_mul(uint64_t *o, const uint64_t *a1, const uint64_t *a2)
{
    uint64_t tmp[GF2X_UNREDUCED_WORDS];

    memset(tmp, 0, sizeof(tmp));

    rvv_toom3_mul(tmp, a1, a2);
    // rvv_reduce(o, tmp);
    reduce(o, tmp);

    memset(tmp, 0, sizeof(tmp));
}