#include "rvv_macros.h"

    .text
    .align 2

/*
 * Reed-Solomon syndrome computation, "option B" (no carry-less multiply).
 *
 * Each syndrome i is   synd[i] = cdw[0] ^ XOR_p cdw[p+1] . alpha^(i*(p+1))   in GF(2^8).
 * One operand of every product (cdw[p+1]) is a *scalar* byte that is the same
 * for all syndromes, so the multiply degenerates to: for every set bit k of
 * cdw[p+1], XOR in the constant vector (alpha^(i*(p+1)) . x^k mod 0x11D).
 *
 * Those constants are precomputed (by the C caller) into a flat table laid out
 * as  tbl[(p*8 + k)*N + i],  i.e. for position p and bit k a contiguous run of
 * N=n_synd field elements (one per syndrome lane). The kernel is therefore just
 * masked loads + XORs: no per-lane reduction chain, fully data-oblivious.
 *
 * Length-agnostic: the syndrome dimension is strip-mined with vsetvli, so this
 * runs unchanged on any VLEN. LMUL m4 is a code choice, not a board assumption.
 *
 * void rs_syndromes_vect(uint16_t *synd, const uint8_t *cdw,
 *                        const uint16_t *tbl, long n_synd, long n_pos)
 *   a0 = synd    output, n_synd elements
 *   a1 = cdw     received bytes
 *   a2 = tbl     precomputed constants, n_pos*8*n_synd elements
 *   a3 = n_synd  number of syndromes (also the table row stride, in elements)
 *   a4 = n_pos   number of codeword positions (PARAM_N1 - 1)
 */

/* Accumulate the contribution of bit \k of the scalar byte t4 into acc (v4).
 * t3 = running row pointer, t6 = row stride in bytes, v8 = scratch. */
.macro SYND_STEP k
    vle16.v v8, (t3)            /* constants alpha^(i*(p+1)) . x^\k for all lanes */
    srli    t5, t4, \k
    andi    t5, t5, 1
    neg     t5, t5             /* 0 - bit -> 0x0000 / 0xFFFF mask                 */
    vand.vx v8, v8, t5
    vxor.vv v4, v4, v8
    add     t3, t3, t6         /* advance to next (p,k) row                       */
.endm

    .globl rs_syndromes_vect
    .type  rs_syndromes_vect, @function
rs_syndromes_vect:
    beqz    a3, .Lret          /* n_synd == 0                                     */
    lbu     a7, (a1)           /* a7 = cdw[0]                                     */
    slli    t6, a3, 1          /* t6 = n_synd * 2 = table row stride in bytes     */
    mv      t1, a3             /* t1 = remaining syndromes                        */
    mv      a5, a0             /* a5 = syndrome write pointer                     */
    mv      a6, a2             /* a6 = table base for current strip               */
.Lstrip:
    VSETVLI_E16_M4(t0, t1)     /* t0 = vl                                         */
    vmv.v.i v4, 0              /* acc = 0                                         */
    li      t2, 0             /* p = 0                                            */
    mv      t3, a6             /* rp = strip base (row p=0, k=0)                  */
.Lpos:
    add     t5, a1, t2
    lbu     t4, 1(t5)          /* t4 = cdw[p+1]                                   */
    SYND_STEP 0
    SYND_STEP 1
    SYND_STEP 2
    SYND_STEP 3
    SYND_STEP 4
    SYND_STEP 5
    SYND_STEP 6
    SYND_STEP 7
    addi    t2, t2, 1
    blt     t2, a4, .Lpos
    vxor.vx v4, v4, a7         /* ^= cdw[0] in every lane                         */
    vse16.v v4, (a5)
    slli    t5, t0, 1          /* vl * 2 bytes                                    */
    add     a5, a5, t5         /* next syndrome slice                             */
    add     a6, a6, t5         /* next i-offset of the table                      */
    sub     t1, t1, t0
    bnez    t1, .Lstrip
.Lret:
    ret
    .size rs_syndromes_vect, .-rs_syndromes_vect
