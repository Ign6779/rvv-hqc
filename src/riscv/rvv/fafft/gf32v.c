#include "gf32v.h"

extern void gf32v_rvv_zero(gf32v_word_t *a, unsigned nwords);
extern void gf32v_rvv_copy(gf32v_word_t *out, const gf32v_word_t *in, unsigned nwords);
extern void gf32v_rvv_xor(gf32v_word_t *out, const gf32v_word_t *a, const gf32v_word_t *b, unsigned nwords);
extern void gf32v_rvv_xor_inplace(gf32v_word_t *a, const gf32v_word_t *b, unsigned nwords);

#define GF32V_TOTAL_WORDS (GF32V_PLANES * GF32V_WORDS)

void gf32v_zero(gf32v_array *a) {
    gf32v_rvv_zero(&a->plane[0][0], GF32V_TOTAL_WORDS);
}

void gf32v_copy(gf32v_array *out, const gf32v_array *in) {
    gf32v_rvv_copy(&out->plane[0][0], &in->plane[0][0], GF32V_TOTAL_WORDS);
}

void gf32v_add(gf32v_array *out, const gf32v_array *a, const gf32v_array *b) {
    gf32v_rvv_xor(&out->plane[0][0], &a->plane[0][0], &b->plane[0][0], GF32V_TOTAL_WORDS);
}

void gf32v_add_inplace(gf32v_array *a, const gf32v_array *b) {
    gf32v_rvv_xor_inplace(&a->plane[0][0], &b->plane[0][0], GF32V_TOTAL_WORDS);
}