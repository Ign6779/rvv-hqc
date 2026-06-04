#include "gf32v_tower.h"
#include <stdint.h>

extern void gf32v_rvv_mul(gf32v_word_t *out, const gf32v_word_t *a, const gf32v_word_t *b, unsigned nwords);
extern void gf32v_rvv_mul_0x2(gf32v_word_t *out, const gf32v_word_t *a, unsigned nwords);
extern void gf32v_rvv_mul_0x5(gf32v_word_t *out, const gf32v_word_t *a, unsigned nwords);

void gf32v_mul(gf32v_array *out, const gf32v_array *a, const gf32v_array *b) {
    if (out == a || out == b) {
        gf32v_array tmp;
        gf32v_rvv_mul(&tmp.plane[0][0], &a->plane[0][0], &b->plane[0][0], GF32V_WORDS);
        *out = tmp;
        return;
    }

    gf32v_rvv_mul(&out->plane[0][0], &a->plane[0][0], &b->plane[0][0], GF32V_WORDS);
}

void gf32v_mul_scalar(gf32v_array *out, const gf32v_array *a, uint32_t scalar) {
    gf32v_array c;

    for (unsigned i = 0; i < GF32V_PLANES; i++) {
        gf32v_word_t mask = (scalar & 1u) ? UINT64_MAX : 0;
        for (unsigned j = 0; j < GF32V_WORDS; j++) {
            c.plane[i][j] = mask;
        }
        scalar >>= 1;
    }

    gf32v_mul(out, a, &c);
}

void gf32v_square(gf32v_array *out, const gf32v_array *a) {
    gf32v_mul(out, a, a);
}

// These might be an issue if out == a but we ball
void gf32v_mul_0x2(gf32v_array *out, const gf32v_array *a) {
    gf32v_rvv_mul_0x2(&out->plane[0][0], &a->plane[0][0], GF32V_WORDS);
}

void gf32v_mul_0x5(gf32v_array *out, const gf32v_array *a) {
    gf32v_rvv_mul_0x5(&out->plane[0][0], &a->plane[0][0], GF32V_WORDS);
}