#include "reduce.h"

void fafft_reduce(uint64_t *out, const uint64_t *in) {
    /*
     * Placeholder only.
     * Real implementation must reduce the FAFFT product modulo x^PARAM_N - 1.
     *
     * For now, copy the low PARAM_N words and mask the final partial word.
     */

    for (unsigned i = 0; i < PARAM_N_WORDS; i++) {
        out[i] = in[i];
    }

#if (PARAM_N % 64) != 0
    out[PARAM_N_WORDS - 1] &= (((uint64_t)1 << (PARAM_N % 64)) - 1);
#endif
}