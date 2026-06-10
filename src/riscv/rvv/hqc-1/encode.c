#include "encode.h"
#include <string.h>

extern void fafft_rvv_encode(gf32v_word_t *out, const uint64_t *in);
extern void fafft_rvv_decode(uint64_t *out, const gf32v_word_t *in);

void fafft_encode(gf32v_array *out, const uint64_t *in) {
    fafft_rvv_encode(&out->plane[0][0], in);
}

void fafft_decode(uint64_t *out, const gf32v_array *in) {
    fafft_rvv_decode(out, &in->plane[0][0]);
}