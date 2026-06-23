/**
 * @file gf_vect.c
 * @brief Plain-C orchestration on top of the gf_vect.S vector leaves.
 *
 * gf_inverse_vect is just the scalar addition chain applied to whole arrays:
 * every step is one of the batched .S kernels, so this file needs no vector
 * code (and therefore no RVV intrinsics) at all.
 */

#include "gf_vect.h"

#include <string.h>

/*
 * Inverse via the same addition chain as the scalar gf_inverse:
 *   1 2 3 4 7 11 15 30 60 120 127 254   (a^254 = a^(-1); 0 maps to 0).
 * Tile the array so the three temporaries stay on the stack; each chain step
 * is a batched op over the current block. GFV_BLK is a tuning knob, not a bound.
 */
#define GFV_BLK 64

void gf_inverse_vect(uint16_t *out, const uint16_t *a, size_t n) {
    uint16_t inv[GFV_BLK], t1[GFV_BLK], t2[GFV_BLK];

    for (size_t off = 0; off < n; off += GFV_BLK) {
        size_t blk = (n - off < GFV_BLK) ? (n - off) : GFV_BLK;
        const uint16_t *ab = a + off;

        gf_square_vect(inv, ab, blk);   /* a^2   */
        gf_mul_vect(t1, inv, ab, blk);  /* a^3   */
        gf_square_vect(inv, inv, blk);  /* a^4   */
        gf_mul_vect(t2, inv, t1, blk);  /* a^7   */
        gf_mul_vect(t1, inv, t2, blk);  /* a^11  */
        gf_mul_vect(inv, t1, inv, blk); /* a^15  */
        gf_square_vect(inv, inv, blk);  /* a^30  */
        gf_square_vect(inv, inv, blk);  /* a^60  */
        gf_square_vect(inv, inv, blk);  /* a^120 */
        gf_mul_vect(inv, inv, t2, blk); /* a^127 */
        gf_square_vect(inv, inv, blk);  /* a^254 */

        memcpy(out + off, inv, blk * sizeof(uint16_t));
    }
}
