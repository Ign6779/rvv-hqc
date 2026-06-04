#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "gf32v_tower.h"

static uint64_t rng_state = 0x123456789abcdef0ULL;

static uint64_t rng64(void) {
    uint64_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    rng_state = x;
    return x;
}

static void rand_gf32v(gf32v_array *a) {
    for (unsigned i = 0; i < GF32V_PLANES; i++) {
        for (unsigned j = 0; j < GF32V_WORDS; j++) {
            a->plane[i][j] = rng64();
        }
    }
}

static int eq_gf32v(const gf32v_array *a, const gf32v_array *b) {
    return memcmp(a, b, sizeof(*a)) == 0;
}

static void print_first_diff(const gf32v_array *got, const gf32v_array *expected) {
    for (unsigned i = 0; i < GF32V_PLANES; i++) {
        for (unsigned j = 0; j < GF32V_WORDS; j++) {
            uint64_t g = got->plane[i][j];
            uint64_t e = expected->plane[i][j];

            if (g != e) {
                printf("first diff: plane=%u word=%u\n", i, j);
                printf("got      = 0x%016lx\n", g);
                printf("expected = 0x%016lx\n", e);
                printf("xor diff = 0x%016lx\n", g ^ e);
                return;
            }
        }
    }
}

static void set_scalar_gf32v(gf32v_array *out, uint32_t scalar) {
    for (unsigned i = 0; i < GF32V_PLANES; i++) {
        uint64_t mask = (scalar & 1u) ? UINT64_MAX : 0;

        for (unsigned j = 0; j < GF32V_WORDS; j++) {
            out->plane[i][j] = mask;
        }

        scalar >>= 1;
    }
}

static void ref_copy(uint64_t *out, const uint64_t *a, unsigned planes) {
    memcpy(out, a, planes * GF32V_WORDS * sizeof(uint64_t));
}

static void ref_xor(uint64_t *out, const uint64_t *a, const uint64_t *b, unsigned planes) {
    for (unsigned i = 0; i < planes * GF32V_WORDS; i++) {
        out[i] = a[i] ^ b[i];
    }
}

static void ref_xor_inplace(uint64_t *out, const uint64_t *a, unsigned planes) {
    for (unsigned i = 0; i < planes * GF32V_WORDS; i++) {
        out[i] ^= a[i];
    }
}

static void ref_gf4_mul2(uint64_t *out, const uint64_t *a) {
    for (unsigned j = 0; j < GF32V_WORDS; j++) {
        uint64_t a0 = a[0 * GF32V_WORDS + j];
        uint64_t a1 = a[1 * GF32V_WORDS + j];

        out[0 * GF32V_WORDS + j] = a1;
        out[1 * GF32V_WORDS + j] = a0 ^ a1;
    }
}

static void ref_gf4_mul(uint64_t *out, const uint64_t *a, const uint64_t *b) {
    for (unsigned j = 0; j < GF32V_WORDS; j++) {
        uint64_t a0 = a[0 * GF32V_WORDS + j];
        uint64_t a1 = a[1 * GF32V_WORDS + j];

        uint64_t b0 = b[0 * GF32V_WORDS + j];
        uint64_t b1 = b[1 * GF32V_WORDS + j];

        out[0 * GF32V_WORDS + j] = (a0 & b0) ^ (a1 & b1);
        out[1 * GF32V_WORDS + j] = (a0 & b1) ^ (a1 & (b0 ^ b1));
    }
}

static void ref_gf16_mul8(uint64_t *out, const uint64_t *a) {
    uint64_t t[2 * GF32V_WORDS];

    ref_gf4_mul2(t, a + 2 * GF32V_WORDS);
    ref_copy(out + 2 * GF32V_WORDS, a, 2);
    ref_xor(out, a + 2 * GF32V_WORDS, t, 2);
}

static void ref_gf16_mul(uint64_t *out, const uint64_t *a, const uint64_t *b) {
    uint64_t s1[2 * GF32V_WORDS];
    uint64_t s2[2 * GF32V_WORDS];
    uint64_t m0[2 * GF32V_WORDS];
    uint64_t m1[2 * GF32V_WORDS];
    uint64_t m2[2 * GF32V_WORDS];
    uint64_t red[2 * GF32V_WORDS];

    ref_xor(s1, a, a + 2 * GF32V_WORDS, 2);
    ref_xor(s2, b, b + 2 * GF32V_WORDS, 2);

    ref_gf4_mul(m0, a, b);
    ref_gf4_mul(m1, s1, s2);
    ref_gf4_mul(m2, a + 2 * GF32V_WORDS, b + 2 * GF32V_WORDS);

    ref_gf4_mul2(red, m2);

    ref_xor(out, m0, red, 2);
    ref_xor(out + 2 * GF32V_WORDS, m1, m0, 2);
}

static void ref_gf256_mul80(uint64_t *out, const uint64_t *a) {
    uint64_t t[4 * GF32V_WORDS];

    ref_gf16_mul8(out + 4 * GF32V_WORDS, a);
    ref_gf16_mul8(t, a + 4 * GF32V_WORDS);

    ref_xor_inplace(out + 4 * GF32V_WORDS, t, 4);
    ref_gf16_mul8(out, t);
}

static void ref_gf256_mul(uint64_t *out, const uint64_t *a, const uint64_t *b) {
    uint64_t s1[4 * GF32V_WORDS];
    uint64_t s2[4 * GF32V_WORDS];
    uint64_t high[4 * GF32V_WORDS];
    uint64_t red[4 * GF32V_WORDS];

    ref_xor(s1, a, a + 4 * GF32V_WORDS, 4);
    ref_xor(s2, b, b + 4 * GF32V_WORDS, 4);

    ref_gf16_mul(out, a, b);
    ref_gf16_mul(out + 4 * GF32V_WORDS, s1, s2);
    ref_xor_inplace(out + 4 * GF32V_WORDS, out, 4);

    ref_gf16_mul(high, a + 4 * GF32V_WORDS, b + 4 * GF32V_WORDS);
    ref_gf16_mul8(red, high);

    ref_xor_inplace(out, red, 4);
}

static void ref_gf216_mul8000(uint64_t *out, const uint64_t *a) {
    uint64_t t[8 * GF32V_WORDS];

    ref_gf256_mul80(out + 8 * GF32V_WORDS, a);
    ref_gf256_mul80(t, a + 8 * GF32V_WORDS);

    ref_xor_inplace(out + 8 * GF32V_WORDS, t, 8);
    ref_gf256_mul80(out, t);
}

static void ref_gf216_mul(uint64_t *out, const uint64_t *a, const uint64_t *b) {
    uint64_t s1[8 * GF32V_WORDS];
    uint64_t s2[8 * GF32V_WORDS];
    uint64_t high[8 * GF32V_WORDS];
    uint64_t red[8 * GF32V_WORDS];

    ref_xor(s1, a, a + 8 * GF32V_WORDS, 8);
    ref_xor(s2, b, b + 8 * GF32V_WORDS, 8);

    ref_gf256_mul(out, a, b);
    ref_gf256_mul(out + 8 * GF32V_WORDS, s1, s2);
    ref_xor_inplace(out + 8 * GF32V_WORDS, out, 8);

    ref_gf256_mul(high, a + 8 * GF32V_WORDS, b + 8 * GF32V_WORDS);
    ref_gf256_mul80(red, high);

    ref_xor_inplace(out, red, 8);
}

static void ref_gf232_mul(gf32v_array *out, const gf32v_array *a, const gf32v_array *b) {
    uint64_t *o = &out->plane[0][0];
    const uint64_t *aa = &a->plane[0][0];
    const uint64_t *bb = &b->plane[0][0];

    uint64_t s1[16 * GF32V_WORDS];
    uint64_t s2[16 * GF32V_WORDS];
    uint64_t high[16 * GF32V_WORDS];
    uint64_t red[16 * GF32V_WORDS];

    ref_xor(s1, aa, aa + 16 * GF32V_WORDS, 16);
    ref_xor(s2, bb, bb + 16 * GF32V_WORDS, 16);

    ref_gf216_mul(o, aa, bb);
    ref_gf216_mul(o + 16 * GF32V_WORDS, s1, s2);
    ref_xor_inplace(o + 16 * GF32V_WORDS, o, 16);

    ref_gf216_mul(high, aa + 16 * GF32V_WORDS, bb + 16 * GF32V_WORDS);
    ref_gf216_mul8000(red, high);

    ref_xor_inplace(o, red, 16);
}

int main(void) {
    for (unsigned t = 0; t < 100; t++) {
        gf32v_array a;
        gf32v_array b;
        gf32v_array rvv;
        gf32v_array ref;
        gf32v_array two;
        gf32v_array five;

        rand_gf32v(&a);
        rand_gf32v(&b);

        set_scalar_gf32v(&two, 0x2);
        set_scalar_gf32v(&five, 0x5);

        /*
         * Test gf32v_mul wrapper.
         * This also tests the full gf32v_rvv_mul assembly path underneath.
         */
        gf32v_mul(&rvv, &a, &b);
        ref_gf232_mul(&ref, &a, &b);

        if (!eq_gf32v(&rvv, &ref)) {
            printf("FAIL mul test %u\n", t);
            print_first_diff(&rvv, &ref);
            return 1;
        }

        /*
         * Test gf32v_square wrapper.
         * This should be exactly equivalent to a * a.
         */
        gf32v_square(&rvv, &a);
        ref_gf232_mul(&ref, &a, &a);

        if (!eq_gf32v(&rvv, &ref)) {
            printf("FAIL square test %u\n", t);
            return 1;
        }

        /*
         * Test gf32v_mul_0x2 wrapper.
         * Reference is ordinary GF(2^32) multiplication by scalar 0x2.
         */
        gf32v_mul_0x2(&rvv, &a);
        ref_gf232_mul(&ref, &a, &two);

        if (!eq_gf32v(&rvv, &ref)) {
            printf("FAIL mul_0x2 test %u\n", t);
            return 1;
        }

        /*
         * Test gf32v_mul_0x5 wrapper.
         * Reference is ordinary GF(2^32) multiplication by scalar 0x5.
         */
        gf32v_mul_0x5(&rvv, &a);
        ref_gf232_mul(&ref, &a, &five);

        if (!eq_gf32v(&rvv, &ref)) {
            printf("FAIL mul_0x5 test %u\n", t);
            return 1;
        }

        /*
         * Aliasing test: gf32v_mul with out == a.
         */
        gf32v_array expected_alias;
        gf32v_array actual_alias;

        ref_gf232_mul(&expected_alias, &a, &b);

        actual_alias = a;
        gf32v_mul(&actual_alias, &actual_alias, &b);

        if (!eq_gf32v(&actual_alias, &expected_alias)) {
            printf("FAIL mul alias out==a test %u\n", t);
            return 1;
        }

        /*
         * Aliasing test: gf32v_mul with out == b.
         */
        actual_alias = b;
        gf32v_mul(&actual_alias, &a, &actual_alias);

        if (!eq_gf32v(&actual_alias, &expected_alias)) {
            printf("FAIL mul alias out==b test %u\n", t);
            return 1;
        }

        /*
         * Aliasing test: gf32v_square with out == a.
         */
        ref_gf232_mul(&expected_alias, &a, &a);

        actual_alias = a;
        gf32v_square(&actual_alias, &actual_alias);

        if (!eq_gf32v(&actual_alias, &expected_alias)) {
            printf("FAIL square alias test %u\n", t);
            return 1;
        }

        /*
         * Aliasing test: gf32v_mul_0x2 with out == a.
         */
        ref_gf232_mul(&expected_alias, &a, &two);

        actual_alias = a;
        gf32v_mul_0x2(&actual_alias, &actual_alias);

        if (!eq_gf32v(&actual_alias, &expected_alias)) {
            printf("FAIL mul_0x2 alias test %u\n", t);
            return 1;
        }

        /*
         * Aliasing test: gf32v_mul_0x5 with out == a.
         */
        ref_gf232_mul(&expected_alias, &a, &five);

        actual_alias = a;
        gf32v_mul_0x5(&actual_alias, &actual_alias);

        if (!eq_gf32v(&actual_alias, &expected_alias)) {
            printf("FAIL mul_0x5 alias test %u\n", t);
            return 1;
        }
    }

    printf("PASS gf32v_tower\n");
    return 0;
}