/**
 * @file benchmark_profile.c
 * @brief Breaks Keygen/Encaps/Decaps down into cycle-percentage categories
 * (polynomial multiplication, code encode/decode, vector generation,
 * SHAKE/hash, other) for building a pie chart.
 *
 * This mirrors crypto_kem_keypair/enc/dec (kem.c) and hqc_pke_keygen/encrypt/decrypt
 * (hqc.c) call-for-call, but wraps each sub-step in a cycle counter instead of
 * modifying the library itself.
 */
#define _DEFAULT_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#include "code.h"
#include "crypto_memset.h"
#include "data_structures.h"
#include "gf2x.h"
#include "parameters.h"
#include "parsing.h"
#include "symmetric.h"
#include "vector.h"

#if defined(__x86_64__) || defined(__i386__)
#include <x86intrin.h>
#endif

#define NB_TEST    10
#define NB_SAMPLES 10

typedef enum {
    CAT_POLY_MUL = 0,
    CAT_CODE_ENCODE,
    CAT_CODE_DECODE,
    CAT_VECTOR_GEN,
    CAT_SHAKE_HASH,
    CAT_COUNT
} category_t;

static const char *category_name[CAT_COUNT] = {
    "Polynomial multiplication",
    "Code encode (RS+RM)",
    "Code decode (RS+RM)",
    "Vector generation",
    "SHAKE / hash",
};

static uint64_t g_cycles[CAT_COUNT];

static inline uint64_t cpucycles(void) {
#if defined(__riscv)
    uint64_t cycles;
    __asm__ __volatile__(
        "fence\n\t"
        "csrr %0, cycle\n\t"
        : "=r"(cycles)
        :
        : "memory");
    return cycles;
#elif defined(__x86_64__) || defined(__i386__)
    return __rdtsc();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
#endif
}

#define TIME_CAT(cat, stmt)                            \
    do {                                                \
        uint64_t _t0 = cpucycles();                     \
        stmt;                                            \
        g_cycles[cat] += cpucycles() - _t0;               \
    } while (0)

/* ---- Instrumented mirrors of hqc_pke_keygen/encrypt/decrypt (src/ref/hqc.c) ---- */

static void instrumented_pke_keygen(uint8_t *ek_pke, uint8_t *dk_pke, uint8_t *seed) {
    uint8_t keypair_seed[2 * SEED_BYTES] = {0};
    uint8_t *seed_dk = keypair_seed;
    uint8_t *seed_ek = keypair_seed + SEED_BYTES;
    shake256_xof_ctx dk_xof_ctx = {0};
    shake256_xof_ctx ek_xof_ctx = {0};

    uint64_t x[VEC_N_SIZE_64] = {0};
    uint64_t y[VEC_N_SIZE_64] = {0};
    uint64_t h[VEC_N_SIZE_64] = {0};
    uint64_t s[VEC_N_SIZE_64] = {0};

    TIME_CAT(CAT_SHAKE_HASH, hash_i(keypair_seed, seed));

    TIME_CAT(CAT_SHAKE_HASH, xof_init(&dk_xof_ctx, seed_dk, SEED_BYTES));
    TIME_CAT(CAT_VECTOR_GEN, vect_sample_fixed_weight1(&dk_xof_ctx, y, PARAM_OMEGA));
    TIME_CAT(CAT_VECTOR_GEN, vect_sample_fixed_weight1(&dk_xof_ctx, x, PARAM_OMEGA));

    TIME_CAT(CAT_SHAKE_HASH, xof_init(&ek_xof_ctx, seed_ek, SEED_BYTES));
    TIME_CAT(CAT_VECTOR_GEN, vect_set_random(&ek_xof_ctx, h));
    TIME_CAT(CAT_POLY_MUL, vect_mul(s, y, h));
    vect_add(s, x, s, VEC_N_SIZE_64);

    memcpy(ek_pke, seed_ek, SEED_BYTES);
    memcpy(ek_pke + SEED_BYTES, s, VEC_N_SIZE_BYTES);
    memcpy(dk_pke, seed_dk, SEED_BYTES);

    memset_zero(keypair_seed, sizeof keypair_seed);
    memset_zero(x, sizeof x);
    memset_zero(y, sizeof y);
    memset_zero(&dk_xof_ctx, sizeof dk_xof_ctx);
}

static void instrumented_pke_encrypt(ciphertext_pke_t *c_pke, const uint8_t *ek_pke, const uint64_t *m,
                                      const uint8_t *theta) {
    shake256_xof_ctx theta_xof_ctx = {0};
    uint64_t h[VEC_N_SIZE_64] = {0};
    uint64_t s[VEC_N_SIZE_64] = {0};
    uint64_t r1[VEC_N_SIZE_64] = {0};
    uint64_t r2[VEC_N_SIZE_64] = {0};
    uint64_t e[VEC_N_SIZE_64] = {0};
    uint64_t tmp[VEC_N_SIZE_64] = {0};

    TIME_CAT(CAT_SHAKE_HASH, xof_init(&theta_xof_ctx, theta, SEED_BYTES));

    hqc_ek_pke_from_string(h, s, ek_pke);

    TIME_CAT(CAT_VECTOR_GEN, vect_sample_fixed_weight2(&theta_xof_ctx, r2, PARAM_OMEGA_R));
    TIME_CAT(CAT_VECTOR_GEN, vect_sample_fixed_weight2(&theta_xof_ctx, e, PARAM_OMEGA_E));
    TIME_CAT(CAT_VECTOR_GEN, vect_sample_fixed_weight2(&theta_xof_ctx, r1, PARAM_OMEGA_R));

    TIME_CAT(CAT_POLY_MUL, vect_mul(c_pke->u, r2, h));
    vect_add(c_pke->u, r1, c_pke->u, VEC_N_SIZE_64);

    TIME_CAT(CAT_CODE_ENCODE, code_encode(c_pke->v, m));

    TIME_CAT(CAT_POLY_MUL, vect_mul(tmp, r2, s));
    vect_add(tmp, e, tmp, VEC_N_SIZE_64);
    vect_truncate(tmp);
    vect_add(c_pke->v, c_pke->v, tmp, VEC_N1N2_SIZE_64);

    memset_zero(r1, sizeof r1);
    memset_zero(r2, sizeof r2);
    memset_zero(e, sizeof e);
    memset_zero(tmp, sizeof tmp);
    memset_zero(&theta_xof_ctx, sizeof theta_xof_ctx);
}

static uint8_t instrumented_pke_decrypt(uint64_t *m, const uint8_t *dk_pke, const ciphertext_pke_t *c_pke) {
    uint64_t y[VEC_N_SIZE_64] = {0};
    uint64_t tmp1[VEC_N_SIZE_64] = {0};
    uint64_t tmp2[VEC_N_SIZE_64] = {0};

    hqc_dk_pke_from_string(y, dk_pke);

    TIME_CAT(CAT_POLY_MUL, vect_mul(tmp1, y, c_pke->u));
    vect_truncate(tmp1);
    vect_add(tmp2, c_pke->v, tmp1, VEC_N1N2_SIZE_64);

    TIME_CAT(CAT_CODE_DECODE, code_decode(m, tmp2));

    memset_zero(y, sizeof y);
    memset_zero(tmp1, sizeof tmp1);
    memset_zero(tmp2, sizeof tmp2);

    return 0;
}

/* ---- Instrumented mirrors of crypto_kem_keypair/enc/dec (src/common/kem.c) ---- */

static int instrumented_kem_keypair(uint8_t *ek_kem, uint8_t *dk_kem) {
    uint8_t seed_kem[SEED_BYTES] = {0};
    uint8_t sigma[PARAM_SECURITY_BYTES] = {0};
    uint8_t seed_pke[SEED_BYTES] = {0};
    shake256_xof_ctx ctx_kem;

    uint8_t ek_pke[PUBLIC_KEY_BYTES] = {0};
    uint8_t dk_pke[SEED_BYTES] = {0};

    TIME_CAT(CAT_SHAKE_HASH, prng_get_bytes(seed_kem, SEED_BYTES));

    TIME_CAT(CAT_SHAKE_HASH, {
        xof_init(&ctx_kem, seed_kem, SEED_BYTES);
        xof_get_bytes(&ctx_kem, seed_pke, SEED_BYTES);
        xof_get_bytes(&ctx_kem, sigma, PARAM_SECURITY_BYTES);
    });

    instrumented_pke_keygen(ek_pke, dk_pke, seed_pke);

    memcpy(ek_kem, ek_pke, PUBLIC_KEY_BYTES);
    memcpy(dk_kem, ek_kem, PUBLIC_KEY_BYTES);
    memcpy(dk_kem + PUBLIC_KEY_BYTES, dk_pke, SEED_BYTES);
    memcpy(dk_kem + PUBLIC_KEY_BYTES + SEED_BYTES, sigma, PARAM_SECURITY_BYTES);
    memcpy(dk_kem + PUBLIC_KEY_BYTES + SEED_BYTES + PARAM_SECURITY_BYTES, seed_kem, SEED_BYTES);

    memset_zero(seed_kem, sizeof seed_kem);
    memset_zero(sigma, sizeof sigma);
    memset_zero(seed_pke, sizeof seed_pke);
    memset_zero(dk_pke, sizeof dk_pke);

    return 0;
}

static int instrumented_kem_enc(uint8_t *c_kem, uint8_t *K, const uint8_t *ek_kem) {
    uint8_t m[PARAM_SECURITY_BYTES] = {0};
    uint8_t K_theta[SHARED_SECRET_BYTES + SEED_BYTES] = {0};
    uint8_t theta[SEED_BYTES] = {0};
    uint8_t hash_ek_kem[SEED_BYTES] = {0};
    ciphertext_kem_t c_kem_t = {0};

    TIME_CAT(CAT_SHAKE_HASH, {
        prng_get_bytes(m, PARAM_SECURITY_BYTES);
        prng_get_bytes(c_kem_t.salt, SALT_BYTES);
    });

    TIME_CAT(CAT_SHAKE_HASH, {
        hash_h(hash_ek_kem, ek_kem);
        hash_g(K_theta, hash_ek_kem, m, c_kem_t.salt);
    });
    memcpy(theta, K_theta + SEED_BYTES, SEED_BYTES);

    instrumented_pke_encrypt(&c_kem_t.c_pke, ek_kem, (uint64_t *)m, theta);

    hqc_c_kem_to_string(c_kem, &c_kem_t);
    memcpy(K, K_theta, SHARED_SECRET_BYTES);

    memset_zero(m, sizeof m);
    memset_zero(K_theta, sizeof K_theta);
    memset_zero(theta, sizeof theta);

    return 0;
}

static int instrumented_kem_dec(uint8_t *K_prime, const uint8_t *c_kem, const uint8_t *dk_kem) {
    uint8_t ek_pke[PUBLIC_KEY_BYTES] = {0};
    uint8_t dk_pke[SEED_BYTES] = {0};
    uint8_t sigma[PARAM_SECURITY_BYTES] = {0};
    uint8_t m_prime[PARAM_SECURITY_BYTES] = {0};
    uint8_t hash_ek_kem[SEED_BYTES] = {0};
    uint8_t K_theta_prime[SHARED_SECRET_BYTES + SEED_BYTES] = {0};
    uint8_t K_bar[SHARED_SECRET_BYTES] = {0};
    uint8_t theta_prime[SEED_BYTES] = {0};
    ciphertext_kem_t c_kem_t = {0};
    ciphertext_kem_t c_kem_prime_t = {0};
    uint8_t result;

    memcpy(ek_pke, dk_kem, PUBLIC_KEY_BYTES);
    memcpy(dk_pke, dk_kem + PUBLIC_KEY_BYTES, SEED_BYTES);
    memcpy(sigma, dk_kem + PUBLIC_KEY_BYTES + SEED_BYTES, PARAM_SECURITY_BYTES);

    hqc_c_kem_from_string(&c_kem_t.c_pke, c_kem_t.salt, c_kem);

    result = instrumented_pke_decrypt((uint64_t *)m_prime, dk_pke, &c_kem_t.c_pke);

    TIME_CAT(CAT_SHAKE_HASH, {
        hash_h(hash_ek_kem, ek_pke);
        hash_g(K_theta_prime, hash_ek_kem, m_prime, c_kem_t.salt);
    });
    memcpy(K_prime, K_theta_prime, SHARED_SECRET_BYTES);
    memcpy(theta_prime, K_theta_prime + SHARED_SECRET_BYTES, SEED_BYTES);

    instrumented_pke_encrypt(&c_kem_prime_t.c_pke, ek_pke, (uint64_t *)m_prime, theta_prime);
    memcpy(c_kem_prime_t.salt, c_kem_t.salt, SALT_BYTES);

    TIME_CAT(CAT_SHAKE_HASH, hash_j(K_bar, hash_ek_kem, sigma, &c_kem_t));
    result |= vect_compare((uint8_t *)c_kem_t.c_pke.u, (uint8_t *)c_kem_prime_t.c_pke.u, VEC_N_SIZE_BYTES);
    result |= vect_compare((uint8_t *)c_kem_t.c_pke.v, (uint8_t *)c_kem_prime_t.c_pke.v, VEC_N1N2_SIZE_BYTES);
    result |= vect_compare(c_kem_t.salt, c_kem_prime_t.salt, SALT_BYTES);
    result -= 1;
    for (size_t i = 0; i < SHARED_SECRET_BYTES; ++i) {
        K_prime[i] = (K_prime[i] & result) ^ (K_bar[i] & ~result);
    }

    memset_zero(dk_pke, sizeof dk_pke);
    memset_zero(sigma, sizeof sigma);
    memset_zero(m_prime, sizeof m_prime);
    memset_zero(K_theta_prime, sizeof K_theta_prime);
    memset_zero(K_bar, sizeof K_bar);
    memset_zero(theta_prime, sizeof theta_prime);

    return 0;
}

/* ---- Driver ---- */

static void reset_categories(void) {
    memset(g_cycles, 0, sizeof g_cycles);
}

static void report(const char *phase, uint64_t total) {
    uint64_t tracked = 0;
    for (int i = 0; i < CAT_COUNT; i++) tracked += g_cycles[i];
    uint64_t other = total > tracked ? total - tracked : 0;

    printf("\n--- %s ---\n", phase);
    for (int i = 0; i < CAT_COUNT; i++) {
        printf("%-28s %6.2f%%\n", category_name[i], 100.0 * (double)g_cycles[i] / (double)total);
    }
    printf("%-28s %6.2f%%\n", "Other (parsing/adds/memcpy)", 100.0 * (double)other / (double)total);
}

int main(void) {
    unsigned char pk[PUBLIC_KEY_BYTES];
    unsigned char sk[SECRET_KEY_BYTES];
    unsigned char ct[CIPHERTEXT_BYTES];
    unsigned char ss1[SHARED_SECRET_BYTES];
    unsigned char ss2[SHARED_SECRET_BYTES];

    unsigned char seed[48] = {0};
    syscall(SYS_getrandom, seed, 48, 0);
    prng_init(seed, NULL, 48, 0);

    const int iters = NB_TEST * NB_SAMPLES;

    /* Keygen */
    reset_categories();
    uint64_t t0 = cpucycles();
    for (int i = 0; i < iters; i++) {
        instrumented_kem_keypair(pk, sk);
    }
    uint64_t keygen_total = cpucycles() - t0;
    report("Keygen", keygen_total);

    /* Encaps */
    reset_categories();
    t0 = cpucycles();
    for (int i = 0; i < iters; i++) {
        instrumented_kem_enc(ct, ss1, pk);
    }
    uint64_t encaps_total = cpucycles() - t0;
    report("Encaps", encaps_total);

    /* Decaps */
    instrumented_kem_enc(ct, ss1, pk);
    reset_categories();
    t0 = cpucycles();
    for (int i = 0; i < iters; i++) {
        instrumented_kem_dec(ss2, ct, sk);
        if (memcmp(ss1, ss2, SHARED_SECRET_BYTES) != 0) {
            fprintf(stderr, "decaps mismatch\n");
            return 1;
        }
    }
    uint64_t decaps_total = cpucycles() - t0;
    report("Decaps", decaps_total);

    printf("\n");
    return 0;
}
