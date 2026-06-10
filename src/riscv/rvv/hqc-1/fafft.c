#include "fafft.h"
#include "fafft_params.h"
#include "fafft_types.h"

#include "basis_cvt.h"
#include "encode.h"
#include "fftlch.h"
#include "pointwise.h"
#include "reduce.h"

#include "parameters.h"

#include <stdint.h>
#include <string.h>

static void fafft_forward(gf32 *out, const uint64_t *in) {
    static f2 coeffs[FAFFT_N_BITS];
    static f2 basis[FAFFT_N_BITS];
    static gf32 encoded[FAFFT_NP];


    /*
    * TODO: move this unpacking to RVV kernel. This is just unpacking from uint64_t to arr
    */
    memset(coeffs, 0, FAFFT_N_BITS * sizeof(f2));

        for (size_t i = 0; i < PARAM_N; i++) {
        coeffs[i] = (f2)((in[i >> 6] >> (i & 63)) & 1ULL);
    }

    fafft_basis_cvt(basis, coeffs, FAFFT_N_BITS);
    fafft_encode(encoded, basis);
    fafft_fftlch(out, encoded);
}

static void fafft_inverse(uint64_t *out, const gf32 *in) {
        static gf32 ifft[FAFFT_NP];
    static f2 decoded[FAFFT_N_BITS];
    static f2 coeffs[FAFFT_N_BITS];

    fafft_inverse_fftlch(ifft, in);
    fafft_decode(decoded, ifft);
    fafft_inverse_basis_cvt(coeffs, decoded, FAFFT_N_BITS);

    // TODO: again, move to RVV kernel
    memset(out, 0, FAFFT_N_WORDS * sizeof(uint64_t));

        for (size_t i = 0; i < FAFFT_N_BITS; i++) {
        uint64_t bit = (uint64_t)(coeffs[i] & 1U);
        out[i >> 6] ^= bit << (i & 63);
    }
}

void fafft_mul_unreduced(uint64_t *out, const uint64_t *a, const uint64_t *b) {
    static gf32 eval_a[FAFFT_NP];
    static gf32 eval_b[FAFFT_NP];
    static gf32 eval_c[FAFFT_NP];

    fafft_forward(eval_a, a);
    fafft_forward(eval_b, b);

    fafft_pointwise_mul(eval_c, eval_a, eval_b);

    fafft_inverse(out, eval_c);
}

void fafft_mul_reduced(uint64_t *out, const uint64_t *a, const uint64_t *b) {
    static uint64_t unreduced[FAFFT_N_WORDS];

    fafft_mul_unreduced(unreduced, a, b);
    fafft_reduce(out, unreduced);
}