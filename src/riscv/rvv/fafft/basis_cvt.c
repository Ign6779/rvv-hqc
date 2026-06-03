/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * FAFFT BasisCvt for HQC on RISC-V RVV.
 *
 * This file adapts the BasisCvt structure from BitPolyMul:
 *   Copyright (C) 2017 Ming-Shing Chen
 *
 * Original project:
 *   BitPolyMul, fast-crypto-lab/bitpolymul
 *
 * Adaptation:
 *   - separated into the HQC RVV FAFFT backend
 *   - x86 SIMD removed
 *   - long XOR ranges delegated to RVV assembly kernels
 */

#include "basis_cvt.h"
#include "fafft_params.h"

#include <string.h>

/*TODO: RVV these*/
extern void fafft_rvv_xor_down_u8(f2 *poly, unsigned st, unsigned len, unsigned diff);
extern void fafft_rvv_xor_up_u8(f2 *poly, unsigned st, unsigned len, unsigned diff);

static inline unsigned get_num_blocks(unsigned poly_len, unsigned blk_size) {
    return poly_len / blk_size;
}

static inline unsigned deg_si(unsigned si) {
    return 1u << si;
}

static unsigned get_si_2_pow(unsigned si, unsigned deg) {
    unsigned si_deg = deg_si(si);
    unsigned r = 1;

    while ((si_deg << r) < deg) {
        r++;
    }

    return 1u << (r - 1);
}

static unsigned get_max_si(unsigned deg) {
    unsigned si = 0;
    unsigned si_attempt = 1;

    while (deg > (1u << si_attempt)) {
        si = si_attempt;
        si_attempt <<= 1;
    }

    return si;
}

/* Division step for representing blocks in powers of s_i. */
static void poly_div(f2 *poly, unsigned n_terms, unsigned blk_size, unsigned si, unsigned pow) {
    if (si == 0) {
        return;
    }

    const unsigned si_degree = deg_si(si) * pow;
    const unsigned deg_diff = si_degree - pow;
    const unsigned deg_blk = get_num_blocks(n_terms, blk_size) - 1;

    const unsigned st = (deg_blk - deg_diff + 1) * blk_size;
    const unsigned len = (deg_blk - si_degree + 1) * blk_size;
    const unsigned diff = deg_diff * blk_size;

    fafft_rvv_xor_down_u8(poly, st, len, diff);
}

/* Represent a polynomial chunk in powers of the selected s_i. */
static void represent_in_si(f2 *poly, unsigned n_terms, unsigned blk_size, unsigned si) {
    if (si == 0) {
        return;
    }

    const unsigned num_blocks = get_num_blocks(n_terms, blk_size);

    if (num_blocks <= 2) {
        return;
    }

    const unsigned degree_in_blocks = num_blocks - 1;
    const unsigned degree_basic_form_si = deg_si(si);

    if (degree_basic_form_si > degree_in_blocks) {
        return;
    }

    unsigned pow = get_si_2_pow(si, degree_in_blocks);

    while (pow > 0) {
        const unsigned chunk_size = blk_size * 2u * pow * deg_si(si);

        for (unsigned i = 0; i < n_terms; i += chunk_size) {
            poly_div(poly + i, chunk_size, blk_size, si, pow);
        }

        pow >>= 1;
    }
}

/* Recursive monomial basis -> LCH / novelpoly basis. */
static void basis_cvt_recursive(f2 *poly, unsigned n_terms, unsigned blk_size) {
    const unsigned num_blocks = get_num_blocks(n_terms, blk_size);

    if (num_blocks <= 2) {
        return;
    }

    const unsigned degree_in_blocks = num_blocks - 1;
    const unsigned si = get_max_si(degree_in_blocks);

    represent_in_si(poly, n_terms, blk_size, si);

    const unsigned new_blk_size = deg_si(si) * blk_size;

    basis_cvt_recursive(poly, n_terms, new_blk_size);

    for (unsigned i = 0; i < n_terms; i += new_blk_size) {
        basis_cvt_recursive(poly + i, new_blk_size, blk_size);
    }
}

void fafft_basis_cvt(f2 *out, const f2 *in, unsigned n) {
    if (out != in) {
        memcpy(out, in, n * sizeof(f2));
    }

    basis_cvt_recursive(out, n, 1);
}


// INVERSE
/* Inverse division step. */
static void inverse_poly_div(f2 *poly, unsigned n_terms, unsigned blk_size, unsigned si, unsigned pow) {
    if (si == 0) {
        return;
    }

    const unsigned si_degree = deg_si(si) * pow;
    const unsigned deg_diff = si_degree - pow;
    const unsigned deg_blk = get_num_blocks(n_terms, blk_size) - 1;

    const unsigned st = blk_size * (si_degree - deg_diff);
    const unsigned len = (deg_blk - si_degree + 1) * blk_size;
    const unsigned diff = deg_diff * blk_size;

    fafft_rvv_xor_up_u8(poly, st, len, diff);
}

/* Inverse representation from powers of s_i. */
static void inverse_represent_in_si(f2 *poly, unsigned n_terms, unsigned blk_size, unsigned si) {
    if (si == 0) {
        return;
    }

    const unsigned num_blocks = get_num_blocks(n_terms, blk_size);

    if (num_blocks <= 2) {
        return;
    }

    const unsigned degree_in_blocks = num_blocks - 1;
    const unsigned degree_basic_form_si = deg_si(si);

    if (degree_basic_form_si > degree_in_blocks) {
        return;
    }

    unsigned pow = 1;

    while (pow * deg_si(si) <= degree_in_blocks) {
        const unsigned chunk_size = blk_size * 2u * pow * deg_si(si);

        for (unsigned i = 0; i < n_terms; i += chunk_size) {
            inverse_poly_div(poly + i, chunk_size, blk_size, si, pow);
        }

        pow <<= 1;
    }
}

/* Recursive LCH / novelpoly basis -> monomial basis. */
static void inverse_basis_cvt_recursive(f2 *poly, unsigned n_terms, unsigned blk_size) {
    const unsigned num_blocks = get_num_blocks(n_terms, blk_size);

    if (num_blocks <= 2) {
        return;
    }

    const unsigned degree_in_blocks = num_blocks - 1;
    const unsigned si = get_max_si(degree_in_blocks);
    const unsigned new_blk_size = deg_si(si) * blk_size;

    for (unsigned i = 0; i < n_terms; i += new_blk_size) {
        inverse_basis_cvt_recursive(poly + i, new_blk_size, blk_size);
    }

    inverse_basis_cvt_recursive(poly, n_terms, new_blk_size);
    inverse_represent_in_si(poly, n_terms, blk_size, si);
}

void fafft_inverse_basis_cvt(f2 *out, const f2 *in, unsigned n) {
    if (out != in) {
        memcpy(out, in, n * sizeof(f2));
    }

    inverse_basis_cvt_recursive(out, n, 1);
}