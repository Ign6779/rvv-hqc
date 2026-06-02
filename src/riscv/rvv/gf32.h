#ifndef HQC_GF32_H
#define HQC_GF32_H

#include <stdint.h>

typedef uint32_t gf32;

/**
 * Multiplies two elements of GF(2^32).
 *
 * Field:
 *   GF(2)[x] / (x^32 + x^22 + x^2 + x + 1)
 *
 * @param[in] a First field element
 * @param[in] b Second field element
 * @return a * b in GF(2^32)
 */
gf32 gf32_mul(gf32 a, gf32 b);

/**
 * Squares an element of GF(2^32).
 *
 * @param[in] a Field element
 * @return a^2 in GF(2^32)
 */
gf32 gf32_square(gf32 a);

#endif