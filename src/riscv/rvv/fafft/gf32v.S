#include "rvv_macros.h"

    .text
    .align 2

    .globl gf32v_rvv_zero
    .type gf32v_rvv_zero, @function
gf32v_rvv_zero:
    beqz    a1, 2f

1:
    VSETVLI_E64_M8(t0, a1)

    vmv.v.i v0, 0
    vse.v   v0, (a0)

    slli    t1, t0, 3
    add     a0, a0, t1
    sub     a1, a1, t0
    bnez    a1, 1b

2:
    ret

    .size gf32v_rvv_zero, .-gf32v_rvv_zero


    .globl gf32v_rvv_copy
    .type gf32v_rvv_copy, @function
gf32v_rvv_copy:
    beqz    a2, 2f

1:
    VSETVLI_E64_M8(t0, a2)

    vle.v   v0, (a1)
    vse.v   v0, (a0)

    slli    t1, t0, 3
    add     a0, a0, t1
    add     a1, a1, t1
    sub     a2, a2, t0
    bnez    a2, 1b

2:
    ret

    .size gf32v_rvv_copy, .-gf32v_rvv_copy


    .globl gf32v_rvv_xor
    .type gf32v_rvv_xor, @function
gf32v_rvv_xor:
    beqz    a3, 2f

1:
    VSETVLI_E64_M8(t0, a3)

    vle.v   v0, (a1)
    vle.v   v8, (a2)
    vxor.vv v0, v0, v8
    vse.v   v0, (a0)

    slli    t1, t0, 3
    add     a0, a0, t1
    add     a1, a1, t1
    add     a2, a2, t1
    sub     a3, a3, t0
    bnez    a3, 1b

2:
    ret

    .size gf32v_rvv_xor, .-gf32v_rvv_xor


    .globl gf32v_rvv_xor_inplace
    .type gf32v_rvv_xor_inplace, @function
gf32v_rvv_xor_inplace:
    beqz    a2, 2f

1:
    VSETVLI_E64_M8(t0, a2)

    vle.v   v0, (a0)
    vle.v   v8, (a1)
    vxor.vv v0, v0, v8
    vse.v   v0, (a0)

    slli    t1, t0, 3
    add     a0, a0, t1
    add     a1, a1, t1
    sub     a2, a2, t0
    bnez    a2, 1b

2:
    ret

    .size gf32v_rvv_xor_inplace, .-gf32v_rvv_xor_inplace