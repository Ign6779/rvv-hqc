/**
 * @file gf2x.c
 * @brief HQC polynomial multiplication backend: sparse-by-dense (RVV branch).
 */

#include "gf2x.h"
#include "sparse_mul.h"

#include <stdint.h>

/**
 * Carry-less multiplication modulo X^PARAM_N - 1.
 *
 * HQC always passes the fixed-weight (sparse) operand as a1, so we route
 * straight to the sparse-by-dense kernel. The kernel is correct for any
 * weight of a1; it is simply fastest when a1 is sparse.
 */
void vect_mul(uint64_t *o, const uint64_t *a1, const uint64_t *a2) {
    sparse_mul(o, a1, a2);
}
