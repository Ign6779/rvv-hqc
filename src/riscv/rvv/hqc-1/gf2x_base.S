#include "gf2x_macros.inc"

    .text
    .extern rvv_kara96_mul
    .globl rvv_toom_base_mul
    .type rvv_toom_base_mul, @function

# void rvv_toom_base_mul(uint64_t *out, const uint64_t *a, const uint64_t *b, size_t nwords)
# a0 = out
# a1 = a
# a2 = b
# a3 = nwords
rvv_toom_base_mul:
    addi sp, sp, -16
    sd ra, 8(sp)
    sd s0, 0(sp)

    call rvv_kara96_mul

    ld s0, 0(sp)
    ld ra, 8(sp)
    addi sp, sp, 16
    ret

    .size rvv_toom_base_mul, .-rvv_toom_base_mul