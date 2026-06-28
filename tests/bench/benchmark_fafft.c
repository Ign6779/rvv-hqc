/*
 * Per-stage cycle breakdown of the RVV FAFFT polynomial multiply.
 *
 * Times each stage of vect_mul() separately using the RISC-V `cycle` CSR
 * (same primitive as benchmark_kem.c), so you can see where the work goes.
 *
 * Build: picked up automatically by tests/bench (benchmark_*.c glob), links
 * the hqc_1_riscv library which exports the FAFFT headers. Run on the board.
 */

#define _DEFAULT_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if !defined(__riscv)
/* FAFFT + the cycle CSR are RISC-V only; other arch bench builds are a no-op. */
int main(void) { return 0; }
#else

#include "parameters.h"

#include "fafft.h"
#include "fafft_layout.h"
#include "basis_cvt.h"
#include "encode.h"
#include "fftlch.h"
#include "pointwise.h"
#include "reduce.h"

#if FAFFT_N_BITS != 65536
/* FAFFT path only exists for HQC-1; make the other variants a no-op binary. */
int main(void) {
    printf("benchmark_fafft: FAFFT only built for HQC-1, skipping.\n");
    return 0;
}
#else

#define ITERS 200

static inline uint64_t rdcycle(void) {
    uint64_t c;
    __asm__ __volatile__("fence\n\tcsrr %0, cycle\n\t" : "=r"(c) : : "memory");
    return c;
}

/* file-scope so we don't blow the stack: each gf32v_array / FAFFT buffer is 8 KB */
static uint64_t in_a[FAFFT_N_WORDS];
static uint64_t in_b[FAFFT_N_WORDS];
static uint64_t basis_a[FAFFT_N_WORDS];
static uint64_t basis_b[FAFFT_N_WORDS];
static gf32v_array enc_a, enc_b;
static gf32v_array eval_a, eval_b, eval_c;
static gf32v_array ifft;
static uint64_t decoded[FAFFT_N_WORDS];
static uint64_t unreduced[FAFFT_N_WORDS];
static uint64_t out[VEC_N_SIZE_64];

/* Run `stmt` ITERS times, return the minimum per-call cycle count. */
#define TIME_MIN(dst, stmt)                              \
    do {                                                 \
        uint64_t _best = UINT64_MAX;                     \
        for (int _i = 0; _i < ITERS; _i++) {             \
            uint64_t _t0 = rdcycle();                    \
            stmt;                                        \
            uint64_t _t1 = rdcycle();                    \
            uint64_t _d = _t1 - _t0;                     \
            if (_d < _best) _best = _d;                  \
        }                                                \
        (dst) = _best;                                   \
    } while (0)

/* one forward transform, reproducing fafft.c's static fafft_forward() */
static void prep_forward_input(uint64_t *basis, const uint64_t *in) {
    memset(basis, 0, FAFFT_N_WORDS * sizeof(uint64_t));
    memcpy(basis, in, VEC_N_SIZE_64 * sizeof(uint64_t));
}

int main(void) {
    srand(1);
    for (size_t i = 0; i < VEC_N_SIZE_64; i++) {
        in_a[i] = ((uint64_t)rand() << 32) ^ (uint64_t)rand();
        in_b[i] = ((uint64_t)rand() << 32) ^ (uint64_t)rand();
    }

    /* Warm up: first fftlch builds twiddle tables lazily; warm caches too. */
    prep_forward_input(basis_a, in_a);
    fafft_basis_cvt_inplace(basis_a);
    fafft_encode(&enc_a, basis_a);
    fafft_fftlch(&eval_a, &enc_a);
    fafft_pointwise_mul(&eval_c, &eval_a, &eval_a);
    fafft_inverse_fftlch(&ifft, &eval_c);
    fafft_decode(decoded, &ifft);
    fafft_inverse_basis_cvt(unreduced, decoded);
    fafft_reduce(out, unreduced);

    uint64_t c_basis, c_encode, c_fftlch, c_point, c_ifftlch, c_decode,
        c_ibasis, c_reduce, c_full;

    /* FORWARD stages (timed on operand a; b is identical work) */
    TIME_MIN(c_basis, {
        prep_forward_input(basis_a, in_a);
        fafft_basis_cvt_inplace(basis_a);
    });
    /* basis_a now holds a converted basis; encode from it repeatedly */
    TIME_MIN(c_encode, fafft_encode(&enc_a, basis_a));
    TIME_MIN(c_fftlch, fafft_fftlch(&eval_a, &enc_a));

    /* prepare a real eval_a, eval_b for pointwise */
    prep_forward_input(basis_a, in_a);
    fafft_basis_cvt_inplace(basis_a);
    fafft_encode(&enc_a, basis_a);
    fafft_fftlch(&eval_a, &enc_a);
    prep_forward_input(basis_b, in_b);
    fafft_basis_cvt_inplace(basis_b);
    fafft_encode(&enc_b, basis_b);
    fafft_fftlch(&eval_b, &enc_b);

    /* POINTWISE */
    TIME_MIN(c_point, fafft_pointwise_mul(&eval_c, &eval_a, &eval_b));

    /* INVERSE stages */
    TIME_MIN(c_ifftlch, fafft_inverse_fftlch(&ifft, &eval_c));
    TIME_MIN(c_decode, fafft_decode(decoded, &ifft));
    TIME_MIN(c_ibasis, fafft_inverse_basis_cvt(unreduced, decoded));
    TIME_MIN(c_reduce, fafft_reduce(out, unreduced));

    /* Ground-truth full multiply for cross-check */
    TIME_MIN(c_full, fafft_mul_reduced(out, in_a, in_b));

    /* Model total: forward runs twice (a and b). */
    uint64_t fwd_once = c_basis + c_encode + c_fftlch;
    uint64_t model = 2 * fwd_once + c_point + c_ifftlch + c_decode + c_ibasis +
                     c_reduce;

    printf("\n=== FAFFT per-stage cycle breakdown (min of %d) ===\n", ITERS);
    printf("%-26s %12s  %6s\n", "stage", "cycles", "%model");
    printf("------------------------------------------------\n");
#define ROW(name, c, mult)                                                   \
    printf("%-26s %12llu  %5.1f%%\n", name, (unsigned long long)(c),         \
           100.0 * (double)((mult) * (c)) / (double)model)
    ROW("basis_cvt      (x2)", c_basis, 2);
    ROW("encode         (x2)", c_encode, 2);
    ROW("fftlch fwd     (x2)", c_fftlch, 2);
    ROW("pointwise gf32v_mul", c_point, 1);
    ROW("inverse_fftlch", c_ifftlch, 1);
    ROW("decode", c_decode, 1);
    ROW("inverse_basis_cvt", c_ibasis, 1);
    ROW("reduce", c_reduce, 1);
#undef ROW
    printf("------------------------------------------------\n");
    printf("%-26s %12llu\n", "model total (2*fwd+...)",
           (unsigned long long)model);
    printf("%-26s %12llu\n", "measured full mul", (unsigned long long)c_full);
    printf("forward-once subtotal: %llu cycles\n\n",
           (unsigned long long)fwd_once);
    return 0;
}

#endif /* FAFFT_N_BITS == 65536 */
#endif /* __riscv */
