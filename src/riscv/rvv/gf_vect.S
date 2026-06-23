#include "rvv_macros.h"

    .text
    .align 2

/* Carryless shift-add of bit \sh of b (v8) into acc v12, base a in v4. */
.macro CLMUL_STEP sh
    vsrl.vi v16, v8, \sh        /* bit \sh of b -> low bit          */
    vand.vi v16, v16, 1
    vrsub.vx v16, v16, x0       /* 0 - bit -> 0x0000/0xFFFF mask    */
    vsll.vi v20, v4, \sh        /* a << \sh                         */
    vand.vv v20, v20, v16       /* keep only on selected lanes      */
    vxor.vv v12, v12, v20
.endm

/* Fold high bit \bit of v12 with reduction constant \cst = x^\bit mod 0x11D. */
.macro REDUCE_STEP bit, cst
    vsrl.vi v16, v12, \bit
    vand.vi v16, v16, 1
    vrsub.vx v16, v16, x0
    li      t2, \cst
    vand.vx v16, v16, t2
    vxor.vv v12, v12, v16
.endm

/* Full mod-0x11D reduction of v12 (degree <= 14 -> < 8); result masked to a byte. */
.macro REDUCE_ALL
    REDUCE_STEP 8,  0x1d
    REDUCE_STEP 9,  0x3a
    REDUCE_STEP 10, 0x74
    REDUCE_STEP 11, 0xe8
    REDUCE_STEP 12, 0xcd
    REDUCE_STEP 13, 0x87
    REDUCE_STEP 14, 0x13
    vand.vx v12, v12, t3        /* t3 = 0xFF */
.endm

/* Carryless product of v4 (a) and v8 (b) -> v12 (unreduced, 15-bit). a,b pre-masked. */
.macro CLMUL_ALL
    vmv.v.i v12, 0
    CLMUL_STEP 0
    CLMUL_STEP 1
    CLMUL_STEP 2
    CLMUL_STEP 3
    CLMUL_STEP 4
    CLMUL_STEP 5
    CLMUL_STEP 6
    CLMUL_STEP 7
.endm


/* void gf_carryless_mul_vect(uint16_t *out, const uint16_t *a, const uint16_t *b, size_t n) */
    .globl gf_carryless_mul_vect
    .type  gf_carryless_mul_vect, @function
gf_carryless_mul_vect:
    beqz    a3, 2f
    li      t3, 0xFF
1:
    VSETVLI_E16_M4(t0, a3)
    vle16.v v4, (a1)
    vle16.v v8, (a2)
    vand.vx v4, v4, t3
    vand.vx v8, v8, t3
    CLMUL_ALL
    vse16.v v12, (a0)
    slli    t1, t0, 1
    add     a0, a0, t1
    add     a1, a1, t1
    add     a2, a2, t1
    sub     a3, a3, t0
    bnez    a3, 1b
2:
    ret
    .size gf_carryless_mul_vect, .-gf_carryless_mul_vect


/* void gf_reduce_vect(uint16_t *out, const uint16_t *in, size_t n) */
    .globl gf_reduce_vect
    .type  gf_reduce_vect, @function
gf_reduce_vect:
    beqz    a2, 2f
    li      t3, 0xFF
1:
    VSETVLI_E16_M4(t0, a2)
    vle16.v v12, (a1)
    REDUCE_ALL
    vse16.v v12, (a0)
    slli    t1, t0, 1
    add     a0, a0, t1
    add     a1, a1, t1
    sub     a2, a2, t0
    bnez    a2, 1b
2:
    ret
    .size gf_reduce_vect, .-gf_reduce_vect


/* void gf_mul_vect(uint16_t *out, const uint16_t *a, const uint16_t *b, size_t n) */
    .globl gf_mul_vect
    .type  gf_mul_vect, @function
gf_mul_vect:
    beqz    a3, 2f
    li      t3, 0xFF
1:
    VSETVLI_E16_M4(t0, a3)
    vle16.v v4, (a1)
    vle16.v v8, (a2)
    vand.vx v4, v4, t3
    vand.vx v8, v8, t3
    CLMUL_ALL
    REDUCE_ALL
    vse16.v v12, (a0)
    slli    t1, t0, 1
    add     a0, a0, t1
    add     a1, a1, t1
    add     a2, a2, t1
    sub     a3, a3, t0
    bnez    a3, 1b
2:
    ret
    .size gf_mul_vect, .-gf_mul_vect


/* void gf_square_vect(uint16_t *out, const uint16_t *a, size_t n) */
    .globl gf_square_vect
    .type  gf_square_vect, @function
gf_square_vect:
    beqz    a2, 2f
    li      t3, 0xFF
1:
    VSETVLI_E16_M4(t0, a2)
    vle16.v v4, (a1)
    vand.vx v12, v4, t3        /* x = a & 0xFF                       */
    vsll.vi v16, v12, 4        /* spread 8 bits to even positions:   */
    vor.vv  v12, v12, v16      /*   x = (x | x<<4) & 0x0F0F           */
    li      t2, 0x0F0F
    vand.vx v12, v12, t2
    vsll.vi v16, v12, 2
    vor.vv  v12, v12, v16      /*   x = (x | x<<2) & 0x3333           */
    li      t2, 0x3333
    vand.vx v12, v12, t2
    vsll.vi v16, v12, 1
    vor.vv  v12, v12, v16      /*   x = (x | x<<1) & 0x5555           */
    li      t2, 0x5555
    vand.vx v12, v12, t2
    REDUCE_ALL
    vse16.v v12, (a0)
    slli    t1, t0, 1
    add     a0, a0, t1
    add     a1, a1, t1
    sub     a2, a2, t0
    bnez    a2, 1b
2:
    ret
    .size gf_square_vect, .-gf_square_vect