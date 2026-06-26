#include "gf2x_macros.inc"
    .section .bss
    .align 3

# BUFFER DECLARATIONS
# can probably be more efficient with buffer allocations
U0:
    .zero EVAL_WORDS * 8
U1:
    .zero EVAL_WORDS * 8
U2:
    .zero EVAL_WORDS * 8

V0:
    .zero EVAL_WORDS * 8
V1:
    .zero EVAL_WORDS * 8
V2:
    .zero EVAL_WORDS * 8

EA:
    .zero EVAL_WORDS * 8 
EB:
    .zero EVAL_WORDS * 8

W0:
    .zero PROD_WORDS * 8
W1:
    .zero PROD_WORDS * 8
W2:
    .zero PROD_WORDS * 8
W3:
    .zero PROD_WORDS * 8
W4:
    .zero PROD_WORDS * 8

C0:
    .zero PROD_WORDS * 8
C1:
    .zero PROD_WORDS * 8
C2:
    .zero PROD_WORDS * 8
C3:
    .zero PROD_WORDS * 8
C4:
    .zero PROD_WORDS * 8

T0:
    .zero PROD_WORDS * 8
T1:
    .zero PROD_WORDS * 8
T2:
    .zero PROD_WORDS * 8
T3:
    .zero PROD_WORDS * 8

    .text
    .extern rvv_toom_base_mul
    .globl rvv_toom3_mul
    .type rvv_toom3_mul, @function

rvv_toom3_mul:
    addi sp, sp, -16
    sd ra, 8(sp)
    sd s0, 0(sp)

    mv s0, a0

# CLEANING SQUAD
# repeats later
    mv t3, a0
    ZERO_WORDS t3, TMP_WORDS

    la t3, U0
    ZERO_WORDS t3, EVAL_WORDS
    la t3, U1
    ZERO_WORDS t3, EVAL_WORDS
    la t3, U2
    ZERO_WORDS t3, EVAL_WORDS

    la t3, V0
    ZERO_WORDS t3, EVAL_WORDS
    la t3, V1
    ZERO_WORDS t3, EVAL_WORDS
    la t3, V2
    ZERO_WORDS t3, EVAL_WORDS

    la t3, EA
    ZERO_WORDS t3, EVAL_WORDS
    la t3, EB
    ZERO_WORDS t3, EVAL_WORDS

    la t3, W0
    ZERO_WORDS t3, PROD_WORDS
    la t3, W1
    ZERO_WORDS t3, PROD_WORDS
    la t3, W2
    ZERO_WORDS t3, PROD_WORDS
    la t3, W3
    ZERO_WORDS t3, PROD_WORDS
    la t3, W4
    ZERO_WORDS t3, PROD_WORDS

    la t3, C0
    ZERO_WORDS t3, PROD_WORDS
    la t3, C1
    ZERO_WORDS t3, PROD_WORDS
    la t3, C2
    ZERO_WORDS t3, PROD_WORDS
    la t3, C3
    ZERO_WORDS t3, PROD_WORDS
    la t3, C4
    ZERO_WORDS t3, PROD_WORDS

    la t3, T0
    ZERO_WORDS t3, PROD_WORDS
    la t3, T1
    ZERO_WORDS t3, PROD_WORDS
    la t3, T2
    ZERO_WORDS t3, PROD_WORDS
    la t3, T3
    ZERO_WORDS t3, PROD_WORDS

# TOOM3 SPLIT
    # U0 = A + 0 * TOOM_WORDS
    la t3, U0
    mv t4, a1
    COPY_WORDS t3, t4, TOOM_WORDS

    # U1 = A + 1 * TOOM_WORDS
    la t3, U1
    mv t4, a1
    addi t4, t4, TOOM_WORDS * 8
    COPY_WORDS t3, t4, TOOM_WORDS

    # U2 = A + 2 * TOOM_WORDS
    # Only copy the real input words. Do not read padded words from A.
    la t3, U2
    mv t4, a1
    addi t4, t4, (2 * TOOM_WORDS) * 8
    COPY_WORDS t3, t4, U2_REAL_WORDS

    # Mask unused high bits of final real word in U2.
    la t3, U2
    ld t4, 88 * 8(t3)
    li t5, LAST_WORD_MASK
    and t4, t4, t5
    sd t4, 88 * 8(t3)

    # V0 = B + 0 * TOOM_WORDS
    la t3, V0
    mv t4, a2
    COPY_WORDS t3, t4, TOOM_WORDS

    # V1 = B + 1 * TOOM_WORDS
    la t3, V1
    mv t4, a2
    addi t4, t4, TOOM_WORDS * 8
    COPY_WORDS t3, t4, TOOM_WORDS

    # V2 = B + 2 * TOOM_WORDS
    # Only copy the real input words. Do not read padded words from B.
    la t3, V2
    mv t4, a2
    addi t4, t4, (2 * TOOM_WORDS) * 8
    COPY_WORDS t3, t4, U2_REAL_WORDS

    # Mask unused high bits of final real word in V2.
    la t3, V2
    ld t4, 88 * 8(t3)
    li t5, LAST_WORD_MASK
    and t4, t4, t5
    sd t4, 88 * 8(t3)

# TOOM3 EVALUATION
    # already have U0, V0
    # W0 = U0 * V0
    la a0, W0
    la a1, U0
    la a2, V0
    li a3, EVAL_WORDS
    call rvv_toom_base_mul

    # eval at 1
    la t3, EA
    ZERO_WORDS t3, EVAL_WORDS

    la t3, EB
    ZERO_WORDS t3, EVAL_WORDS

    # EA = A(1) = U0 ^ U1 ^ U2
    la t3, EA
    la t4, U0
    la t5, U1
    la t6, U2
    XOR3_WORDS t3, t4, t5, t6, EVAL_WORDS

    # EB = B(1) = V0 ^ V1 ^ V2
    la t3, EB
    la t4, V0
    la t5, V1
    la t6, V2
    XOR3_WORDS t3, t4, t5, t6, EVAL_WORDS

    # W1 = EA * EB
    la a0, W1
    la a1, EA
    la a2, EB
    li a3, EVAL_WORDS
    call rvv_toom_base_mul

    # eval at x = X^w, w = 64
    # possible optimizations in changing w
    la t3, EA
    ZERO_WORDS t3, EVAL_WORDS

    la t3, EB
    ZERO_WORDS t3, EVAL_WORDS

    # EA = A(x) = U0 ^ (U1 << 64) ^ (U2 << 128)
    # EA[0..93] ^= U0[0..93]
    la t3, EA
    la t4, U0
    XOR_INTO_WORDS t3, t4, TOOM_WORDS

    # EA[1..94] ^= U1[0..93]
    la t3, EA
    addi t3, t3, 8
    la t4, U1
    XOR_INTO_WORDS t3, t4, TOOM_WORDS

    # EA[2..95] ^= U2[0..93]
    # can possibly reduce this
    la t3, EA
    addi t3, t3, 16
    la t4, U2
    XOR_INTO_WORDS t3, t4, TOOM_WORDS

    # EB = B(x) = V0 ^ (V1 << 64) ^ (V2 << 128)
    # EB[0..93] ^= V0[0..93]
    la t3, EB
    la t4, V0
    XOR_INTO_WORDS t3, t4, TOOM_WORDS

    # EB[1..94] ^= V1[0..93]
    la t3, EB
    addi t3, t3, 8
    la t4, V1
    XOR_INTO_WORDS t3, t4, TOOM_WORDS

    # EB[2..95] ^= V2[0..93]
    # can possibly reduce this
    la t3, EB
    addi t3, t3, 16
    la t4, V2
    XOR_INTO_WORDS t3, t4, TOOM_WORDS

    # W2 = EA * EB
    la a0, W2
    la a1, EA
    la a2, EB
    li a3, EVAL_WORDS
    call rvv_toom_base_mul

    # eval at x + 1 = X^64 + 1
    # w may change in optimizations
    la t3, EA
    ZERO_WORDS t3, EVAL_WORDS

    la t3, EB
    ZERO_WORDS t3, EVAL_WORDS

    # EA = A(x+1) = U0 ^ U1 ^ U2 ^ (U1 << 64) ^ (U2 << 128)
    # EA = U0 ^ U1 ^ U2
    la t3, EA
    la t4, U0
    la t5, U1
    la t6, U2
    XOR3_WORDS t3, t4, t5, t6, EVAL_WORDS

    # EA ^= U1 << 64
    # EA[1..94] ^= U1[0..93]
    la t3, EA
    addi t3, t3, 8
    la t4, U1
    XOR_INTO_WORDS t3, t4, TOOM_WORDS

    # EA ^= U2 << 128
    # EA[2..95] ^= U2[0..93]
    la t3, EA
    addi t3, t3, 16
    la t4, U2
    XOR_INTO_WORDS t3, t4, TOOM_WORDS

    # EB = B(x+1) = V0 ^ V1 ^ V2 ^ (V1 << 64) ^ (V2 << 128)
    # EB = V0 ^ V1 ^ V2
    la t3, EB
    la t4, V0
    la t5, V1
    la t6, V2
    XOR3_WORDS t3, t4, t5, t6, EVAL_WORDS

    # EB ^= V1 << 64
    # EB[1..94] ^= V1[0..93]
    la t3, EB
    addi t3, t3, 8
    la t4, V1
    XOR_INTO_WORDS t3, t4, TOOM_WORDS

    # EB ^= V2 << 128
    # EB[2..95] ^= V2[0..93]
    la t3, EB
    addi t3, t3, 16
    la t4, V2
    XOR_INTO_WORDS t3, t4, TOOM_WORDS

    # W3 = EA * EB
    la a0, W3
    la a1, EA
    la a2, EB
    li a3, EVAL_WORDS
    call rvv_toom_base_mul

    # already have U2, V2
    # W4 = U2 * V2
    la a0, W4
    la a1, U2
    la a2, V2
    li a3, EVAL_WORDS
    call rvv_toom_base_mul

# TOOM3 INTERPOLATION
    # c0 = C(0) = W0
    la t3, C0
    la t4, W0
    COPY_WORDS t3, t4, PROD_WORDS

    # c4 = C(infinity) = W4
    la t3, C4
    la t4, W4
    COPY_WORDS t3, t4, PROD_WORDS

    # T0 = W2 ^ C0 ^ (C4 << 4 words)
    # This removes c0 and c4*x^4 from C(x)
    la t3, T0
    ZERO_WORDS t3, PROD_WORDS

    # T0 = W2
    la t3, T0
    la t4, W2
    COPY_WORDS t3, t4, PROD_WORDS

    # T0 ^= C0
    la t3, T0
    la t4, C0
    XOR_INTO_WORDS t3, t4, PROD_WORDS

    # T0 ^= C4 << 4 words
    # 4 words = x^4, because x = X^64
    la t3, T0
    addi t3, t3, 32
    la t4, C4
    XOR_INTO_WORDS t3, t4, PROD_WORDS-4

    # T1 = T0 / x
    la t3, T1
    ZERO_WORDS t3, PROD_WORDS

    la t3, T1
    la t4, T0
    DIV_X_WORDS t3, t4

    # T2 = W3 ^ C0 ^ C4 ^ (C4 << 4 words)
    # This removes c0 and c4*(x+1)^4 from C(x+1)
    # Bc (x+1)^4 = x^4 + 1
    la t3, T2
    ZERO_WORDS t3, PROD_WORDS

    # T2 = W3
    la t3, T2
    la t4, W3
    COPY_WORDS t3, t4, PROD_WORDS

    # T2 ^= C0
    la t3, T2
    la t4, C0
    XOR_INTO_WORDS t3, t4, PROD_WORDS

    # T2 ^= C4
    la t3, T2
    la t4, C4
    XOR_INTO_WORDS t3, t4, PROD_WORDS

    # T2 ^= C4*x^4
    la t3, T2
    addi t3, t3, 32
    la t4, C4
    XOR_INTO_WORDS t3, t4, PROD_WORDS-4

    # T3 = T2 / (x + 1)
    la t3, T3
    ZERO_WORDS t3, PROD_WORDS

    la t3, T3
    la t4, T2
    DIV_X_PLUS_1_WORDS t3, t4, PROD_WORDS

    # T0 = T1 ^ T3
    # After this:
    # T0 = c2 ^ c3
    la t3, T0
    ZERO_WORDS t3, PROD_WORDS

    la t3, T0
    la t4, T1
    COPY_WORDS t3, t4, PROD_WORDS

    la t3, T0
    la t4, T3
    XOR_INTO_WORDS t3, t4, PROD_WORDS


    # C1 = W1 ^ C0 ^ C4 ^ T0
    # Since:
    # W1 = c0 ^ c1 ^ c2 ^ c3 ^ c4
    # T0 = c2 ^ c3
    # Therefore:
    # C1 = c1
    la t3, C1
    ZERO_WORDS t3, PROD_WORDS

    la t3, C1
    la t4, W1
    COPY_WORDS t3, t4, PROD_WORDS

    la t3, C1
    la t4, C0
    XOR_INTO_WORDS t3, t4, PROD_WORDS

    la t3, C1
    la t4, C4
    XOR_INTO_WORDS t3, t4, PROD_WORDS

    la t3, C1
    la t4, T0
    XOR_INTO_WORDS t3, t4, PROD_WORDS


    # T2 = T1 ^ C1
    # T2 = c2*x ^ c3*x^2
    la t3, T2
    ZERO_WORDS t3, PROD_WORDS

    la t3, T2
    la t4, T1
    COPY_WORDS t3, t4, PROD_WORDS

    la t3, T2
    la t4, C1
    XOR_INTO_WORDS t3, t4, PROD_WORDS


    # T2 = T2 / x
    # T2 = c2 ^ c3*x
    la t3, T2
    la t4, T2
    DIV_X_WORDS t3, t4


    # T2 ^= T0
    # T2 = (c2 ^ c3*x) ^ (c2 ^ c3)
    # T2 = c3*x ^ c3
    # T2 = c3*(x+1)
    la t3, T2
    la t4, T0
    XOR_INTO_WORDS t3, t4, PROD_WORDS


    # C3 = T2 / (x + 1)
    # C3 = c3
    la t3, C3
    ZERO_WORDS t3, PROD_WORDS

    la t3, C3
    la t4, T2
    DIV_X_PLUS_1_WORDS t3, t4, PROD_WORDS


    # C2 = T0 ^ C3
    # Since T0 = c2 ^ c3:
    # C2 = c2
    la t3, C2
    ZERO_WORDS t3, PROD_WORDS

    la t3, C2
    la t4, T0
    COPY_WORDS t3, t4, PROD_WORDS

    la t3, C2
    la t4, C3
    XOR_INTO_WORDS t3, t4, PROD_WORDS

# TOOM3 RECOMPOSITION
    # tmp = C0 ^ (C1 << 94 words) ^ (C2 << 188 words) ^ (C3 << 282 words) ^ (C4 << 376 words)
    # tmp ^= C0
    mv t3, s0
    la t4, C0
    XOR_INTO_WORDS t3, t4, PROD_WORDS

    # tmp[94..] ^= C1
    mv t3, s0
    li t5, TOOM_WORDS * 8
    add t3, t3, t5
    la t4, C1
    XOR_INTO_WORDS t3, t4, PROD_WORDS

    # tmp[188..] ^= C2
    mv t3, s0
    li t5, (2 * TOOM_WORDS) * 8
    add t3, t3, t5
    la t4, C2
    XOR_INTO_WORDS t3, t4, PROD_WORDS

    # tmp[282..] ^= C3
    mv t3, s0
    li t5, (3 * TOOM_WORDS) * 8
    add t3, t3, t5
    la t4, C3
    XOR_INTO_WORDS t3, t4, PROD_WORDS

    # tmp[376..] ^= C4
    # sketch af but we ball
    mv t3, s0
    li t5, (4 * TOOM_WORDS) * 8
    add t3, t3, t5
    la t4, C4
    XOR_INTO_WORDS t3, t4, TMP_WORDS - (4 * TOOM_WORDS)


# WIPE BUFFERS
# CLEANING SQUAD: RETURN OF THE KING
    la t3, U0
    ZERO_WORDS t3, EVAL_WORDS
    la t3, U1
    ZERO_WORDS t3, EVAL_WORDS
    la t3, U2
    ZERO_WORDS t3, EVAL_WORDS

    la t3, V0
    ZERO_WORDS t3, EVAL_WORDS
    la t3, V1
    ZERO_WORDS t3, EVAL_WORDS
    la t3, V2
    ZERO_WORDS t3, EVAL_WORDS

    la t3, EA
    ZERO_WORDS t3, EVAL_WORDS
    la t3, EB
    ZERO_WORDS t3, EVAL_WORDS

    la t3, W0
    ZERO_WORDS t3, PROD_WORDS
    la t3, W1
    ZERO_WORDS t3, PROD_WORDS
    la t3, W2
    ZERO_WORDS t3, PROD_WORDS
    la t3, W3
    ZERO_WORDS t3, PROD_WORDS
    la t3, W4
    ZERO_WORDS t3, PROD_WORDS

    la t3, C0
    ZERO_WORDS t3, PROD_WORDS
    la t3, C1
    ZERO_WORDS t3, PROD_WORDS
    la t3, C2
    ZERO_WORDS t3, PROD_WORDS
    la t3, C3
    ZERO_WORDS t3, PROD_WORDS
    la t3, C4
    ZERO_WORDS t3, PROD_WORDS

    la t3, T0
    ZERO_WORDS t3, PROD_WORDS
    la t3, T1
    ZERO_WORDS t3, PROD_WORDS
    la t3, T2
    ZERO_WORDS t3, PROD_WORDS
    la t3, T3
    ZERO_WORDS t3, PROD_WORDS
    

    ld s0, 0(sp)
    ld ra, 8(sp)
    addi sp, sp, 16
    ret

    .size rvv_toom3_mul, .-rvv_toom3_mul