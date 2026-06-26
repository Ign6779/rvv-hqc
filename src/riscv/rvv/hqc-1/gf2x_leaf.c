#include <stdint.h>
#include <stddef.h>
#include <string.h>

void gf2x_mul3_leaf(uint64_t *restrict r,
                    const uint64_t *restrict a,
                    const uint64_t *restrict b);

/*
 * Constant-time carry-less multiply of two 3-word (192-bit) GF(2)[x]
 * polynomials, producing a 7-word buffer (the true product is <= 384 bits,
 * so r[6] stays 0; the 7th word matches the K6 sub-product buffer size).
 *
 * Branchless bit-serial schoolbook:
 *   - The only loop variable `j` is the public bit index 0..63; every shift
 *     amount is derived from it, never from operand data.
 *   - Each bit of `a` selects a shifted copy of `b` via an arithmetic mask
 *     (mask = 0 or all-ones), so there is no secret-dependent branch.
 *   - No table is indexed by operand data, so there is no secret-dependent
 *     memory access. This removes the cache-timing side channel that the
 *     previous 4-bit-nibble lookup table had.
 */
static inline void gf2x_mul3_ct(uint64_t *restrict r,
                                const uint64_t *restrict a,
                                const uint64_t *restrict b)
{
    const uint64_t a0 = a[0];
    const uint64_t a1 = a[1];
    const uint64_t a2 = a[2];

    const uint64_t b0 = b[0];
    const uint64_t b1 = b[1];
    const uint64_t b2 = b[2];

    uint64_t r0 = 0, r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;

    for (unsigned j = 0; j < 64; j++) {
        /*
         * bs = b << j, held in 4 words (192-bit b shifted by <= 63 fits in
         * 256 bits). The carry from the previous word is
         *   (b[k] >> (64 - j))
         * which is undefined for j == 0; compute it as ((b[k] >> 1) >> inv)
         * with inv = 63 - j, which is well-defined for all j in 0..63 and
         * yields 0 when j == 0 (no carry on a zero shift).
         */
        const unsigned inv = 63u - j;
        const uint64_t bs0 = b0 << j;
        const uint64_t bs1 = (b1 << j) ^ ((b0 >> 1) >> inv);
        const uint64_t bs2 = (b2 << j) ^ ((b1 >> 1) >> inv);
        const uint64_t bs3 = (b2 >> 1) >> inv;

        /* mask = all-ones if bit j of a[i] is set, else 0 */
        const uint64_t m0 = (uint64_t)0 - ((a0 >> j) & 1U);
        const uint64_t m1 = (uint64_t)0 - ((a1 >> j) & 1U);
        const uint64_t m2 = (uint64_t)0 - ((a2 >> j) & 1U);

        /* word i of a contributes b << j into r starting at word i */
        r0 ^= bs0 & m0;
        r1 ^= (bs1 & m0) ^ (bs0 & m1);
        r2 ^= (bs2 & m0) ^ (bs1 & m1) ^ (bs0 & m2);
        r3 ^= (bs3 & m0) ^ (bs2 & m1) ^ (bs1 & m2);
        r4 ^=               (bs3 & m1) ^ (bs2 & m2);
        r5 ^=                             (bs3 & m2);
    }

    r[0] = r0;
    r[1] = r1;
    r[2] = r2;
    r[3] = r3;
    r[4] = r4;
    r[5] = r5;
    r[6] = 0;
}

void gf2x_mul3_leaf(uint64_t *restrict r,
                    const uint64_t *restrict a,
                    const uint64_t *restrict b)
{
    gf2x_mul3_ct(r, a, b);
}
