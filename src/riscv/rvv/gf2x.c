/**
 * @file gf2x.c
 */

#include "gf2x.h"
#include "gf32.h"
#include <stdint.h>
#include <string.h>
#include "parameters.h"

typedef uint8_t f2;

/**
 * BasisCvt: monomial basis -> novel polynomial basis.
 *
 * TODO: replace identity with real BasisCvt.
 */
static void basis_cvt(f2 *out, const f2 *in) {
    memcpy(out, in, FAFFT_N_BITS * sizeof(f2));
}

/**
 * iBasisCvt: novel polynomial basis -> monomial basis.
 *
 * TODO: replace identity with real inverse BasisCvt.
 */
static void inverse_basis_cvt(f2 *out, const f2 *in) {
    memcpy(out, in, FAFFT_N_BITS * sizeof(f2));
}

/**
 * Encode: f2[FAFFT_N_BITS] -> gf32[FAFFT_NP].
 *
 * TODO: implement real Encode.
 */
static void encode(gf32 *out, const f2 *in) {
    (void)in;
    memset(out, 0, FAFFT_NP * sizeof(gf32));
}

/**
 * Decode: gf32[FAFFT_NP] -> f2[FAFFT_N_BITS].
 *
 * TODO: implement real Decode.
 */
static void decode(f2 *out, const gf32 *in) {
    (void)in;
    memset(out, 0, FAFFT_N_BITS * sizeof(f2));
}

/**
 * FFTLCH: gf32[FAFFT_NP] -> gf32[FAFFT_NP].
 *
 * TODO: implement real FFTLCH.
 */
static void fftlch(gf32 *out, const gf32 *in) {
    memcpy(out, in, FAFFT_NP * sizeof(gf32));
}

/**
 * IFFTLCH: gf32[FAFFT_NP] -> gf32[FAFFT_NP].
 *
 * TODO: implement real inverse FFTLCH.
 */
static void inverse_fftlch(gf32 *out, const gf32 *in) {
    memcpy(out, in, FAFFT_NP * sizeof(gf32));
}

/**
 * PWM: pointwise multiplication in GF(2^32).
 */
static void pointwise_mul(gf32 *out, const gf32 *a, const gf32 *b) {
    for (size_t i = 0; i < FAFFT_NP; i++) {
        out[i] = gf32_mul(a[i], b[i]);
    }
}

/**
 * Forward FAFFT:
 *
 * packed uint64_t polynomial
 * -> f2 coefficient array
 * -> BasisCvt
 * -> Encode
 * -> FFTLCH
 */
static void fafft_forward(gf32 *out, const uint64_t *in) {
    f2 coeffs[FAFFT_N_BITS];
    f2 basis[FAFFT_N_BITS];
    gf32 encoded[FAFFT_NP];

    /*
     * Convert packed polynomial to binary coefficient array.
     * This is only storage conversion, not BasisCvt and not Encode.
     */
    memset(coeffs, 0, FAFFT_N_BITS * sizeof(f2));

    for (size_t i = 0; i < PARAM_N; i++) {
        coeffs[i] = (f2)((in[i >> 6] >> (i & 63)) & 1ULL);
    }

    basis_cvt(basis, coeffs);
    encode(encoded, basis);
    fftlch(out, encoded);
}

/**
 * Inverse FAFFT:
 *
 * IFFTLCH
 * -> Decode
 * -> iBasisCvt
 * -> packed uint64_t unreduced polynomial
 */
static void fafft_inverse(uint64_t *out, const gf32 *in) {
    gf32 ifft[FAFFT_NP];
    f2 decoded[FAFFT_N_BITS];
    f2 coeffs[FAFFT_N_BITS];

    inverse_fftlch(ifft, in);
    decode(decoded, ifft);
    inverse_basis_cvt(coeffs, decoded);

    /*
     * Convert binary coefficient array back to packed polynomial format.
     * This is only storage conversion, not Decode and not iBasisCvt.
     */
    memset(out, 0, FAFFT_N_WORDS * sizeof(uint64_t));

    for (size_t i = 0; i < FAFFT_N_BITS; i++) {
        uint64_t bit = (uint64_t)(coeffs[i] & 1U);
        out[i >> 6] ^= bit << (i & 63);
    }
}

/**
 * Full FAFFT multiplication:
 *
 * FAFFT(a)
 * FAFFT(b)
 * PWM
 * inverse FAFFT
 */
static void fafft_mul(uint64_t *out, const uint64_t *a, const uint64_t *b) {
    gf32 eval_a[FAFFT_NP];
    gf32 eval_b[FAFFT_NP];
    gf32 eval_c[FAFFT_NP];

    fafft_forward(eval_a, a);
    fafft_forward(eval_b, b);

    pointwise_mul(eval_c, eval_a, eval_b);

    fafft_inverse(out, eval_c);
}

/**
 * Reduce FAFFT_N_BITS unreduced product modulo X^PARAM_N - 1.
 * TODO: vectorize
 */
static void reduce_fafft(uint64_t *out, const uint64_t *a) {
    memset(out, 0, VEC_N_SIZE_64 * sizeof(uint64_t));

    for (size_t bit = 0; bit < FAFFT_N_BITS; bit++) {
        uint64_t value = (a[bit >> 6] >> (bit & 63)) & 1ULL;
        size_t dst = bit % PARAM_N;

        out[dst >> 6] ^= value << (dst & 63);
    }

    out[VEC_N_SIZE_64 - 1] &= BITMASK(PARAM_N, 64);
}

/**
 * Carry-less multiplication mod X^PARAM_N - 1.
 */
void vect_mul(uint64_t *o, const uint64_t *a1, const uint64_t *a2) {
    uint64_t unreduced[FAFFT_N_WORDS] = {0};

    fafft_mul(unreduced, a1, a2);
    reduce_fafft(o, unreduced);
}