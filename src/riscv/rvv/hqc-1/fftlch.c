#include "fftlch.h"

#include <stdint.h>

#include "fftlch_consts.h"
#include "gf32v.h"
#include "gf32v_tower.h"
#include "gft_mul_vi_gf32v.h"
#include "gft_mul_const_tables.h"

/*
 * FFTLCH = the butterfly phase of the FAFFT, ported from BIKE's btfy_65536
 * (bikel1/m4f/btfy_ffft.c) to this plane-first uint64 layout.
 *
 * Layout recap: a gf32v_array is plane[32][32] uint64. Word w (uint64) of a
 * plane holds the bit-p of 64 consecutive field elements (elements 64*w..64*w+63).
 * So element e lives at (word = e/64, bit = e%64). This is a mechanical repack of
 * BIKE's block-first uint32 layout: word w == BIKE blocks (2w, 2w+1), low/high.
 *
 * The transform is 11 butterfly stages, applied largest stride first:
 *   - cross-word (stride >= 64 elements): si = 10,9,8,7,6  -> whole-word pairing
 *   - s5         (stride 32 elements):                    -> intra-word, high<->low
 *   - s0..s4     (stride 16,8,4,2,1):                      -> intra-word  [TODO]
 *
 * Every butterfly is the same shape (the v + w split, BIKE paper p.21):
 *     beta*hi = v*hi (+) w*hi          v: full-field, constant per stage
 *     lo ^= beta*hi                    w: subfield twiddle, per group
 *     hi ^= lo
 *
 * v-part: gf32v_mul_scalar by the stage's full-field constant v(27-si).
 * w-part: gf32v_mul_gf256 by a per-word twiddle table T built from s0_gft.
 *         (F16 sub-case folds into F256: in this tower F16 is the low nibble of
 *          F256, so a twiddle < 16 has the same byte rep either way.)
 */

#define LOW32 UINT64_C(0x00000000ffffffff)

/* Full-field representations of the layer constants v17..v22.
 * (Same values verified in tests/unit/test_gft_mul_vi_gf32v.c.) */
#define V17 0x002e597u
#define V18 0x0052257u
#define V19 0x009b170u
#define V20 0x013572fu
#define V21 0x02c7d24u
#define V22 0x0573762u
#define V23 0x092f9fau
#define V24 0x12f1b8du
#define V25 0x2e55791u
#define V26 0x52267c0u
#define V27 0x9b120beu

/*
 * Per-word w-twiddle tables, in the gf256 operand format expected by
 * gf32v_mul_gf256: only planes 0..7 are meaningful (the bitsliced gf256
 * element, broadcast across all 64 lanes of the word). Built once from s0_gft.
 *
 * NOTE: built lazily at first use. These are pure functions of s0_gft and could
 * be emitted as compile-time constants by a host generator later; this is a
 * one-time cost, not per-call.
 */
static gf32v_array T_cw[5];    /* cross-word stages, index s -> si = 10 - s */
static gf32v_array T_s5;       /* the stride-32 intra-word stage             */
static gf32v_array T_intra[5]; /* intra-word stages s4..s0, index k = 0..4   */
static int tables_ready = 0;

/* Write the gf256 element tw into word w of T (planes 0..7, broadcast). */
static void set_word_twiddle(gf32v_array *T, unsigned w, uint32_t tw) {
    for (unsigned p = 0; p < 8; p++) {
        T->plane[p][w] = ((tw >> p) & 1u) ? ~UINT64_C(0) : UINT64_C(0);
    }
}

/*
 * Build a per-lane gf216 twiddle table for an intra-word stage of stride 2^k.
 * Within a 64-bit word the twiddle varies per lane: lane b (element 64*w+b)
 * belongs to butterfly group (64*w+b) >> (k+1), so its twiddle is
 * s0_gft[(64*w+b) >> (k+1)]. Stored bitsliced across planes 0..15 (F2^16).
 */
static void build_intra_table(gf32v_array *T, unsigned k) {
    gf32v_zero(T);
    for (unsigned w = 0; w < GF32V_WORDS; w++) {
        for (unsigned b = 0; b < GF32V_WORD_BITS; b++) {
            unsigned e  = w * GF32V_WORD_BITS + b;
            uint32_t tw = s0_gft[e >> (k + 1)];
            for (unsigned p = 0; p < 16; p++) {
                if ((tw >> p) & 1u) {
                    T->plane[p][w] |= (UINT64_C(1) << b);
                }
            }
        }
    }
}

static void build_tables(void) {
    for (unsigned s = 0; s < 5; s++) {
        unsigned si = 10u - s;
        unsigned ws = 1u << (si - 6); /* word stride = 2^(si-6) */
        gf32v_zero(&T_cw[s]);
        for (unsigned w = 0; w < GF32V_WORDS; w++) {
            unsigned g = w / (2u * ws); /* butterfly group index */
            set_word_twiddle(&T_cw[s], w, s0_gft[g]);
        }
    }

    /* s5: stride 32 -> exactly one twiddle per word, twiddle = s0_gft[w] */
    gf32v_zero(&T_s5);
    for (unsigned w = 0; w < GF32V_WORDS; w++) {
        set_word_twiddle(&T_s5, w, s0_gft[w]);
    }

    /* s4..s0: stride 2^k, k = 4,3,2,1,0 (per-lane gf216 twiddles) */
    for (unsigned k = 0; k < 5; k++) {
        build_intra_table(&T_intra[k], k);
    }

    tables_ready = 1;
}

/* LMUL=8 plane primitives (gf32v.S). */
extern void gf32v_rvv_zero(gf32v_word_t *a, unsigned nwords);
extern void gf32v_rvv_copy(gf32v_word_t *out, const gf32v_word_t *in, unsigned nwords);
extern void gf32v_rvv_xor_inplace(gf32v_word_t *a, const gf32v_word_t *b, unsigned nwords);

/*
 * v-part: multiply the whole array by a fixed full-field constant (V17..V27).
 * Instead of gf32v_mul_scalar (a full GF(2^32) Karatsuba multiply), apply the
 * constant's plane-XOR table: out_plane[j] = XOR of the input planes selected
 * by mask[j]. Each plane-XOR is a whole-plane operation handled by the LMUL=8
 * vector primitives. Requires out != in (each output plane is built directly
 * from input planes); the FFT always passes distinct buffers. Unknown constants
 * fall back to the general multiply so behaviour is never silently wrong.
 */
static void mul_vconst(gf32v_array *out, const gf32v_array *in, uint32_t v_const) {
    const uint32_t *mask = fafft_gft_const_table(v_const);
    if (mask == 0) {
        gf32v_mul_scalar(out, in, v_const);
        return;
    }
    for (unsigned j = 0; j < GF32V_PLANES; j++) {
        uint32_t bits = mask[j];
        if (bits == 0) {
            gf32v_rvv_zero(&out->plane[j][0], GF32V_WORDS);
            continue;
        }
        unsigned first = (unsigned)__builtin_ctz(bits);
        gf32v_rvv_copy(&out->plane[j][0], &in->plane[first][0], GF32V_WORDS);
        bits &= bits - 1u;
        while (bits) {
            unsigned i = (unsigned)__builtin_ctz(bits);
            bits &= bits - 1u;
            gf32v_rvv_xor_inplace(&out->plane[j][0], &in->plane[i][0], GF32V_WORDS);
        }
    }
}

/*
 * Forward cross-word butterfly stage.
 *   ws      : word stride (partners are word w and word w+ws)
 *   v_const : full-field v-part constant for this stage
 *   T       : per-word gf256 w-twiddle table for this stage
 */
static void fwd_crossword(gf32v_array *A, unsigned ws, uint32_t v_const, const gf32v_array *T) {
    static gf32v_array V, W, bA;

    /* beta*A computed for every word; only the values at the "hi" words are used,
     * and each hi word carries its own group's twiddle (see build_tables). */
    mul_vconst(&V, A, v_const);
    gf32v_mul_gf256(&W, A, T);
    gf32v_add(&bA, &V, &W);

    for (unsigned base = 0; base < GF32V_WORDS; base += 2u * ws) {
        for (unsigned i = 0; i < ws; i++) {
            unsigned lo = base + i;
            unsigned hi = base + ws + i;
            for (unsigned k = 0; k < GF32V_PLANES; k++) {
                A->plane[k][lo] ^= bA.plane[k][hi];
                A->plane[k][hi] ^= A->plane[k][lo];
            }
        }
    }
}

/*
 * Forward s5 stage: stride-32 butterfly living inside each 64-bit word.
 * Low half (bits 0..31) and high half (bits 32..63) are the two partners.
 */
static void fwd_s5(gf32v_array *A) {
    static gf32v_array H, VH, WH, bH;

    /* H = high halves brought down into the low 32 bits (high 32 bits become 0). */
    for (unsigned k = 0; k < GF32V_PLANES; k++) {
        for (unsigned w = 0; w < GF32V_WORDS; w++) {
            H.plane[k][w] = A->plane[k][w] >> 32;
        }
    }

    mul_vconst(&VH, &H, V22);
    gf32v_mul_gf256(&WH, &H, &T_s5);
    gf32v_add(&bH, &VH, &WH); /* beta*hi, sitting in the low 32 bits */

    for (unsigned k = 0; k < GF32V_PLANES; k++) {
        for (unsigned w = 0; w < GF32V_WORDS; w++) {
            uint64_t t = A->plane[k][w] ^ bH.plane[k][w]; /* lo ^= beta*hi */
            A->plane[k][w] = t ^ ((t & LOW32) << 32);     /* hi ^= lo      */
        }
    }
}

/*
 * General intra-word butterfly stage, stride shift = 2^k, partners selected by
 * mask (the bit-k-set lanes). w-part uses an F2^16 per-lane twiddle (gf216).
 */
static void fwd_intra(gf32v_array *A, uint64_t mask, unsigned shift, uint32_t v_const, const gf32v_array *T) {
    static gf32v_array H, VH, WH, bH;

    /* H = hi partners (bit-k-set lanes) brought down by `shift` into lo lanes. */
    for (unsigned k = 0; k < GF32V_PLANES; k++) {
        for (unsigned w = 0; w < GF32V_WORDS; w++) {
            H.plane[k][w] = (A->plane[k][w] & mask) >> shift;
        }
    }

    mul_vconst(&VH, &H, v_const);
    gf32v_mul_gf216(&WH, &H, T);
    gf32v_add(&bH, &VH, &WH); /* beta*hi, sitting in the lo lanes */

    for (unsigned k = 0; k < GF32V_PLANES; k++) {
        for (unsigned w = 0; w < GF32V_WORDS; w++) {
            uint64_t t = A->plane[k][w] ^ bH.plane[k][w]; /* lo ^= beta*hi */
            A->plane[k][w] = t ^ ((t & ~mask) << shift);  /* hi ^= lo      */
        }
    }
}

void fafft_fftlch_inplace(gf32v_array *a) {
    static const unsigned ws_arr[5]    = { 16, 8, 4, 2, 1 };
    static const uint32_t v_cw[5]      = { V17, V18, V19, V20, V21 };

    /* intra-word stages s4..s0: stride 2^k for k = 4,3,2,1,0 */
    static const uint64_t mask_intra[5] = {
        MASK_S4_64, MASK_S3_64, MASK_S2_64, MASK_S1_64, MASK_S0_64
    };
    static const unsigned shift_intra[5] = { 16, 8, 4, 2, 1 };
    static const uint32_t v_intra[5]     = { V23, V24, V25, V26, V27 };
    static const unsigned k_intra[5]     = { 4, 3, 2, 1, 0 };

    if (!tables_ready) {
        build_tables();
    }

    /* cross-word stages: si = 10,9,8,7,6 */
    for (unsigned s = 0; s < 5; s++) {
        fwd_crossword(a, ws_arr[s], v_cw[s], &T_cw[s]);
    }

    /* stride-32 intra-word stage */
    fwd_s5(a);

    /* intra-word stages s4,s3,s2,s1,s0 */
    for (unsigned s = 0; s < 5; s++) {
        unsigned k = k_intra[s];
        fwd_intra(a, mask_intra[s], shift_intra[s], v_intra[s], &T_intra[k]);
    }
}

/*
 * Inverse butterfly stages. Each mirrors its forward counterpart with the two
 * butterfly lines swapped:
 *     forward:  lo ^= beta*hi ; hi ^= lo
 *     inverse:  hi ^= lo       ; lo ^= beta*hi
 * The "hi ^= lo" step recovers the original hi, which is then used to undo the
 * "lo ^= beta*hi" step. Same v constants and twiddle tables as the forward.
 */

static void inv_crossword(gf32v_array *A, unsigned ws, uint32_t v_const, const gf32v_array *T) {
    static gf32v_array V, W, bA;

    /* step 1: hi ^= lo  (recovers original hi at the hi words) */
    for (unsigned base = 0; base < GF32V_WORDS; base += 2u * ws) {
        for (unsigned i = 0; i < ws; i++) {
            unsigned lo = base + i;
            unsigned hi = base + ws + i;
            for (unsigned k = 0; k < GF32V_PLANES; k++) {
                A->plane[k][hi] ^= A->plane[k][lo];
            }
        }
    }

    /* beta*A on the array as it now stands (hi words hold the recovered hi). */
    mul_vconst(&V, A, v_const);
    gf32v_mul_gf256(&W, A, T);
    gf32v_add(&bA, &V, &W);

    /* step 2: lo ^= beta*hi */
    for (unsigned base = 0; base < GF32V_WORDS; base += 2u * ws) {
        for (unsigned i = 0; i < ws; i++) {
            unsigned lo = base + i;
            unsigned hi = base + ws + i;
            for (unsigned k = 0; k < GF32V_PLANES; k++) {
                A->plane[k][lo] ^= bA.plane[k][hi];
            }
        }
    }
}

static void inv_s5(gf32v_array *A) {
    static gf32v_array H, VH, WH, bH;

    /* step 1: hi ^= lo  (high half ^= low half) */
    for (unsigned k = 0; k < GF32V_PLANES; k++) {
        for (unsigned w = 0; w < GF32V_WORDS; w++) {
            uint64_t x = A->plane[k][w];
            A->plane[k][w] = x ^ ((x & LOW32) << 32);
        }
    }

    /* H = recovered hi brought down into the low 32 bits */
    for (unsigned k = 0; k < GF32V_PLANES; k++) {
        for (unsigned w = 0; w < GF32V_WORDS; w++) {
            H.plane[k][w] = A->plane[k][w] >> 32;
        }
    }

    mul_vconst(&VH, &H, V22);
    gf32v_mul_gf256(&WH, &H, &T_s5);
    gf32v_add(&bH, &VH, &WH);

    /* step 2: lo ^= beta*hi */
    for (unsigned k = 0; k < GF32V_PLANES; k++) {
        for (unsigned w = 0; w < GF32V_WORDS; w++) {
            A->plane[k][w] ^= bH.plane[k][w];
        }
    }
}

static void inv_intra(gf32v_array *A, uint64_t mask, unsigned shift, uint32_t v_const, const gf32v_array *T) {
    static gf32v_array H, VH, WH, bH;

    /* step 1: hi ^= lo  (hi lanes ^= corresponding lo lanes shifted up) */
    for (unsigned k = 0; k < GF32V_PLANES; k++) {
        for (unsigned w = 0; w < GF32V_WORDS; w++) {
            uint64_t x = A->plane[k][w];
            A->plane[k][w] = x ^ ((x & ~mask) << shift);
        }
    }

    /* H = recovered hi brought down into the lo lanes */
    for (unsigned k = 0; k < GF32V_PLANES; k++) {
        for (unsigned w = 0; w < GF32V_WORDS; w++) {
            H.plane[k][w] = (A->plane[k][w] & mask) >> shift;
        }
    }

    mul_vconst(&VH, &H, v_const);
    gf32v_mul_gf216(&WH, &H, T);
    gf32v_add(&bH, &VH, &WH);

    /* step 2: lo ^= beta*hi */
    for (unsigned k = 0; k < GF32V_PLANES; k++) {
        for (unsigned w = 0; w < GF32V_WORDS; w++) {
            A->plane[k][w] ^= bH.plane[k][w];
        }
    }
}

void fafft_inverse_fftlch_inplace(gf32v_array *a) {
    /* intra-word stages in forward order s4..s0; reversed here -> s0..s4 */
    static const uint64_t mask_intra[5] = {
        MASK_S0_64, MASK_S1_64, MASK_S2_64, MASK_S3_64, MASK_S4_64
    };
    static const unsigned shift_intra[5] = { 1, 2, 4, 8, 16 };
    static const uint32_t v_intra[5]     = { V27, V26, V25, V24, V23 };
    static const unsigned k_intra[5]     = { 0, 1, 2, 3, 4 };

    /* cross-word: forward used ws_arr[s]={16..1}, v={V17..V21}, T_cw[s] (si=10-s) */
    static const unsigned ws_arr[5] = { 16, 8, 4, 2, 1 };
    static const uint32_t v_cw[5]   = { V17, V18, V19, V20, V21 };

    if (!tables_ready) {
        build_tables();
    }

    /* Reverse of the forward order:
     *   forward: cross-word(10..6), s5, s4..s0
     *   inverse: s0..s4, s5, cross-word(6..10)
     */

    /* intra-word stages s0,s1,s2,s3,s4 */
    for (unsigned s = 0; s < 5; s++) {
        inv_intra(a, mask_intra[s], shift_intra[s], v_intra[s], &T_intra[k_intra[s]]);
    }

    /* stride-32 intra-word stage */
    inv_s5(a);

    /* cross-word stages si = 6,7,8,9,10  (T_cw index s = 4,3,2,1,0) */
    for (int s = 4; s >= 0; s--) {
        inv_crossword(a, ws_arr[s], v_cw[s], &T_cw[s]);
    }
}

void fafft_fftlch(gf32v_array *out, const gf32v_array *in) {
    gf32v_copy(out, in);
    fafft_fftlch_inplace(out);
}

void fafft_inverse_fftlch(gf32v_array *out, const gf32v_array *in) {
    gf32v_copy(out, in);
    fafft_inverse_fftlch_inplace(out);
}
