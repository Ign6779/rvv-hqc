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
    VSETVLI_E64_M1(t0, t4)
    slli        t1, t0, 3

    add         t2, a1, t6
    vle64.v       v0, (t2)
    add         t2, t2, t5
    vle64.v       v1, (t2)
    add         t2, t2, t5
    vle64.v       v2, (t2)
    add         t2, t2, t5
    vle64.v       v3, (t2)

    add         t3, a2, t6
    vle64.v       v4, (t3)
    add         t3, t3, t5
    vle64.v       v5, (t3)
    add         t3, t3, t5
    vle64.v       v6, (t3)
    add         t3, t3, t5
    vle64.v       v7, (t3)

    vxor.vv     v8, v0, v2
    vxor.vv     v9, v1, v3
    vxor.vv     v10, v4, v6
    vxor.vv     v11, v5, v7

    vxor.vv     v12, v4, v5
    vand.vv     v13, v0, v4
    vand.vv     v14, v1, v5
    vxor.vv     v20, v13, v14
    vand.vv     v15, v0, v5
    vand.vv     v16, v1, v12
    vxor.vv     v21, v15, v16

    vxor.vv     v12, v10, v11
    vand.vv     v13, v8, v10
    vand.vv     v14, v9, v11
    vxor.vv     v22, v13, v14
    vand.vv     v15, v8, v11
    vand.vv     v16, v9, v12
    vxor.vv     v23, v15, v16

    vxor.vv     v12, v6, v7
    vand.vv     v13, v2, v6
    vand.vv     v14, v3, v7
    vxor.vv     v27, v13, v14
    vand.vv     v15, v2, v7
    vand.vv     v16, v3, v12
    vxor.vv     v30, v15, v16

    add         a4, a0, t6

    vxor.vv     v31, v20, v30
    vse64.v       v31, (a4)
    add         a4, a4, t5

    vxor.vv     v31, v21, v30
    vxor.vv     v31, v31, v27
    vse64.v       v31, (a4)
    add         a4, a4, t5

    vxor.vv     v31, v22, v20
    vse64.v       v31, (a4)
    add         a4, a4, t5

    vxor.vv     v31, v23, v21
    vse64.v       v31, (a4)

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
    VSETVLI_E64_M1(t0, t4)
    slli        t1, t0, 3

    add         t2, a1, t6
    vle64.v       v0, (t2)
    add         t2, t2, t5
    vle64.v       v1, (t2)
    add         t2, t2, t5
    vle64.v       v2, (t2)
    add         t2, t2, t5
    vle64.v       v3, (t2)

    vxor.vv     v4, v2, v3
    vxor.vv     v5, v1, v3
    vxor.vv     v6, v0, v1
    vxor.vv     v6, v6, v4

    add         a4, a0, t6
    vse64.v       v4, (a4)
    add         a4, a4, t5
    vse64.v       v2, (a4)
    add         a4, a4, t5
    vse64.v       v5, (a4)
    add         a4, a4, t5
    vse64.v       v6, (a4)

    add         t6, t6, t1
    sub         t4, t4, t0
    bnez        t4, 1b

2:
    ret

    .size gf32v_rvv_mul16_8, .-gf32v_rvv_mul16_8