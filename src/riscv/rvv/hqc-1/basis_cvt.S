#include "rvv_macros.h"

    .text
    .align 2

    .globl fafft_rvv_xor_up_u8
    .type fafft_rvv_xor_up_u8, @function
fafft_rvv_xor_up_u8:
    beqz    a2, 2f

    add     t1, a0, a1        # dest pointer = poly + st
    add     t2, t1, a3        # src pointer = poly + st + diff

1:
    VSETVLI_E8_M8(t0, a2)

    vle8.v   v0, (t1)          # v0-v7
    vle8.v   v8, (t2)          # v8-v15
    vxor.vv v0, v0, v8
    vse8.v   v0, (t1)

    add     t1, t1, t0
    add     t2, t2, t0
    sub     a2, a2, t0
    bnez    a2, 1b

2:
    ret

    .size fafft_rvv_xor_up_u8, .-fafft_rvv_xor_up_u8


    .globl fafft_rvv_xor_down_u8
    .type fafft_rvv_xor_down_u8, @function
fafft_rvv_xor_down_u8:
    beqz    a2, 2f

    add     t1, a0, a1        # end-ish destination pointer
    add     t2, t1, a3        # end-ish source pointer

1:
    mv      t3, a2

    # Do not process more than diff bytes at once.
    # May have to remove beqz
    beqz    a3, 3f
    bleu    a2, a3, 3f
    mv      t3, a3

3:
    VSETVLI_E8_M8(t0, t3)

    sub     t1, t1, t0
    sub     t2, t2, t0

    vle8.v   v0, (t1)          # v0-v7
    vle8.v   v8, (t2)          # v8-v15
    vxor.vv v0, v0, v8
    vse8.v   v0, (t1)

    sub     a2, a2, t0
    bnez    a2, 1b

2:
    ret

    .size fafft_rvv_xor_down_u8, .-fafft_rvv_xor_down_u8