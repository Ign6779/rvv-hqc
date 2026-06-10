/**
 * @file gf2x.c
 *
 * HQC polynomial multiplication backend using RVV FAFFT.
 */

#include "gf2x.h"
#include "parameters.h"
#include "fafft.h"

#include <stdint.h>

/**
 * Carry-less multiplication modulo X^PARAM_N - 1.
 *
 * HQC calls this function.
 * The actual FAFFT implementation lives in riscv/rvv/fafft/.
 */
void vect_mul(uint64_t *o, const uint64_t *a1, const uint64_t *a2) {
    fafft_mul_reduced(o, a1, a2);
}