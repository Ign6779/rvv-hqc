#include "rvv_macros.h"

    .text
    .align 2

    .globl fafft_plane_copy
    .type fafft_plane_copy, @function
fafft_plane_copy:
    # void fafft_plane_copy(uint64_t *dst, const uint64_t *src)
    # Copies GF32V_WORDS=32 u64s. Loop required: VLEN determines how many
    # elements fit per iteration.
    li      t2, 32
1:
    VSETVLI_E64_M8(t0, t2)
    vle64.v  v0, (a1)
    vse64.v  v0, (a0)
    slli    t1, t0, 3
    add     a0, a0, t1
    add     a1, a1, t1
    sub     t2, t2, t0
    bnez    t2, 1b
    ret

    .size fafft_plane_copy, .-fafft_plane_copy


    .globl fafft_plane_xor
    .type fafft_plane_xor, @function
fafft_plane_xor:
    # void fafft_plane_xor(uint64_t *dst, const uint64_t *src)
    # dst ^= src, GF32V_WORDS=32 u64s.
    li      t2, 32
1:
    VSETVLI_E64_M8(t0, t2)
    vle64.v  v0, (a0)
    vle64.v  v8, (a1)
    vxor.vv v0, v0, v8
    vse64.v  v0, (a0)
    slli    t1, t0, 3
    add     a0, a0, t1
    add     a1, a1, t1
    sub     t2, t2, t0
    bnez    t2, 1b
    ret

    .size fafft_plane_xor, .-fafft_plane_xor
