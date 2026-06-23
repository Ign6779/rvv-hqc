/**
 * @file gf_vect.h
 * @brief Batched GF(2^8) arithmetic for RISC-V Vector (base V, length-agnostic).
 *
 * These are the vector counterparts of the scalar routines in src/riscv/common/gf.c.

 * Field: GF(2^8) with PARAM_GF_POLY = 0x11D (x^8 + x^4 + x^3 + x^2 + 1).
 */

#ifndef HQC_GF_VECT_H
#define HQC_GF_VECT_H

#include <stddef.h>
#include <stdint.h>

/** out[k] = carryless(a[k], b[k]). the unreduced 15-bit product (no field reduction). */
void gf_carryless_mul_vect(uint16_t *out, const uint16_t *a, const uint16_t *b, size_t n);

/** out[k] = in[k] mod PARAM_GF_POLY. reduces a degree-<=14 polynomial to GF(2^8). */
void gf_reduce_vect(uint16_t *out, const uint16_t *in, size_t n);

/** out[k] = a[k] * b[k] in GF(2^8). */
void gf_mul_vect(uint16_t *out, const uint16_t *a, const uint16_t *b, size_t n);

/** out[k] = a[k]^2 in GF(2^8). */
void gf_square_vect(uint16_t *out, const uint16_t *a, size_t n);

/** out[k] = a[k]^(-1) in GF(2^8) (matches the scalar gf_inverse). */
void gf_inverse_vect(uint16_t *out, const uint16_t *a, size_t n);

#endif  // HQC_GF_VECT_H