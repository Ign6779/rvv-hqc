/**
 * @file gf32.c
 * @brief GF(2^32) arithmetic used by the FAFFT multiplication.
 */

#include "gf32.h"
#include <stdint.h>
#include <stddef.h>
#include "parameters.h"

static gf32 gf32_reduce(uint64_t x);
static uint64_t gf32_carryless_mul(gf32 a, gf32 b);

/**
 * @brief Feedback bit positions used for modular reduction by FAFFT_GF32_POLY.
 *
 * FAFFT_GF32_POLY represents:
 *
 *     x^32 + x^22 + x^2 + x + 1
 *
 * Bits set at positions:
 *
 *     32, 22, 2, 1, 0
 *
 * During reduction, the leading x^32 term is handled by shifting the high
 * part down by FAFFT_FIELD_BITS. The constant term is handled by the initial
 * XOR with mod. The remaining feedback taps are therefore:
 *
 *     22, 2, 1
 */
static const uint8_t gf32_reduction_taps[] = {22, 2, 1};

/**
 * @brief Reduce a polynomial modulo FAFFT_GF32_POLY in GF(2^32).
 *
 * This function reduces a 64-bit carry-less multiplication result modulo:
 *
 *     x^32 + x^22 + x^2 + x + 1
 *
 * The input has degree at most 62, since it is the product of two degree < 32
 * binary polynomials.
 *
 * The reduction follows the same structure as gf_reduce() in gf.c
 *
 * Because the largest feedback tap is 22, one reduction pass can leave terms
 * above degree 31. Four fixed passes are enough for degree <= 62:
 *
 *     62 -> 52 -> 42 -> 32 -> 22
 *
 * @param[in] x 64-bit input polynomial
 * @return Reduced GF(2^32) field element
 */
static gf32 gf32_reduce(uint64_t x) {
    uint64_t mod;
    const int reduction_steps = 4;
    const size_t gf32_reduction_tap_count = 3;
    const uint64_t low_mask = (1ULL << FAFFT_FIELD_BITS) - 1ULL;

    for (int i = 0; i < reduction_steps; ++i) {
        mod = x >> FAFFT_FIELD_BITS;
        x &= low_mask;
        x ^= mod;

        uint8_t z1 = 0;
        for (size_t j = gf32_reduction_tap_count; j; --j) {
            uint8_t z2 = gf32_reduction_taps[j - 1];
            uint8_t dist = z2 - z1;

            mod <<= dist;
            x ^= mod;
            z1 = z2;
        }
    }

    return (gf32)x;
}

/**
 * @brief Carry-less multiplication of two GF(2^32) elements.
 *
 * This is the same idea as gf_carryless_mul() in gf.c, but widened from
 * 8-bit inputs to 32-bit inputs.
 *
 * It uses 2-bit windows of a and a small table containing:
 *
 *     0, b, x*b, (x+1)*b
 *
 * The selected table entry is XORed into the result at the appropriate shift.
 *
 * @param[in] a First polynomial
 * @param[in] b Second polynomial
 * @return 64-bit unreduced carry-less product
 */
static uint64_t gf32_carryless_mul(gf32 a, gf32 b) {
    uint64_t r = 0;
    uint64_t g;
    uint64_t u[4];

    u[0] = 0;
    u[1] = (uint64_t)b;
    u[2] = ((uint64_t)b) << 1;
    u[3] = u[2] ^ u[1];

    for (uint8_t i = 0; i < FAFFT_FIELD_BITS; i += 2) {
        g = 0;
        uint64_t window = (a >> i) & 3U;

        for (uint8_t j = 0; j < 4; ++j) {
            uint64_t diff = window ^ j;
            uint64_t is_equal = 1ULL ^ ((diff | -diff) >> 63);
            uint64_t mask = -is_equal;

            g ^= u[j] & mask;
        }

        r ^= g << i;
    }

    return r;
}

/**
 * @brief Multiplies two elements of GF(2^32).
 *
 * @param[in] a First field element
 * @param[in] b Second field element
 * @return a * b in GF(2^32)
 */
gf32 gf32_mul(gf32 a, gf32 b) {
    uint64_t tmp = gf32_carryless_mul(a, b);
    return gf32_reduce(tmp);
}

/**
 * @brief Squares an element of GF(2^32).
 *
 * This simple version reuses gf32_mul(). It is not the fastest possible
 * squaring implementation, but it is easy.
 *
 * @param[in] a Field element
 * @return a^2 in GF(2^32)
 */
gf32 gf32_square(gf32 a) {
    return gf32_mul(a, a);
}