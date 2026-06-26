#include "rvv_macros.h"

    .text
    .align 2

.macro GF32V_MUL_0X2_PAIR
    vle64.v       v0, (t2)
    add         t2, t2, t5
    vle64.v       v8, (t2)

    vse64.v       v8, (t3)

    vxor.vv     v16, v0, v8
    add         t3, t3, t5
    vse64.v       v16, (t3)

    add         t2, t2, t5
    add         t3, t3, t5
.endm

.macro GF32V_MUL_0X5_BLOCK4
    vle64.v       v0, (t2)
    add         t2, t2, t5
    vle64.v       v8, (t2)
    add         t2, t2, t5
    vle64.v       v16, (t2)
    add         t2, t2, t5
    vle64.v       v24, (t2)

    vxor.vv     v16, v16, v24
    vxor.vv     v16, v16, v8
    vxor.vv     v24, v24, v0

    vse64.v       v24, (t3)
    add         t3, t3, t5
    vse64.v       v16, (t3)
    add         t3, t3, t5
    vse64.v       v0, (t3)
    add         t3, t3, t5
    vse64.v       v8, (t3)

    add         t2, t2, t5
    add         t3, t3, t5
.endm

    .globl gf32v_rvv_mul_0x2
    .type  gf32v_rvv_mul_0x2, @function
gf32v_rvv_mul_0x2:
    beqz        a2, 2f

    slli        t5, a2, 3
    mv          t6, zero
    mv          t4, a2

1:
    VSETVLI_E64_M8(t0, t4)
    slli        t1, t0, 3

    add         t2, a1, t6
    add         t3, a0, t6

    GF32V_MUL_0X2_PAIR
    GF32V_MUL_0X2_PAIR
    GF32V_MUL_0X2_PAIR
    GF32V_MUL_0X2_PAIR

    GF32V_MUL_0X2_PAIR
    GF32V_MUL_0X2_PAIR
    GF32V_MUL_0X2_PAIR
    GF32V_MUL_0X2_PAIR

    GF32V_MUL_0X2_PAIR
    GF32V_MUL_0X2_PAIR
    GF32V_MUL_0X2_PAIR
    GF32V_MUL_0X2_PAIR

    GF32V_MUL_0X2_PAIR
    GF32V_MUL_0X2_PAIR
    GF32V_MUL_0X2_PAIR
    GF32V_MUL_0X2_PAIR

    add         t6, t6, t1
    sub         t4, t4, t0
    bnez        t4, 1b

2:
    ret

    .size gf32v_rvv_mul_0x2, .-gf32v_rvv_mul_0x2

    .globl gf32v_rvv_mul_0x5
    .type  gf32v_rvv_mul_0x5, @function
gf32v_rvv_mul_0x5:
    beqz        a2, 2f

    slli        t5, a2, 3
    mv          t6, zero
    mv          t4, a2

1:
    VSETVLI_E64_M8(t0, t4)
    slli        t1, t0, 3

    add         t2, a1, t6
    add         t3, a0, t6

    GF32V_MUL_0X5_BLOCK4
    GF32V_MUL_0X5_BLOCK4

    GF32V_MUL_0X5_BLOCK4
    GF32V_MUL_0X5_BLOCK4

    GF32V_MUL_0X5_BLOCK4
    GF32V_MUL_0X5_BLOCK4

    GF32V_MUL_0X5_BLOCK4
    GF32V_MUL_0X5_BLOCK4

    add         t6, t6, t1
    sub         t4, t4, t0
    bnez        t4, 1b

2:
    ret

    .size gf32v_rvv_mul_0x5, .-gf32v_rvv_mul_0x5


    .extern gf32v_rvv_mul216
    .extern gf32v_rvv_mul216_0x8000

.macro GF32V_XOR16_BODY
    vle64.v       v0, (t2)
    vle64.v       v8, (t3)
    vxor.vv     v0, v0, v8
    vse64.v       v0, (a4)
    add         t2, t2, t5
    add         t3, t3, t5
    add         a4, a4, t5
.endm

.macro GF32V_XOR16_INPLACE_BODY
    vle64.v       v0, (a4)
    vle64.v       v8, (t3)
    vxor.vv     v0, v0, v8
    vse64.v       v0, (a4)
    add         a4, a4, t5
    add         t3, t3, t5
.endm

.Lgf32v_xor16:
    beqz        a3, 2f

    slli        t5, a3, 3
    mv          t6, zero
    mv          t4, a3

1:
    VSETVLI_E64_M8(t0, t4)
    slli        t1, t0, 3

    add         a4, a0, t6
    add         t2, a1, t6
    add         t3, a2, t6

    GF32V_XOR16_BODY
    GF32V_XOR16_BODY
    GF32V_XOR16_BODY
    GF32V_XOR16_BODY

    GF32V_XOR16_BODY
    GF32V_XOR16_BODY
    GF32V_XOR16_BODY
    GF32V_XOR16_BODY

    GF32V_XOR16_BODY
    GF32V_XOR16_BODY
    GF32V_XOR16_BODY
    GF32V_XOR16_BODY

    GF32V_XOR16_BODY
    GF32V_XOR16_BODY
    GF32V_XOR16_BODY
    GF32V_XOR16_BODY

    add         t6, t6, t1
    sub         t4, t4, t0
    bnez        t4, 1b

2:
    ret

.Lgf32v_xor16_inplace:
    beqz        a2, 2f

    slli        t5, a2, 3
    mv          t6, zero
    mv          t4, a2

1:
    VSETVLI_E64_M8(t0, t4)
    slli        t1, t0, 3

    add         a4, a0, t6
    add         t3, a1, t6

    GF32V_XOR16_INPLACE_BODY
    GF32V_XOR16_INPLACE_BODY
    GF32V_XOR16_INPLACE_BODY
    GF32V_XOR16_INPLACE_BODY

    GF32V_XOR16_INPLACE_BODY
    GF32V_XOR16_INPLACE_BODY
    GF32V_XOR16_INPLACE_BODY
    GF32V_XOR16_INPLACE_BODY

    GF32V_XOR16_INPLACE_BODY
    GF32V_XOR16_INPLACE_BODY
    GF32V_XOR16_INPLACE_BODY
    GF32V_XOR16_INPLACE_BODY

    GF32V_XOR16_INPLACE_BODY
    GF32V_XOR16_INPLACE_BODY
    GF32V_XOR16_INPLACE_BODY
    GF32V_XOR16_INPLACE_BODY

    add         t6, t6, t1
    sub         t4, t4, t0
    bnez        t4, 1b

2:
    ret

    .globl gf32v_rvv_mul
    .type  gf32v_rvv_mul, @function
gf32v_rvv_mul:
    beqz        a3, 9f

    addi        sp, sp, -80
    sd          ra, 72(sp)
    sd          s0, 64(sp)
    sd          s1, 56(sp)
    sd          s2, 48(sp)
    sd          s3, 40(sp)
    sd          s4, 32(sp)
    sd          s5, 24(sp)
    sd          s6, 16(sp)
    sd          s7, 8(sp)
    sd          s8, 0(sp)

    mv          s0, a0
    mv          s1, a1
    mv          s2, a2
    mv          s3, a3

    slli        s4, s3, 7
    slli        t0, s4, 1
    add         t0, t0, s4
    sub         sp, sp, t0

    mv          s5, sp
    add         s6, s5, s4
    add         s7, s6, s4
    add         s8, s0, s4

    mv          a0, s5
    mv          a1, s1
    add         a2, s1, s4
    mv          a3, s3
    call        .Lgf32v_xor16

    mv          a0, s6
    mv          a1, s2
    add         a2, s2, s4
    mv          a3, s3
    call        .Lgf32v_xor16

    mv          a0, s0
    mv          a1, s1
    mv          a2, s2
    mv          a3, s3
    call        gf32v_rvv_mul216

    mv          a0, s8
    mv          a1, s5
    mv          a2, s6
    mv          a3, s3
    call        gf32v_rvv_mul216

    mv          a0, s8
    mv          a1, s0
    mv          a2, s3
    call        .Lgf32v_xor16_inplace

    mv          a0, s7
    add         a1, s1, s4
    add         a2, s2, s4
    mv          a3, s3
    call        gf32v_rvv_mul216

    mv          a0, s5
    mv          a1, s7
    mv          a2, s3
    call        gf32v_rvv_mul216_0x8000

    mv          a0, s0
    mv          a1, s5
    mv          a2, s3
    call        .Lgf32v_xor16_inplace

    slli        t0, s4, 1
    add         t0, t0, s4
    add         sp, sp, t0

    ld          s8, 0(sp)
    ld          s7, 8(sp)
    ld          s6, 16(sp)
    ld          s5, 24(sp)
    ld          s4, 32(sp)
    ld          s3, 40(sp)
    ld          s2, 48(sp)
    ld          s1, 56(sp)
    ld          s0, 64(sp)
    ld          ra, 72(sp)
    addi        sp, sp, 80
    ret

9:
    ret

    .size gf32v_rvv_mul, .-gf32v_rvv_mul


    .extern gf32v_rvv_mul256
    .extern gf32v_rvv_mul256_0x80

.macro GF32V_XOR8_BODY
    vle64.v       v0, (t2)
    vle64.v       v8, (t3)
    vxor.vv     v0, v0, v8
    vse64.v       v0, (a4)
    add         t2, t2, t5
    add         t3, t3, t5
    add         a4, a4, t5
.endm

.macro GF32V_XOR8_INPLACE_BODY
    vle64.v       v0, (a4)
    vle64.v       v8, (t3)
    vxor.vv     v0, v0, v8
    vse64.v       v0, (a4)
    add         a4, a4, t5
    add         t3, t3, t5
.endm

.Lgf32v_xor8:
    beqz        a3, 2f

    slli        t5, a3, 3
    mv          t6, zero
    mv          t4, a3

1:
    VSETVLI_E64_M8(t0, t4)
    slli        t1, t0, 3

    add         a4, a0, t6
    add         t2, a1, t6
    add         t3, a2, t6

    GF32V_XOR8_BODY
    GF32V_XOR8_BODY
    GF32V_XOR8_BODY
    GF32V_XOR8_BODY
    GF32V_XOR8_BODY
    GF32V_XOR8_BODY
    GF32V_XOR8_BODY
    GF32V_XOR8_BODY

    add         t6, t6, t1
    sub         t4, t4, t0
    bnez        t4, 1b

2:
    ret

.Lgf32v_xor8_inplace:
    beqz        a2, 2f

    slli        t5, a2, 3
    mv          t6, zero
    mv          t4, a2

1:
    VSETVLI_E64_M8(t0, t4)
    slli        t1, t0, 3

    add         a4, a0, t6
    add         t3, a1, t6

    GF32V_XOR8_INPLACE_BODY
    GF32V_XOR8_INPLACE_BODY
    GF32V_XOR8_INPLACE_BODY
    GF32V_XOR8_INPLACE_BODY
    GF32V_XOR8_INPLACE_BODY
    GF32V_XOR8_INPLACE_BODY
    GF32V_XOR8_INPLACE_BODY
    GF32V_XOR8_INPLACE_BODY

    add         t6, t6, t1
    sub         t4, t4, t0
    bnez        t4, 1b

2:
    ret

    .globl gf32v_rvv_mul216
    .type  gf32v_rvv_mul216, @function
gf32v_rvv_mul216:
    beqz        a3, 9f

    addi        sp, sp, -80
    sd          ra, 72(sp)
    sd          s0, 64(sp)
    sd          s1, 56(sp)
    sd          s2, 48(sp)
    sd          s3, 40(sp)
    sd          s4, 32(sp)
    sd          s5, 24(sp)
    sd          s6, 16(sp)
    sd          s7, 8(sp)
    sd          s8, 0(sp)

    mv          s0, a0
    mv          s1, a1
    mv          s2, a2
    mv          s3, a3

    slli        s4, s3, 6
    slli        t0, s4, 1
    add         t0, t0, s4
    sub         sp, sp, t0

    mv          s5, sp
    add         s6, s5, s4
    add         s7, s6, s4
    add         s8, s0, s4

    mv          a0, s5
    mv          a1, s1
    add         a2, s1, s4
    mv          a3, s3
    call        .Lgf32v_xor8

    mv          a0, s6
    mv          a1, s2
    add         a2, s2, s4
    mv          a3, s3
    call        .Lgf32v_xor8

    mv          a0, s0
    mv          a1, s1
    mv          a2, s2
    mv          a3, s3
    call        gf32v_rvv_mul256

    mv          a0, s8
    mv          a1, s5
    mv          a2, s6
    mv          a3, s3
    call        gf32v_rvv_mul256

    mv          a0, s8
    mv          a1, s0
    mv          a2, s3
    call        .Lgf32v_xor8_inplace

    mv          a0, s7
    add         a1, s1, s4
    add         a2, s2, s4
    mv          a3, s3
    call        gf32v_rvv_mul256

    mv          a0, s5
    mv          a1, s7
    mv          a2, s3
    call        gf32v_rvv_mul256_0x80

    mv          a0, s0
    mv          a1, s5
    mv          a2, s3
    call        .Lgf32v_xor8_inplace

    slli        t0, s4, 1
    add         t0, t0, s4
    add         sp, sp, t0

    ld          s8, 0(sp)
    ld          s7, 8(sp)
    ld          s6, 16(sp)
    ld          s5, 24(sp)
    ld          s4, 32(sp)
    ld          s3, 40(sp)
    ld          s2, 48(sp)
    ld          s1, 56(sp)
    ld          s0, 64(sp)
    ld          ra, 72(sp)
    addi        sp, sp, 80
    ret

9:
    ret

    .size gf32v_rvv_mul216, .-gf32v_rvv_mul216

    .globl gf32v_rvv_mul216_0x8000
    .type  gf32v_rvv_mul216_0x8000, @function
gf32v_rvv_mul216_0x8000:
    beqz        a2, 9f

    addi        sp, sp, -64
    sd          ra, 56(sp)
    sd          s0, 48(sp)
    sd          s1, 40(sp)
    sd          s2, 32(sp)
    sd          s3, 24(sp)
    sd          s4, 16(sp)
    sd          s5, 8(sp)

    mv          s0, a0
    mv          s1, a1
    mv          s2, a2

    slli        s3, s2, 6
    sub         sp, sp, s3
    mv          s4, sp
    add         s5, s0, s3

    mv          a0, s5
    mv          a1, s1
    mv          a2, s2
    call        gf32v_rvv_mul256_0x80

    mv          a0, s4
    add         a1, s1, s3
    mv          a2, s2
    call        gf32v_rvv_mul256_0x80

    mv          a0, s5
    mv          a1, s4
    mv          a2, s2
    call        .Lgf32v_xor8_inplace

    mv          a0, s0
    mv          a1, s4
    mv          a2, s2
    call        gf32v_rvv_mul256_0x80

    add         sp, sp, s3

    ld          s5, 8(sp)
    ld          s4, 16(sp)
    ld          s3, 24(sp)
    ld          s2, 32(sp)
    ld          s1, 40(sp)
    ld          s0, 48(sp)
    ld          ra, 56(sp)
    addi        sp, sp, 64
    ret

9:
    ret

    .size gf32v_rvv_mul216_0x8000, .-gf32v_rvv_mul216_0x8000


    .extern gf32v_rvv_mul16
    .extern gf32v_rvv_mul16_8

.macro GF32V_XOR4_BODY
    vle64.v       v0, (t2)
    vle64.v       v8, (t3)
    vxor.vv     v0, v0, v8
    vse64.v       v0, (a4)
    add         t2, t2, t5
    add         t3, t3, t5
    add         a4, a4, t5
.endm

.macro GF32V_XOR4_INPLACE_BODY
    vle64.v       v0, (a4)
    vle64.v       v8, (t3)
    vxor.vv     v0, v0, v8
    vse64.v       v0, (a4)
    add         a4, a4, t5
    add         t3, t3, t5
.endm

.Lgf32v_xor4:
    beqz        a3, 2f
    slli        t5, a3, 3
    mv          t6, zero
    mv          t4, a3

1:
    VSETVLI_E64_M8(t0, t4)
    slli        t1, t0, 3

    add         a4, a0, t6
    add         t2, a1, t6
    add         t3, a2, t6

    GF32V_XOR4_BODY
    GF32V_XOR4_BODY
    GF32V_XOR4_BODY
    GF32V_XOR4_BODY

    add         t6, t6, t1
    sub         t4, t4, t0
    bnez        t4, 1b

2:
    ret

.Lgf32v_xor4_inplace:
    beqz        a2, 2f
    slli        t5, a2, 3
    mv          t6, zero
    mv          t4, a2

1:
    VSETVLI_E64_M8(t0, t4)
    slli        t1, t0, 3

    add         a4, a0, t6
    add         t3, a1, t6

    GF32V_XOR4_INPLACE_BODY
    GF32V_XOR4_INPLACE_BODY
    GF32V_XOR4_INPLACE_BODY
    GF32V_XOR4_INPLACE_BODY

    add         t6, t6, t1
    sub         t4, t4, t0
    bnez        t4, 1b

2:
    ret

    .globl gf32v_rvv_mul256
    .type  gf32v_rvv_mul256, @function
gf32v_rvv_mul256:
    beqz        a3, 9f

    addi        sp, sp, -80
    sd          ra, 72(sp)
    sd          s0, 64(sp)
    sd          s1, 56(sp)
    sd          s2, 48(sp)
    sd          s3, 40(sp)
    sd          s4, 32(sp)
    sd          s5, 24(sp)
    sd          s6, 16(sp)
    sd          s7, 8(sp)
    sd          s8, 0(sp)

    mv          s0, a0
    mv          s1, a1
    mv          s2, a2
    mv          s3, a3

    slli        s4, s3, 5
    slli        t0, s4, 1
    add         t0, t0, s4
    sub         sp, sp, t0

    mv          s5, sp
    add         s6, s5, s4
    add         s7, s6, s4
    add         s8, s0, s4

    mv          a0, s5
    mv          a1, s1
    add         a2, s1, s4
    mv          a3, s3
    call        .Lgf32v_xor4

    mv          a0, s6
    mv          a1, s2
    add         a2, s2, s4
    mv          a3, s3
    call        .Lgf32v_xor4

    mv          a0, s0
    mv          a1, s1
    mv          a2, s2
    mv          a3, s3
    call        gf32v_rvv_mul16

    mv          a0, s8
    mv          a1, s5
    mv          a2, s6
    mv          a3, s3
    call        gf32v_rvv_mul16

    mv          a0, s8
    mv          a1, s0
    mv          a2, s3
    call        .Lgf32v_xor4_inplace

    mv          a0, s7
    add         a1, s1, s4
    add         a2, s2, s4
    mv          a3, s3
    call        gf32v_rvv_mul16

    mv          a0, s5
    mv          a1, s7
    mv          a2, s3
    call        gf32v_rvv_mul16_8

    mv          a0, s0
    mv          a1, s5
    mv          a2, s3
    call        .Lgf32v_xor4_inplace

    slli        t0, s4, 1
    add         t0, t0, s4
    add         sp, sp, t0

    ld          s8, 0(sp)
    ld          s7, 8(sp)
    ld          s6, 16(sp)
    ld          s5, 24(sp)
    ld          s4, 32(sp)
    ld          s3, 40(sp)
    ld          s2, 48(sp)
    ld          s1, 56(sp)
    ld          s0, 64(sp)
    ld          ra, 72(sp)
    addi        sp, sp, 80
    ret

9:
    ret

    .size gf32v_rvv_mul256, .-gf32v_rvv_mul256

    .globl gf32v_rvv_mul256_0x80
    .type  gf32v_rvv_mul256_0x80, @function
gf32v_rvv_mul256_0x80:
    beqz        a2, 9f

    addi        sp, sp, -64
    sd          ra, 56(sp)
    sd          s0, 48(sp)
    sd          s1, 40(sp)
    sd          s2, 32(sp)
    sd          s3, 24(sp)
    sd          s4, 16(sp)
    sd          s5, 8(sp)

    mv          s0, a0
    mv          s1, a1
    mv          s2, a2

    slli        s3, s2, 5
    sub         sp, sp, s3
    mv          s4, sp
    add         s5, s0, s3

    mv          a0, s5
    mv          a1, s1
    mv          a2, s2
    call        gf32v_rvv_mul16_8

    mv          a0, s4
    add         a1, s1, s3
    mv          a2, s2
    call        gf32v_rvv_mul16_8

    mv          a0, s5
    mv          a1, s4
    mv          a2, s2
    call        .Lgf32v_xor4_inplace

    mv          a0, s0
    mv          a1, s4
    mv          a2, s2
    call        gf32v_rvv_mul16_8

    add         sp, sp, s3

    ld          s5, 8(sp)
    ld          s4, 16(sp)
    ld          s3, 24(sp)
    ld          s2, 32(sp)
    ld          s1, 40(sp)
    ld          s0, 48(sp)
    ld          ra, 56(sp)
    addi        sp, sp, 64
    ret

9:
    ret

    .size gf32v_rvv_mul256_0x80, .-gf32v_rvv_mul256_0x80

    .globl gf32v_rvv_mul16
    .type  gf32v_rvv_mul16, @function
gf32v_rvv_mul16:
    beqz        a3, 2f

    slli        t5, a3, 3
    mv          t6, zero
    mv          t4, a3

1:
    VSETVLI_E64_M2(t0, t4)
    slli        t1, t0, 3

    /* Load the 4 a-planes (a0..a3) and 4 b-planes (b0..b3).
     * LMUL=2 register groups: v0,v2,v4,v6 = a0,a1,a2,a3 ; v8,v10,v12,v14 = b0..b3.
     * The whole routine uses only the 16 even-numbered LMUL=2 groups v0..v30. */
    add         t2, a1, t6
    vle64.v     v0, (t2)            /* a0 */
    add         t2, t2, t5
    vle64.v     v2, (t2)            /* a1 */
    add         t2, t2, t5
    vle64.v     v4, (t2)            /* a2 */
    add         t2, t2, t5
    vle64.v     v6, (t2)            /* a3 */

    add         t3, a2, t6
    vle64.v     v8, (t3)            /* b0 */
    add         t3, t3, t5
    vle64.v     v10, (t3)           /* b1 */
    add         t3, t3, t5
    vle64.v     v12, (t3)           /* b2 */
    add         t3, t3, t5
    vle64.v     v14, (t3)           /* b3 */

    /* Karatsuba sums for the middle sub-product. */
    vxor.vv     v16, v0, v4         /* sa0 = a0^a2 */
    vxor.vv     v18, v2, v6         /* sa1 = a1^a3 */
    vxor.vv     v20, v8, v12        /* sb0 = b0^b2 */
    vxor.vv     v22, v10, v14       /* sb1 = b1^b3 */

    /* Middle GF(2^2) product (sa0,sa1)*(sb0,sb1) -> p2_0=v18, p2_1=v16.
     * Frees sa*/sb* (v16,v18,v20,v22) as it overwrites them last. */
    vxor.vv     v24, v20, v22       /* t  = sb0^sb1 */
    vand.vv     v26, v16, v20       /* sa0&sb0 */
    vand.vv     v28, v18, v22       /* sa1&sb1 */
    vand.vv     v30, v16, v22       /* sa0&sb1 */
    vand.vv     v16, v18, v24       /* sa1&t   (sa0 dead) */
    vxor.vv     v18, v26, v28       /* p2_0 = sa0sb0 ^ sa1sb1 */
    vxor.vv     v16, v30, v16       /* p2_1 = sa0sb1 ^ sa1t  */

    /* Low GF(2^2) product (a0,a1)*(b0,b1) -> p1_0=v22, p1_1=v20.
     * a0,a1,b0,b1 (v0,v2,v8,v10) dead afterwards. */
    vxor.vv     v24, v8, v10        /* t = b0^b1 */
    vand.vv     v26, v0, v8         /* a0&b0 */
    vand.vv     v28, v2, v10        /* a1&b1 */
    vand.vv     v30, v0, v10        /* a0&b1 */
    vand.vv     v20, v2, v24        /* a1&t */
    vxor.vv     v22, v26, v28       /* p1_0 */
    vxor.vv     v20, v30, v20       /* p1_1 */

    /* High GF(2^2) product (a2,a3)*(b2,b3) -> p3_0=v2, p3_1=v0.
     * Reuses the now-dead v0,v2 for the results. */
    vxor.vv     v24, v12, v14       /* t = b2^b3 */
    vand.vv     v26, v4, v12        /* a2&b2 */
    vand.vv     v28, v6, v14        /* a3&b3 */
    vand.vv     v30, v4, v14        /* a2&b3 */
    vand.vv     v0,  v6, v24        /* a3&t */
    vxor.vv     v2,  v26, v28       /* p3_0 */
    vxor.vv     v0,  v30, v0        /* p3_1 */

    /* Combine (same as the LMUL=1 version):
     *   out0 = p1_0 ^ p3_1
     *   out1 = p1_1 ^ p3_1 ^ p3_0
     *   out2 = p2_0 ^ p1_0
     *   out3 = p2_1 ^ p1_1
     * p1_0=v22 p1_1=v20 p2_0=v18 p2_1=v16 p3_0=v2 p3_1=v0. */
    add         a4, a0, t6

    vxor.vv     v24, v22, v0        /* out0 */
    vse64.v     v24, (a4)
    add         a4, a4, t5

    vxor.vv     v26, v20, v0
    vxor.vv     v26, v26, v2        /* out1 */
    vse64.v     v26, (a4)
    add         a4, a4, t5

    vxor.vv     v28, v18, v22       /* out2 */
    vse64.v     v28, (a4)
    add         a4, a4, t5

    vxor.vv     v30, v16, v20       /* out3 */
    vse64.v     v30, (a4)

    add         t6, t6, t1
    sub         t4, t4, t0
    bnez        t4, 1b

2:
    ret

    .size gf32v_rvv_mul16, .-gf32v_rvv_mul16


    .globl gf32v_rvv_mul16_8
    .type  gf32v_rvv_mul16_8, @function
gf32v_rvv_mul16_8:
    beqz        a2, 2f

    slli        t5, a2, 3
    mv          t6, zero
    mv          t4, a2

1:
    VSETVLI_E64_M4(t0, t4)
    slli        t1, t0, 3

    /* LMUL=4 groups: v0,v4,v8,v12 = a0,a1,a2,a3 ; temps v16,v20,v24 (7 of 8 groups). */
    add         t2, a1, t6
    vle64.v     v0, (t2)            /* a0 */
    add         t2, t2, t5
    vle64.v     v4, (t2)            /* a1 */
    add         t2, t2, t5
    vle64.v     v8, (t2)            /* a2 */
    add         t2, t2, t5
    vle64.v     v12, (t2)           /* a3 */

    vxor.vv     v16, v8, v12        /* out0 = a2^a3 */
    vxor.vv     v20, v4, v12        /* out2 = a1^a3 */
    vxor.vv     v24, v0, v4         /* a0^a1 */
    vxor.vv     v24, v24, v16       /* out3 = a0^a1^a2^a3 */

    add         a4, a0, t6
    vse64.v     v16, (a4)           /* out0 */
    add         a4, a4, t5
    vse64.v     v8, (a4)            /* out1 = a2 */
    add         a4, a4, t5
    vse64.v     v20, (a4)           /* out2 */
    add         a4, a4, t5
    vse64.v     v24, (a4)           /* out3 */

    add         t6, t6, t1
    sub         t4, t4, t0
    bnez        t4, 1b

2:
    ret

    .size gf32v_rvv_mul16_8, .-gf32v_rvv_mul16_8


    .globl gf32v_rvv_mul_gf16
    .type  gf32v_rvv_mul_gf16, @function
gf32v_rvv_mul_gf16:
    beqz        a3, 2f

    addi        sp, sp, -64
    sd          ra, 56(sp)
    sd          s0, 48(sp)
    sd          s1, 40(sp)
    sd          s2, 32(sp)
    sd          s3, 24(sp)
    sd          s4, 16(sp)
    sd          s5, 8(sp)
    sd          s6, 0(sp)

    mv          s0, a0          # out
    mv          s1, a1          # a
    mv          s2, a2          # b_gf16
    mv          s3, a3          # nwords

    slli        s4, s3, 3      # bytes per plane = nwords * 8
    slli        s5, s4, 2      # bytes per GF16 chunk = 4 planes
    li          s6, 8          # 32 / 4 chunks

1:
    mv          a0, s0
    mv          a1, s1
    mv          a2, s2
    mv          a3, s3
    call        gf32v_rvv_mul16

    add         s0, s0, s5
    add         s1, s1, s5
    addi        s6, s6, -1
    bnez        s6, 1b

    ld          s6, 0(sp)
    ld          s5, 8(sp)
    ld          s4, 16(sp)
    ld          s3, 24(sp)
    ld          s2, 32(sp)
    ld          s1, 40(sp)
    ld          s0, 48(sp)
    ld          ra, 56(sp)
    addi        sp, sp, 64

2:
    ret

    .size gf32v_rvv_mul_gf16, .-gf32v_rvv_mul_gf16


    .globl gf32v_rvv_mul_gf256
    .type  gf32v_rvv_mul_gf256, @function
gf32v_rvv_mul_gf256:
    beqz        a3, 2f

    addi        sp, sp, -64
    sd          ra, 56(sp)
    sd          s0, 48(sp)
    sd          s1, 40(sp)
    sd          s2, 32(sp)
    sd          s3, 24(sp)
    sd          s4, 16(sp)
    sd          s5, 8(sp)
    sd          s6, 0(sp)

    mv          s0, a0          # out
    mv          s1, a1          # a
    mv          s2, a2          # b_gf256
    mv          s3, a3          # nwords

    slli        s4, s3, 3      # bytes per plane = nwords * 8
    slli        s5, s4, 3      # bytes per GF256 chunk = 8 planes
    li          s6, 4          # 32 / 8 chunks

1:
    mv          a0, s0
    mv          a1, s1
    mv          a2, s2
    mv          a3, s3
    call        gf32v_rvv_mul256

    add         s0, s0, s5
    add         s1, s1, s5
    addi        s6, s6, -1
    bnez        s6, 1b

    ld          s6, 0(sp)
    ld          s5, 8(sp)
    ld          s4, 16(sp)
    ld          s3, 24(sp)
    ld          s2, 32(sp)
    ld          s1, 40(sp)
    ld          s0, 48(sp)
    ld          ra, 56(sp)
    addi        sp, sp, 64

2:
    ret

    .size gf32v_rvv_mul_gf256, .-gf32v_rvv_mul_gf256


    .globl gf32v_rvv_mul_gf216
    .type  gf32v_rvv_mul_gf216, @function
gf32v_rvv_mul_gf216:
    beqz        a3, 2f

    addi        sp, sp, -64
    sd          ra, 56(sp)
    sd          s0, 48(sp)
    sd          s1, 40(sp)
    sd          s2, 32(sp)
    sd          s3, 24(sp)
    sd          s4, 16(sp)
    sd          s5, 8(sp)
    sd          s6, 0(sp)

    mv          s0, a0          # out
    mv          s1, a1          # a
    mv          s2, a2          # b_gf216
    mv          s3, a3          # nwords

    slli        s4, s3, 3      # bytes per plane = nwords * 8
    slli        s5, s4, 4      # bytes per GF216 chunk = 16 planes
    li          s6, 2          # 32 / 16 chunks

1:
    mv          a0, s0
    mv          a1, s1
    mv          a2, s2
    mv          a3, s3
    call        gf32v_rvv_mul216

    add         s0, s0, s5
    add         s1, s1, s5
    addi        s6, s6, -1
    bnez        s6, 1b

    ld          s6, 0(sp)
    ld          s5, 8(sp)
    ld          s4, 16(sp)
    ld          s3, 24(sp)
    ld          s2, 32(sp)
    ld          s1, 40(sp)
    ld          s0, 48(sp)
    ld          ra, 56(sp)
    addi        sp, sp, 64

2:
    ret

    .size gf32v_rvv_mul_gf216, .-gf32v_rvv_mul_gf216