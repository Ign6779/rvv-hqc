// tests/unit/test_gft_mul_vi_gf32v.c

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gft_mul_vi_gf32v.h"
#include "gf32v_tower.h"

static uint64_t rng_state = UINT64_C(0x123456789abcdef0);

static uint64_t rng64(void) {
    uint64_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    rng_state = x;
    return x;
}

static void zero_gf32v(gf32v_array *a) {
    memset(a, 0, sizeof(*a));
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

typedef struct {
    const char *name;
    uint32_t scalar;
    fafft_gft_mul_vi_gf32v_fn fn;
} gft_mul_test_case;

static const gft_mul_test_case cases[] = {
    { "v17", 0x002e597u, fafft_gft_mul_v17_gf32v },
    { "v18", 0x0052257u, fafft_gft_mul_v18_gf32v },
    { "v19", 0x009b170u, fafft_gft_mul_v19_gf32v },
    { "v20", 0x013572fu, fafft_gft_mul_v20_gf32v },
    { "v21", 0x02c7d24u, fafft_gft_mul_v21_gf32v },
    { "v22", 0x0573762u, fafft_gft_mul_v22_gf32v },
    { "v23", 0x092f9fau, fafft_gft_mul_v23_gf32v },
    { "v24", 0x12f1b8du, fafft_gft_mul_v24_gf32v },
    { "v25", 0x2e55791u, fafft_gft_mul_v25_gf32v },
    { "v26", 0x52267c0u, fafft_gft_mul_v26_gf32v },
    { "v27", 0x9b120beu, fafft_gft_mul_v27_gf32v },
};

static int test_random_full_words(void) {
    for (unsigned c = 0; c < sizeof(cases) / sizeof(cases[0]); c++) {
        for (unsigned t = 0; t < 100; t++) {
            gf32v_array in;
            gf32v_array got;
            gf32v_array expected;

            rand_gf32v(&in);

            /*
             * Independent reference:
             * multiply the whole gf32v_array by the scalar constant.
             */
            gf32v_mul_scalar(&expected, &in, cases[c].scalar);

            /*
             * The generated function only computes one word index w.
             * So call it once for every word.
             */
            zero_gf32v(&got);

            for (unsigned w = 0; w < GF32V_WORDS; w++) {
                cases[c].fn(&got, &in, w);
            }

            if (!eq_gf32v(&got, &expected)) {
                printf("FAIL %s random test %u\n", cases[c].name, t);
                print_first_diff(&got, &expected);
                return 1;
            }
        }

        printf("PASS %s random full-word tests\n", cases[c].name);
    }

    return 0;
}

static int test_basis_mappings(void) {
    for (unsigned c = 0; c < sizeof(cases) / sizeof(cases[0]); c++) {
        for (unsigned in_plane = 0; in_plane < GF32V_PLANES; in_plane++) {
            gf32v_array in;
            gf32v_array got;
            gf32v_array expected;

            zero_gf32v(&in);

            /*
             * Basis-style test:
             * only one input plane is non-zero.
             * This catches wrong input-plane dependencies in the mapping.
             */
            for (unsigned w = 0; w < GF32V_WORDS; w++) {
                in.plane[in_plane][w] =
                    UINT64_C(0x8000000000000001) ^ ((uint64_t)in_plane << 32) ^ w;
            }

            gf32v_mul_scalar(&expected, &in, cases[c].scalar);

            zero_gf32v(&got);

            for (unsigned w = 0; w < GF32V_WORDS; w++) {
                cases[c].fn(&got, &in, w);
            }

            if (!eq_gf32v(&got, &expected)) {
                printf("FAIL %s basis mapping test input_plane=%u\n",
                       cases[c].name,
                       in_plane);
                print_first_diff(&got, &expected);
                return 1;
            }
        }

        printf("PASS %s basis mapping tests\n", cases[c].name);
    }

    return 0;
}

static int test_alias_out_equals_in(void) {
    for (unsigned c = 0; c < sizeof(cases) / sizeof(cases[0]); c++) {
        for (unsigned t = 0; t < 20; t++) {
            gf32v_array in;
            gf32v_array actual;
            gf32v_array expected;

            rand_gf32v(&in);

            gf32v_mul_scalar(&expected, &in, cases[c].scalar);

            actual = in;

            for (unsigned w = 0; w < GF32V_WORDS; w++) {
                cases[c].fn(&actual, &actual, w);
            }

            if (!eq_gf32v(&actual, &expected)) {
                printf("FAIL %s alias out==in test %u\n", cases[c].name, t);
                print_first_diff(&actual, &expected);
                return 1;
            }
        }

        printf("PASS %s alias tests\n", cases[c].name);
    }

    return 0;
}

int main(void) {
    if (test_random_full_words() != 0) {
        return 1;
    }

    if (test_basis_mappings() != 0) {
        return 1;
    }

    if (test_alias_out_equals_in() != 0) {
        return 1;
    }

    printf("PASS gft_mul_vi_gf32v\n");
    return 0;
}