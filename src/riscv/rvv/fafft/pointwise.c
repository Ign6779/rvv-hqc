#include "pointwise.h"
#include "gf32v_tower.h"

void fafft_pointwise_mul(gf32v_array *out, const gf32v_array *a, const gf32v_array *b) {
    gf32v_mul(out, a, b);
}