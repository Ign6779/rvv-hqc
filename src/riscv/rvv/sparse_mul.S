#include "rvv_macros.h"

/*
 * RVV kernels for sparse-by-dense multiplication (see sparse_mul.c).
 *
 * All kernels are vector-length agnostic: each strip-mines over its word
 * count with VSETVLI at LMUL=8, so they run unchanged on any VLEN.
 * Mnemonics match the rest of the RVV code (vle64.v/vse64.v/vxor.vv/...);
 * only VSETVLI is emitted through rvv_macros.h for the 0.7.1 vtype encoding.
 *
 * Register convention (RISC-V): a0-a3 = args, t0 = granted vl, t1-t3 = temps.
 * Vector groups v0, v8, v16 are LMUL=8 register groups.
 */

    .text
    .align 2

/*
 * void sparse_rvv_rotate_xor(uint64_t *acc, const uint64_t *win,
 *                            uint64_t bo, uint64_t nwords);
 *
 * acc[j] ^= (win[j] >> bo) | (win[j+1] << (64 - bo))      for bo in [1,63]
 * acc[j] ^= win[j]                                         for bo == 0
 *
 * This is one cyclic rotation of the dense operand XOR-accumulated into acc.
 */
    .globl sparse_rvv_rotate_xor
    .type  sparse_rvv_rotate_xor, @function
sparse_rvv_rotate_xor:
    beqz    a3, 9f
    beqz    a2, 5f              # bo == 0 -> no carry, plain XOR

    li      t2, 64
    sub     t2, t2, a2          # t2 = inv = 64 - bo

1:
    VSETVLI_E64_M8(t0, a3)
    vle64.v   v0, (a1)            # win[j ..]
    addi    t3, a1, 8
    vle64.v   v8, (t3)            # win[j+1 ..]
    vsrl.vx v0, v0, a2          # win[j]   >> bo
    vsll.vx v8, v8, t2          # win[j+1] << inv
    vor.vv  v0, v0, v8
    vle64.v   v16, (a0)           # acc[j ..]
    vxor.vv v0, v0, v16
    vse64.v   v0, (a0)

    slli    t1, t0, 3
    add     a0, a0, t1
    add     a1, a1, t1
    sub     a3, a3, t0
    bnez    a3, 1b
    ret

5:                              # bo == 0
    VSETVLI_E64_M8(t0, a3)
    vle64.v   v0, (a1)
    vle64.v   v8, (a0)
    vxor.vv v0, v0, v8
    vse64.v   v0, (a0)

    slli    t1, t0, 3
    add     a0, a0, t1
    add     a1, a1, t1
    sub     a3, a3, t0
    bnez    a3, 5b
9:
    ret

    .size sparse_rvv_rotate_xor, .-sparse_rvv_rotate_xor


/*
 * void sparse_rvv_or_shl(uint64_t *dst, const uint64_t *src,
 *                        uint64_t sh, uint64_t nwords);
 *
 * dst[j] |= src[j] << sh        (doubled-buffer build, low part of each word)
 */
    .globl sparse_rvv_or_shl
    .type  sparse_rvv_or_shl, @function
sparse_rvv_or_shl:
    beqz    a3, 2f
1:
    VSETVLI_E64_M8(t0, a3)
    vle64.v   v0, (a1)
    vsll.vx v0, v0, a2
    vle64.v   v8, (a0)
    vor.vv  v0, v0, v8
    vse64.v   v0, (a0)

    slli    t1, t0, 3
    add     a0, a0, t1
    add     a1, a1, t1
    sub     a3, a3, t0
    bnez    a3, 1b
2:
    ret

    .size sparse_rvv_or_shl, .-sparse_rvv_or_shl


/*
 * void sparse_rvv_or_shr(uint64_t *dst, const uint64_t *src,
 *                        uint64_t sh, uint64_t nwords);
 *
 * dst[j] |= src[j] >> sh        (doubled-buffer build, carry into next word)
 */
    .globl sparse_rvv_or_shr
    .type  sparse_rvv_or_shr, @function
sparse_rvv_or_shr:
    beqz    a3, 2f
1:
    VSETVLI_E64_M8(t0, a3)
    vle64.v   v0, (a1)
    vsrl.vx v0, v0, a2
    vle64.v   v8, (a0)
    vor.vv  v0, v0, v8
    vse64.v   v0, (a0)

    slli    t1, t0, 3
    add     a0, a0, t1
    add     a1, a1, t1
    sub     a3, a3, t0
    bnez    a3, 1b
2:
    ret

    .size sparse_rvv_or_shr, .-sparse_rvv_or_shr


/*
 * void sparse_rvv_xor(uint64_t *dst, const uint64_t *src, uint64_t nwords);
 *
 * dst[j] ^= src[j]              (merge of the multiple accumulators)
 */
    .globl sparse_rvv_xor
    .type  sparse_rvv_xor, @function
sparse_rvv_xor:
    beqz    a2, 2f
1:
    VSETVLI_E64_M8(t0, a2)
    vle64.v   v0, (a0)
    vle64.v   v8, (a1)
    vxor.vv v0, v0, v8
    vse64.v   v0, (a0)

    slli    t1, t0, 3
    add     a0, a0, t1
    add     a1, a1, t1
    sub     a2, a2, t0
    bnez    a2, 1b
2:
    ret

    .size sparse_rvv_xor, .-sparse_rvv_xor
