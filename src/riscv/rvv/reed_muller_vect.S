#include "rvv_macros.h"

    .text
    .align 2

/* void rm_encode_vect(uint32_t *temp, const uint32_t *msg, long n1) */
    .globl rm_encode_vect
    .type  rm_encode_vect, @function
rm_encode_vect:
    beqz    a2, 2f
    li      t1, 0xAAAAAAAA
    li      t2, 0xCCCCCCCC
    li      t3, 0xF0F0F0F0
    li      t4, 0xFF00FF00
    li      t5, 0xFFFF0000
    slli    t6, a2, 2
    mv      a3, a2
    mv      a4, a0
    mv      a5, a1
1:
    VSETVLI_E32_M4(t0, a3)
    vle32.v v4, (a5)

    vsrl.vi v8, v4, 7
    vand.vi v8, v8, 1
    vrsub.vx v12, v8, x0

    vand.vi v8, v4, 1
    vrsub.vx v8, v8, x0
    vand.vx v8, v8, t1
    vxor.vv v12, v12, v8

    vsrl.vi v8, v4, 1
    vand.vi v8, v8, 1
    vrsub.vx v8, v8, x0
    vand.vx v8, v8, t2
    vxor.vv v12, v12, v8

    vsrl.vi v8, v4, 2
    vand.vi v8, v8, 1
    vrsub.vx v8, v8, x0
    vand.vx v8, v8, t3
    vxor.vv v12, v12, v8

    vsrl.vi v8, v4, 3
    vand.vi v8, v8, 1
    vrsub.vx v8, v8, x0
    vand.vx v8, v8, t4
    vxor.vv v12, v12, v8

    vsrl.vi v8, v4, 4
    vand.vi v8, v8, 1
    vrsub.vx v8, v8, x0
    vand.vx v8, v8, t5
    vxor.vv v12, v12, v8

    vsrl.vi v8, v4, 5
    vand.vi v8, v8, 1
    vrsub.vx v8, v8, x0
    vxor.vv v16, v12, v8

    vsrl.vi v20, v4, 6
    vand.vi v20, v20, 1
    vrsub.vx v20, v20, x0
    vxor.vv v24, v16, v20

    vxor.vv v20, v24, v8

    vse32.v v12, (a4)
    add     a6, a4, t6
    vse32.v v16, (a6)
    add     a6, a6, t6
    vse32.v v20, (a6)
    add     a6, a6, t6
    vse32.v v24, (a6)

    slli    a6, t0, 2
    add     a4, a4, a6
    add     a5, a5, a6
    sub     a3, a3, t0
    bnez    a3, 1b
2:
    ret
    .size rm_encode_vect, .-rm_encode_vect


/* void rm_hadamard_vect(int16_t *a, int16_t *b, long n1) ; result ends in b */
    .globl rm_hadamard_vect
    .type  rm_hadamard_vect, @function
rm_hadamard_vect:
    addi    sp, sp, -56
    sd      ra, 48(sp)
    sd      s0, 40(sp)
    sd      s1, 32(sp)
    sd      s2, 24(sp)
    sd      s3, 16(sp)
    sd      s4, 8(sp)
    sd      s5, 0(sp)

    mv      s0, a0
    mv      s1, a1
    slli    s2, a2, 1
    mv      s3, a2
    li      s5, 0
.Lpass:
    li      s4, 0
.Lbf:
    slli    a3, s4, 1
    mul     a3, a3, s2
    add     t2, s0, a3
    add     t3, t2, s2
    mul     a3, s4, s2
    add     t4, s1, a3
    addi    a4, s4, 64
    mul     a4, a4, s2
    add     t5, s1, a4
    mv      t6, s3
.Lstrip:
    VSETVLI_E16_M8(t0, t6)
    vle16.v v0, (t2)
    vle16.v v8, (t3)
    vadd.vv v16, v0, v8
    vse16.v v16, (t4)
    vsub.vv v24, v0, v8
    vse16.v v24, (t5)
    slli    t1, t0, 1
    add     t2, t2, t1
    add     t3, t3, t1
    add     t4, t4, t1
    add     t5, t5, t1
    sub     t6, t6, t0
    bnez    t6, .Lstrip
    addi    s4, s4, 1
    li      a3, 64
    blt     s4, a3, .Lbf
    mv      a3, s0
    mv      s0, s1
    mv      s1, a3
    addi    s5, s5, 1
    li      a3, 7
    blt     s5, a3, .Lpass

    ld      ra, 48(sp)
    ld      s0, 40(sp)
    ld      s1, 32(sp)
    ld      s2, 24(sp)
    ld      s3, 16(sp)
    ld      s4, 8(sp)
    ld      s5, 0(sp)
    addi    sp, sp, 56
    ret
    .size rm_hadamard_vect, .-rm_hadamard_vect


/* void rm_find_peaks_vect(int16_t *out, const int16_t *transform, long n1) */
    .globl rm_find_peaks_vect
    .type  rm_find_peaks_vect, @function
rm_find_peaks_vect:
    beqz    a2, 2f
    slli    t6, a2, 1
    li      t5, 128
    mv      a3, a2
    mv      a4, a1
    mv      a5, a0
1:
    VSETVLI_E16_M4(t0, a3)
    vmv.v.i v12, 0
    vmv.v.i v16, 0
    vmv.v.i v20, 0
    mv      a6, a4
    li      t1, 0
.Lpos:
    vle16.v v4, (a6)
    vrsub.vx v8, v4, x0
    vmslt.vx v0, v4, x0
    vmerge.vvm v24, v4, v8, v0
    vmslt.vv v0, v12, v24
    vmerge.vvm v16, v16, v4, v0
    vmerge.vxm v20, v20, t1, v0
    vmerge.vvm v12, v12, v24, v0
    add     a6, a6, t6
    addi    t1, t1, 1
    blt     t1, t5, .Lpos
    vmsgt.vx v0, v16, x0
    vor.vx  v24, v20, t5
    vmerge.vvm v20, v20, v24, v0
    vse16.v v20, (a5)
    slli    a7, t0, 1
    add     a4, a4, a7
    add     a5, a5, a7
    sub     a3, a3, t0
    bnez    a3, 1b
2:
    ret
    .size rm_find_peaks_vect, .-rm_find_peaks_vect
