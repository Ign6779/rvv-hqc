#include "fafft.h"

#include "basis_cvt.h"
#include "encode.h"
#include "fftlch.h"
#include "pointwise.h"
#include "reduce.h"

#include "fafft_layout.h"
#include "fafft_params.h"

#include <stdint.h>

// I aint dealing with the others for now
#if FAFFT_N_BITS != 65536
#error "RVV FAFFT currently supports only HQC-1"
#endif

static void fafft_forward(gf32v_array *out, const uint64_t *in) {
    static uint64_t basis[FAFFT_N_WORDS];
    static gf32v_array encoded;

    fafft_basis_cvt(basis, in);
    fafft_encode(&encoded, basis);
    fafft_fftlch(out, &encoded);
}

static void fafft_inverse(uint64_t *out, const gf32v_array *in) {
    static gf32v_array ifft;
    static uint64_t decoded[FAFFT_N_WORDS];

    fafft_inverse_fftlch(&ifft, in);
    fafft_decode(decoded, &ifft);
    fafft_inverse_basis_cvt(out, decoded);
}

void fafft_mul_unreduced(uint64_t *out, const uint64_t *a, const uint64_t *b) {
    static gf32v_array eval_a;
    static gf32v_array eval_b;
    static gf32v_array eval_c;

    fafft_forward(&eval_a, a);
    fafft_forward(&eval_b, b);

    fafft_pointwise_mul(&eval_c, &eval_a, &eval_b);

    fafft_inverse(out, &eval_c);
}

void fafft_mul_reduced(uint64_t *out, const uint64_t *a, const uint64_t *b) {
    static uint64_t unreduced[FAFFT_N_WORDS];

    fafft_mul_unreduced(unreduced, a, b);
    fafft_reduce(out, unreduced);
}