#ifndef GF2X_MACROS_INC
#define GF2X_MACROS_INC

#define VSETVLI_E8_M1(rd, rs1)   .insn i 0x57, 0x7, rd, rs1, 0
#define VSETVLI_E16_M1(rd, rs1)  .insn i 0x57, 0x7, rd, rs1, 4
#define VSETVLI_E32_M1(rd, rs1)  .insn i 0x57, 0x7, rd, rs1, 8
#define VSETVLI_E64_M1(rd, rs1)  .insn i 0x57, 0x7, rd, rs1, 12
#define VSETVLI_E64_M8(rd, rs1)  .insn i 0x57, 0x7, rd, rs1, 15

#define TOOM_WORDS          94
#define EVAL_WORDS          96
#define U2_REAL_WORDS       89
#define TMP_WORDS           565
#define LAST_WORD_MASK      0x1f
#define PROD_WORDS          193

#define K96_WORDS             EVAL_WORDS
#define K96_HALF_WORDS        (K96_WORDS / 2)
#define K96_SUBPROD_WORDS     (2 * K96_HALF_WORDS + 1)

#define K48_WORDS             K96_HALF_WORDS
#define K48_HALF_WORDS        (K48_WORDS / 2)
#define K48_SUBPROD_WORDS     (2 * K48_HALF_WORDS + 1)

#define K24_WORDS             K48_HALF_WORDS
#define K24_HALF_WORDS        (K24_WORDS / 2)
#define K24_SUBPROD_WORDS     (2 * K24_HALF_WORDS + 1)

#define K12_WORDS             K24_HALF_WORDS
#define K12_HALF_WORDS        (K12_WORDS / 2)
#define K12_SUBPROD_WORDS     (2 * K12_HALF_WORDS + 1)

#define K6_WORDS              K12_HALF_WORDS
#define K6_HALF_WORDS         (K6_WORDS / 2)
#define K6_SUBPROD_WORDS      (2 * K6_HALF_WORDS + 1)

.macro ZERO_WORDS ptr_reg, nwords
    li t0, \nwords
.L_zero_loop\@:
    beqz t0, .L_zero_done\@
    VSETVLI_E64_M8(t1, t0)
    vmv.v.i v0, 0
    vse64.v v0, (\ptr_reg)
    slli t2, t1, 3
    add \ptr_reg, \ptr_reg, t2
    sub t0, t0, t1
    j .L_zero_loop\@
.L_zero_done\@:
.endm

.macro COPY_WORDS dst_reg, src_reg, nwords
    li t0, \nwords
.L_copy_loop\@:
    beqz t0, .L_copy_done\@
    VSETVLI_E64_M8(t1, t0)
    vle64.v v0, (\src_reg)
    vse64.v v0, (\dst_reg)
    slli t2, t1, 3
    add \src_reg, \src_reg, t2
    add \dst_reg, \dst_reg, t2
    sub t0, t0, t1
    j .L_copy_loop\@
.L_copy_done\@:
.endm

.macro XOR2_WORDS dst_reg, src1_reg, src2_reg, nwords
    li t0, \nwords
.L_xor2_loop\@:
    beqz t0, .L_xor2_done\@
    VSETVLI_E64_M8(t1, t0)
    vle64.v v0, (\src1_reg)
    vle64.v v8, (\src2_reg)
    vxor.vv v16, v0, v8
    vse64.v v16, (\dst_reg)
    slli t2, t1, 3
    add \src1_reg, \src1_reg, t2
    add \src2_reg, \src2_reg, t2
    add \dst_reg, \dst_reg, t2
    sub t0, t0, t1
    j .L_xor2_loop\@
.L_xor2_done\@:
.endm

.macro XOR3_WORDS dst_reg, src1_reg, src2_reg, src3_reg, nwords
    li t0, \nwords
.L_xor3_loop\@:
    beqz t0, .L_xor3_done\@
    VSETVLI_E64_M8(t1, t0)
    vle64.v v0, (\src1_reg)
    vle64.v v8, (\src2_reg)
    vle64.v v16, (\src3_reg)
    vxor.vv v0, v0, v8
    vxor.vv v0, v0, v16
    vse64.v v0, (\dst_reg)
    slli t2, t1, 3
    add \src1_reg, \src1_reg, t2
    add \src2_reg, \src2_reg, t2
    add \src3_reg, \src3_reg, t2
    add \dst_reg, \dst_reg, t2
    sub t0, t0, t1
    j .L_xor3_loop\@
.L_xor3_done\@:
.endm

.macro XOR_INTO_WORDS dst_reg, src_reg, nwords
    li t0, \nwords
.L_xor_into_loop\@:
    beqz t0, .L_xor_into_done\@
    VSETVLI_E64_M8(t1, t0)
    vle64.v v0, (\dst_reg)
    vle64.v v8, (\src_reg)
    vxor.vv v0, v0, v8
    vse64.v v0, (\dst_reg)
    slli t2, t1, 3
    add \dst_reg, \dst_reg, t2
    add \src_reg, \src_reg, t2
    sub t0, t0, t1
    j .L_xor_into_loop\@
.L_xor_into_done\@:
.endm

.macro DIV_X_WORDS dst_reg, src_reg
    addi \src_reg, \src_reg, 8
    COPY_WORDS \dst_reg, \src_reg, PROD_WORDS - 1
    sd zero, 0(\dst_reg)
.endm

.macro DIV_X_PLUS_1_WORDS dst_reg, src_reg, nwords
    li t0, \nwords
    li t1, 0
.L_div_xp1_loop\@:
    beqz t0, .L_div_xp1_done\@
    ld t2, 0(\src_reg)
    xor t1, t1, t2
    sd t1, 0(\dst_reg)
    addi \src_reg, \src_reg, 8
    addi \dst_reg, \dst_reg, 8
    addi t0, t0, -1
    j .L_div_xp1_loop\@
.L_div_xp1_done\@:
.endm

#endif