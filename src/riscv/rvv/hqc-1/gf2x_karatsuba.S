#include "gf2x_macros.inc"

    .section .bss
    .align 3

# Temporary buffers
#K96
K96_TA:
    .zero K96_HALF_WORDS * 8
K96_TB:
    .zero K96_HALF_WORDS * 8

K96_Z0:
    .zero K96_SUBPROD_WORDS * 8
K96_Z2:
    .zero K96_SUBPROD_WORDS * 8

K96_ZM:
    .zero K96_SUBPROD_WORDS * 8

#K48
K48_TA:
    .zero K48_HALF_WORDS * 8
K48_TB:
    .zero K48_HALF_WORDS * 8

K48_Z0:
    .zero K48_SUBPROD_WORDS * 8
K48_Z2:
    .zero K48_SUBPROD_WORDS * 8
K48_ZM:
    .zero K48_SUBPROD_WORDS * 8

#K24
K24_TA:
    .zero K24_HALF_WORDS * 8
K24_TB:
    .zero K24_HALF_WORDS * 8

K24_Z0:
    .zero K24_SUBPROD_WORDS * 8
K24_Z2:
    .zero K24_SUBPROD_WORDS * 8
K24_ZM:
    .zero K24_SUBPROD_WORDS * 8

#K12
K12_TA:
    .zero K12_HALF_WORDS * 8
K12_TB:
    .zero K12_HALF_WORDS * 8

K12_Z0:
    .zero K12_SUBPROD_WORDS * 8
K12_Z2:
    .zero K12_SUBPROD_WORDS * 8
K12_ZM:
    .zero K12_SUBPROD_WORDS * 8

#K6
K6_TA:
    .zero K6_HALF_WORDS * 8
K6_TB:
    .zero K6_HALF_WORDS * 8

K6_Z0:
    .zero K6_SUBPROD_WORDS * 8
K6_Z2:
    .zero K6_SUBPROD_WORDS * 8
K6_ZM:
    .zero K6_SUBPROD_WORDS * 8


    .text
    .extern gf2x_mul3_leaf

# KARATSUBA_96
    .globl rvv_kara96_mul
    .type rvv_kara96_mul, @function

rvv_kara96_mul:
    addi sp, sp, -32
    sd ra, 24(sp)
    sd s0, 16(sp)
    sd s1, 8(sp)
    sd s2, 0(sp)

    mv s0, a0
    mv s1, a1
    mv s2, a2

# CLEANING DUTYYY
    mv t3, s0
    ZERO_WORDS t3, PROD_WORDS

    # la t3, K96_TA
    # ZERO_WORDS t3, K96_HALF_WORDS

    # la t3, K96_TB
    # ZERO_WORDS t3, K96_HALF_WORDS

    # la t3, K96_Z0
    # ZERO_WORDS t3, K96_SUBPROD_WORDS

    # la t3, K96_Z2
    # ZERO_WORDS t3, K96_SUBPROD_WORDS

    # la t3, K96_ZM
    # ZERO_WORDS t3, K96_SUBPROD_WORDS

# DECOMPOSITION
    # TA = A0 ^ A1
    la t3, K96_TA
    mv t4, s1
    mv t5, s1
    addi t5, t5, K96_HALF_WORDS * 8
    XOR2_WORDS t3, t4, t5, K96_HALF_WORDS

    # TB = B0 ^ B1
    la t3, K96_TB
    mv t4, s2
    mv t5, s2
    addi t5, t5, K96_HALF_WORDS * 8
    XOR2_WORDS t3, t4, t5, K96_HALF_WORDS

    # Z0 = A0 * B0
    la a0, K96_Z0
    mv a1, s1
    mv a2, s2
    # li a3, K96_HALF_WORDS
    call rvv_kara48_mul

    # Z2 = A1 * B1
    la a0, K96_Z2

    mv a1, s1
    addi a1, a1, K96_HALF_WORDS * 8

    mv a2, s2
    addi a2, a2, K96_HALF_WORDS * 8

    # li a3, K96_HALF_WORDS
    call rvv_kara48_mul


    # ZM = TA * TB
    la a0, K96_ZM
    la a1, K96_TA
    la a2, K96_TB
    # li a3, K96_HALF_WORDS
    call rvv_kara48_mul

    # middle = ZM ^ Z0 ^ Z2
    la t3, K96_ZM
    la t4, K96_Z0
    XOR_INTO_WORDS t3, t4, K96_SUBPROD_WORDS

    la t3, K96_ZM
    la t4, K96_Z2
    XOR_INTO_WORDS t3, t4, K96_SUBPROD_WORDS

# RECOMPOSITION
    # out ^= Z0
    mv t3, s0
    la t4, K96_Z0
    XOR_INTO_WORDS t3, t4, K96_SUBPROD_WORDS

    # out[48..] ^= middle
    mv t3, s0
    addi t3, t3, K96_HALF_WORDS * 8
    la t4, K96_ZM
    XOR_INTO_WORDS t3, t4, K96_SUBPROD_WORDS

    # out[96..] ^= Z2
    mv t3, s0
    addi t3, t3, K96_WORDS * 8
    la t4, K96_Z2
    XOR_INTO_WORDS t3, t4, K96_SUBPROD_WORDS

# CLEANING DUTY: THE EMPIRE STRIKES BACK
    # la t3, K96_TA
    # ZERO_WORDS t3, K96_HALF_WORDS

    # la t3, K96_TB
    # ZERO_WORDS t3, K96_HALF_WORDS

    # la t3, K96_Z0
    # ZERO_WORDS t3, K96_SUBPROD_WORDS

    # la t3, K96_Z2
    # ZERO_WORDS t3, K96_SUBPROD_WORDS

    # la t3, K96_ZM
    # ZERO_WORDS t3, K96_SUBPROD_WORDS


    ld s2, 0(sp)
    ld s1, 8(sp)
    ld s0, 16(sp)
    ld ra, 24(sp)
    addi sp, sp, 32
    ret

    .size rvv_kara96_mul, .-rvv_kara96_mul

#KARATSUBA_48
    .globl rvv_kara48_mul
    .type rvv_kara48_mul, @function

rvv_kara48_mul:
    addi sp, sp, -32
    sd ra, 24(sp)
    sd s0, 16(sp)
    sd s1, 8(sp)
    sd s2, 0(sp)

    mv s0, a0
    mv s1, a1
    mv s2, a2

# HAHAH JONATHAN CLEANING DUTY
    mv t3, s0
    ZERO_WORDS t3, K96_SUBPROD_WORDS

    # la t3, K48_TA
    # ZERO_WORDS t3, K48_HALF_WORDS

    # la t3, K48_TB
    # ZERO_WORDS t3, K48_HALF_WORDS

    # la t3, K48_Z0
    # ZERO_WORDS t3, K48_SUBPROD_WORDS

    # la t3, K48_Z2
    # ZERO_WORDS t3, K48_SUBPROD_WORDS

    # la t3, K48_ZM
    # ZERO_WORDS t3, K48_SUBPROD_WORDS

# DECOMPOSITION
    # TA = A0 ^ A1
    la t3, K48_TA
    mv t4, s1
    mv t5, s1
    addi t5, t5, K48_HALF_WORDS * 8
    XOR2_WORDS t3, t4, t5, K48_HALF_WORDS

    # TB = B0 ^ B1
    la t3, K48_TB
    mv t4, s2
    mv t5, s2
    addi t5, t5, K48_HALF_WORDS * 8
    XOR2_WORDS t3, t4, t5, K48_HALF_WORDS

    # Z0 = A0 * B0
    la a0, K48_Z0
    mv a1, s1
    mv a2, s2
    # li a3, K48_HALF_WORDS
    call rvv_kara24_mul

    # Z2 = A1 * B1
    la a0, K48_Z2

    mv a1, s1
    addi a1, a1, K48_HALF_WORDS * 8

    mv a2, s2
    addi a2, a2, K48_HALF_WORDS * 8

    # li a3, K48_HALF_WORDS
    call rvv_kara24_mul


    # ZM = TA * TB
    la a0, K48_ZM
    la a1, K48_TA
    la a2, K48_TB
    # li a3, K48_HALF_WORDS
    call rvv_kara24_mul

    # middle = ZM ^ Z0 ^ Z2
    la t3, K48_ZM
    la t4, K48_Z0
    XOR_INTO_WORDS t3, t4, K48_SUBPROD_WORDS

    la t3, K48_ZM
    la t4, K48_Z2
    XOR_INTO_WORDS t3, t4, K48_SUBPROD_WORDS

# RECOMPOSITION
    # out ^= Z0
    mv t3, s0
    la t4, K48_Z0
    XOR_INTO_WORDS t3, t4, K48_SUBPROD_WORDS

    # out[24..] ^= middle
    mv t3, s0
    addi t3, t3, K48_HALF_WORDS * 8
    la t4, K48_ZM
    XOR_INTO_WORDS t3, t4, K48_SUBPROD_WORDS

    # out[48..] ^= Z2
    mv t3, s0
    addi t3, t3, K48_WORDS * 8
    la t4, K48_Z2
    XOR_INTO_WORDS t3, t4, K48_SUBPROD_WORDS

# THE LORD SAID: LET THE BUFFERS BE CLEAN
    # la t3, K48_TA
    # ZERO_WORDS t3, K48_HALF_WORDS

    # la t3, K48_TB
    # ZERO_WORDS t3, K48_HALF_WORDS

    # la t3, K48_Z0
    # ZERO_WORDS t3, K48_SUBPROD_WORDS

    # la t3, K48_Z2
    # ZERO_WORDS t3, K48_SUBPROD_WORDS

    # la t3, K48_ZM
    # ZERO_WORDS t3, K48_SUBPROD_WORDS

    ld s2, 0(sp)
    ld s1, 8(sp)
    ld s0, 16(sp)
    ld ra, 24(sp)
    addi sp, sp, 32
    ret

    .size rvv_kara48_mul, .-rvv_kara48_mul

# KARATSUBA_24
    .globl rvv_kara24_mul
    .type rvv_kara24_mul, @function

rvv_kara24_mul:
    addi sp, sp, -32
    sd ra, 24(sp)
    sd s0, 16(sp)
    sd s1, 8(sp)
    sd s2, 0(sp)

    mv s0, a0
    mv s1, a1
    mv s2, a2

# CLEAN
    mv t3, s0
    ZERO_WORDS t3, K48_SUBPROD_WORDS

    # la t3, K24_TA
    # ZERO_WORDS t3, K24_HALF_WORDS

    # la t3, K24_TB
    # ZERO_WORDS t3, K24_HALF_WORDS

    # la t3, K24_Z0
    # ZERO_WORDS t3, K24_SUBPROD_WORDS

    # la t3, K24_Z2
    # ZERO_WORDS t3, K24_SUBPROD_WORDS

    # la t3, K24_ZM
    # ZERO_WORDS t3, K24_SUBPROD_WORDS

# DECOMPOSITION
    # TA = A0 ^ A1
    la t3, K24_TA
    mv t4, s1
    mv t5, s1
    addi t5, t5, K24_HALF_WORDS * 8
    XOR2_WORDS t3, t4, t5, K24_HALF_WORDS

    # TB = B0 ^ B1
    la t3, K24_TB
    mv t4, s2
    mv t5, s2
    addi t5, t5, K24_HALF_WORDS * 8
    XOR2_WORDS t3, t4, t5, K24_HALF_WORDS

    # Z0 = A0 * B0
    la a0, K24_Z0
    mv a1, s1
    mv a2, s2
    # li a3, K24_HALF_WORDS
    call rvv_kara12_mul


    # Z2 = A1 * B1
    la a0, K24_Z2

    mv a1, s1
    addi a1, a1, K24_HALF_WORDS * 8

    mv a2, s2
    addi a2, a2, K24_HALF_WORDS * 8

    # li a3, K24_HALF_WORDS
    call rvv_kara12_mul

    # ZM = TA * TB 
    la a0, K24_ZM
    la a1, K24_TA
    la a2, K24_TB
    # li a3, K24_HALF_WORDS
    call rvv_kara12_mul

    # middle = ZM ^ Z0 ^ Z2
    la t3, K24_ZM
    la t4, K24_Z0
    XOR_INTO_WORDS t3, t4, K24_SUBPROD_WORDS

    la t3, K24_ZM
    la t4, K24_Z2
    XOR_INTO_WORDS t3, t4, K24_SUBPROD_WORDS

# RECOMPOSITION
    # out ^= Z0
    mv t3, s0
    la t4, K24_Z0
    XOR_INTO_WORDS t3, t4, K24_SUBPROD_WORDS

    # out[12..] ^= middle
    mv t3, s0
    addi t3, t3, K24_HALF_WORDS * 8
    la t4, K24_ZM
    XOR_INTO_WORDS t3, t4, K24_SUBPROD_WORDS

    # out[24..] ^= Z2
    mv t3, s0
    addi t3, t3, K24_WORDS * 8
    la t4, K24_Z2
    XOR_INTO_WORDS t3, t4, K24_SUBPROD_WORDS


# CLEAN TEMPORARIES
    # la t3, K24_TA
    # ZERO_WORDS t3, K24_HALF_WORDS

    # la t3, K24_TB
    # ZERO_WORDS t3, K24_HALF_WORDS

    # la t3, K24_Z0
    # ZERO_WORDS t3, K24_SUBPROD_WORDS

    # la t3, K24_Z2
    # ZERO_WORDS t3, K24_SUBPROD_WORDS

    # la t3, K24_ZM
    # ZERO_WORDS t3, K24_SUBPROD_WORDS


    ld s2, 0(sp)
    ld s1, 8(sp)
    ld s0, 16(sp)
    ld ra, 24(sp)
    addi sp, sp, 32
    ret

    .size rvv_kara24_mul, .-rvv_kara24_mul

# KARATSUBA_12
    .globl rvv_kara12_mul
    .type rvv_kara12_mul, @function

rvv_kara12_mul:
    addi sp, sp, -32
    sd ra, 24(sp)
    sd s0, 16(sp)
    sd s1, 8(sp)
    sd s2, 0(sp)

    mv s0, a0
    mv s1, a1
    mv s2, a2

# CLEAN
    # Output of 12-word multiplication is 25 words.
    mv t3, s0
    ZERO_WORDS t3, K24_SUBPROD_WORDS

    # la t3, K12_TA
    # ZERO_WORDS t3, K12_HALF_WORDS

    # la t3, K12_TB
    # ZERO_WORDS t3, K12_HALF_WORDS

    # la t3, K12_Z0
    # ZERO_WORDS t3, K12_SUBPROD_WORDS

    # la t3, K12_Z2
    # ZERO_WORDS t3, K12_SUBPROD_WORDS

    # la t3, K12_ZM
    # ZERO_WORDS t3, K12_SUBPROD_WORDS

# DECOMPOSITION
    # TA = A0 ^ A1
    # A0 = a[0..5]
    # A1 = a[6..11]
    la t3, K12_TA
    mv t4, s1
    mv t5, s1
    addi t5, t5, K12_HALF_WORDS * 8
    XOR2_WORDS t3, t4, t5, K12_HALF_WORDS

    # TB = B0 ^ B1
    # B0 = b[0..5]
    # B1 = b[6..11]
    la t3, K12_TB
    mv t4, s2
    mv t5, s2
    addi t5, t5, K12_HALF_WORDS * 8
    XOR2_WORDS t3, t4, t5, K12_HALF_WORDS

    # Z0 = A0 * B0
    la a0, K12_Z0
    mv a1, s1
    mv a2, s2
    # li a3, K12_HALF_WORDS
    call scalar_kara6_mul

    # Z2 = A1 * B1
    la a0, K12_Z2

    mv a1, s1
    addi a1, a1, K12_HALF_WORDS * 8

    mv a2, s2
    addi a2, a2, K12_HALF_WORDS * 8

    # li a3, K12_HALF_WORDS
    call scalar_kara6_mul

    # ZM = TA * TB
    la a0, K12_ZM
    la a1, K12_TA
    la a2, K12_TB
    # li a3, K12_HALF_WORDS
    call scalar_kara6_mul

    # middle = ZM ^ Z0 ^ Z2
    la t3, K12_ZM
    la t4, K12_Z0
    XOR_INTO_WORDS t3, t4, K12_SUBPROD_WORDS

    la t3, K12_ZM
    la t4, K12_Z2
    XOR_INTO_WORDS t3, t4, K12_SUBPROD_WORDS

# RECOMPOSITION
    # out ^= Z0
    mv t3, s0
    la t4, K12_Z0
    XOR_INTO_WORDS t3, t4, K12_SUBPROD_WORDS

    # out[6..] ^= middle
    mv t3, s0
    addi t3, t3, K12_HALF_WORDS * 8
    la t4, K12_ZM
    XOR_INTO_WORDS t3, t4, K12_SUBPROD_WORDS

    # out[12..] ^= Z2
    mv t3, s0# 
    addi t3, t3, K12_WORDS * 8
    la t4, K12_Z2
    XOR_INTO_WORDS t3, t4, K12_SUBPROD_WORDS

# CLEAN TEMPORARIES
    # la t3, K12_TA
    # ZERO_WORDS t3, K12_HALF_WORDS

    # la t3, K12_TB
    # ZERO_WORDS t3, K12_HALF_WORDS

    # la t3, K12_Z0
    # ZERO_WORDS t3, K12_SUBPROD_WORDS

    # la t3, K12_Z2
    # ZERO_WORDS t3, K12_SUBPROD_WORDS

    # la t3, K12_ZM
    # ZERO_WORDS t3, K12_SUBPROD_WORDS

    ld s2, 0(sp)
    ld s1, 8(sp)
    ld s0, 16(sp)
    ld ra, 24(sp)
    addi sp, sp, 32
    ret

    .size rvv_kara12_mul, .-rvv_kara12_mul

# KARATSUBA_6
    .globl rvv_kara6_mul
    .type rvv_kara6_mul, @function

rvv_kara6_mul:
    addi sp, sp, -32
    sd ra, 24(sp)
    sd s0, 16(sp)
    sd s1, 8(sp)
    sd s2, 0(sp)

    mv s0, a0
    mv s1, a1
    mv s2, a2

# CLEAN
    # Output of 6-word multiplication is 13 words.
    mv t3, s0
    ZERO_WORDS t3, K12_SUBPROD_WORDS

    # la t3, K6_TA
    # ZERO_WORDS t3, K6_HALF_WORDS

    # la t3, K6_TB
    # ZERO_WORDS t3, K6_HALF_WORDS

    # la t3, K6_Z0
    # ZERO_WORDS t3, K6_SUBPROD_WORDS

    # la t3, K6_Z2
    # ZERO_WORDS t3, K6_SUBPROD_WORDS

    # la t3, K6_ZM
    # ZERO_WORDS t3, K6_SUBPROD_WORDS

# DECOMPOSITION
    # TA = A0 ^ A1
    # A0 = a[0..2]
    # A1 = a[3..5]
    la t3, K6_TA
    mv t4, s1
    mv t5, s1
    addi t5, t5, K6_HALF_WORDS * 8
    XOR2_WORDS t3, t4, t5, K6_HALF_WORDS

    # TB = B0 ^ B1
    # B0 = b[0..2]
    # B1 = b[3..5]
    la t3, K6_TB
    mv t4, s2
    mv t5, s2
    addi t5, t5, K6_HALF_WORDS * 8
    XOR2_WORDS t3, t4, t5, K6_HALF_WORDS

    # Z0 = A0 * B0
    la a0, K6_Z0
    mv a1, s1
    mv a2, s2
    # li a3, K6_HALF_WORDS
    call gf2x_mul3_leaf

    # Z2 = A1 * B1
    la a0, K6_Z2
    mv a1, s1
    addi a1, a1, K6_HALF_WORDS * 8
    mv a2, s2
    addi a2, a2, K6_HALF_WORDS * 8
    # li a3, K6_HALF_WORDS
    call gf2x_mul3_leaf

    # ZM = TA * TB
    la a0, K6_ZM
    la a1, K6_TA
    la a2, K6_TB
    # li a3, K6_HALF_WORDS
    call gf2x_mul3_leaf

    # middle = ZM ^ Z0 ^ Z2
    la t3, K6_ZM
    la t4, K6_Z0
    XOR_INTO_WORDS t3, t4, K6_SUBPROD_WORDS

    la t3, K6_ZM
    la t4, K6_Z2
    XOR_INTO_WORDS t3, t4, K6_SUBPROD_WORDS

# RECOMPOSITION
    # out ^= Z0
    mv t3, s0
    la t4, K6_Z0
    XOR_INTO_WORDS t3, t4, K6_SUBPROD_WORDS

    # out[3..] ^= middle
    mv t3, s0
    addi t3, t3, K6_HALF_WORDS * 8
    la t4, K6_ZM
    XOR_INTO_WORDS t3, t4, K6_SUBPROD_WORDS

    # out[6..] ^= Z2
    mv t3, s0
    addi t3, t3, K6_WORDS * 8
    la t4, K6_Z2
    XOR_INTO_WORDS t3, t4, K6_SUBPROD_WORDS

# CLEAN TEMPORARIES
    # la t3, K6_TA
    # ZERO_WORDS t3, K6_HALF_WORDS

    # la t3, K6_TB
    # ZERO_WORDS t3, K6_HALF_WORDS

    # la t3, K6_Z0
    # ZERO_WORDS t3, K6_SUBPROD_WORDS

    # la t3, K6_Z2
    # ZERO_WORDS t3, K6_SUBPROD_WORDS

    # la t3, K6_ZM
    # ZERO_WORDS t3, K6_SUBPROD_WORDS

    ld s2, 0(sp)
    ld s1, 8(sp)
    ld s0, 16(sp)
    ld ra, 24(sp)
    addi sp, sp, 32
    ret

    .size rvv_kara6_mul, .-rvv_kara6_mul

# KARATSUBA_6 SCALAR
    .globl scalar_kara6_mul
    .type scalar_kara6_mul, @function

scalar_kara6_mul:
    addi sp, sp, -32
    sd ra, 24(sp)
    sd s0, 16(sp)
    sd s1, 8(sp)
    sd s2, 0(sp)

    mv s0, a0
    mv s1, a1
    mv s2, a2

    # TA = A0 ^ A1
    la t3, K6_TA

    ld t0, 0(s1)
    ld t1, 24(s1)
    xor t0, t0, t1
    sd t0, 0(t3)

    ld t0, 8(s1)
    ld t1, 32(s1)
    xor t0, t0, t1
    sd t0, 8(t3)

    ld t0, 16(s1)
    ld t1, 40(s1)
    xor t0, t0, t1
    sd t0, 16(t3)

    # TB = B0 ^ B1
    la t3, K6_TB

    ld t0, 0(s2)
    ld t1, 24(s2)
    xor t0, t0, t1
    sd t0, 0(t3)

    ld t0, 8(s2)
    ld t1, 32(s2)
    xor t0, t0, t1
    sd t0, 8(t3)

    ld t0, 16(s2)
    ld t1, 40(s2)
    xor t0, t0, t1
    sd t0, 16(t3)

    # Z0 = A0 * B0
    la a0, K6_Z0
    mv a1, s1
    mv a2, s2
    call gf2x_mul3_leaf

    # Z2 = A1 * B1
    la a0, K6_Z2
    addi a1, s1, 24
    addi a2, s2, 24
    call gf2x_mul3_leaf

    # ZM = TA * TB
    la a0, K6_ZM
    la a1, K6_TA
    la a2, K6_TB
    call gf2x_mul3_leaf

    # M = ZM ^ Z0 ^ Z2
    la t3, K6_ZM
    la t4, K6_Z0
    la t5, K6_Z2

    ld t0, 0(t3)
    ld t1, 0(t4)
    xor t0, t0, t1
    ld t1, 0(t5)
    xor t0, t0, t1
    sd t0, 0(t3)

    ld t0, 8(t3)
    ld t1, 8(t4)
    xor t0, t0, t1
    ld t1, 8(t5)
    xor t0, t0, t1
    sd t0, 8(t3)

    ld t0, 16(t3)
    ld t1, 16(t4)
    xor t0, t0, t1
    ld t1, 16(t5)
    xor t0, t0, t1
    sd t0, 16(t3)

    ld t0, 24(t3)
    ld t1, 24(t4)
    xor t0, t0, t1
    ld t1, 24(t5)
    xor t0, t0, t1
    sd t0, 24(t3)

    ld t0, 32(t3)
    ld t1, 32(t4)
    xor t0, t0, t1
    ld t1, 32(t5)
    xor t0, t0, t1
    sd t0, 32(t3)

    ld t0, 40(t3)
    ld t1, 40(t4)
    xor t0, t0, t1
    ld t1, 40(t5)
    xor t0, t0, t1
    sd t0, 40(t3)

    ld t0, 48(t3)
    ld t1, 48(t4)
    xor t0, t0, t1
    ld t1, 48(t5)
    xor t0, t0, t1
    sd t0, 48(t3)

# RECOMPOSITION
    la t3, K6_Z0
    la t4, K6_ZM
    la t5, K6_Z2

    # out[0] = Z0[0]
    ld t0, 0(t3)
    sd t0, 0(s0)

    # out[1] = Z0[1]
    ld t0, 8(t3)
    sd t0, 8(s0)

    # out[2] = Z0[2]
    ld t0, 16(t3)
    sd t0, 16(s0)

    # out[3] = Z0[3] ^ M[0]
    ld t0, 24(t3)
    ld t1, 0(t4)
    xor t0, t0, t1
    sd t0, 24(s0)

    # out[4] = Z0[4] ^ M[1]
    ld t0, 32(t3)
    ld t1, 8(t4)
    xor t0, t0, t1
    sd t0, 32(s0)

    # out[5] = Z0[5] ^ M[2]
    ld t0, 40(t3)
    ld t1, 16(t4)
    xor t0, t0, t1
    sd t0, 40(s0)

    # out[6] = Z0[6] ^ M[3] ^ Z2[0]
    ld t0, 48(t3)
    ld t1, 24(t4)
    xor t0, t0, t1
    ld t1, 0(t5)
    xor t0, t0, t1
    sd t0, 48(s0)

    # out[7] = M[4] ^ Z2[1]
    ld t0, 32(t4)
    ld t1, 8(t5)
    xor t0, t0, t1
    sd t0, 56(s0)

    # out[8] = M[5] ^ Z2[2]
    ld t0, 40(t4)
    ld t1, 16(t5)
    xor t0, t0, t1
    sd t0, 64(s0)

    # out[9] = M[6] ^ Z2[3]
    ld t0, 48(t4)
    ld t1, 24(t5)
    xor t0, t0, t1
    sd t0, 72(s0)

    # out[10] = Z2[4]
    ld t0, 32(t5)
    sd t0, 80(s0)

    # out[11] = Z2[5]
    ld t0, 40(t5)
    sd t0, 88(s0)

    # out[12] = Z2[6]
    ld t0, 48(t5)
    sd t0, 96(s0)

    ld s2, 0(sp)
    ld s1, 8(sp)
    ld s0, 16(sp)
    ld ra, 24(sp)
    addi sp, sp, 32
    ret

    .size scalar_kara6_mul, .-scalar_kara6_mul