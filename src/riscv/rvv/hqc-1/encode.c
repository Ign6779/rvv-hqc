// Adapted from BIKE encode_to_gft/decode_from_gft circuits.
#include "encode.h"

#include <stddef.h>
#include <stdint.h>

#include "fafft_params.h"
#include "fafft_layout.h"

#if FAFFT_N_BITS != 65536
#error "encode.c currently supports only FAFFT_N_BITS=65536"
#endif

#if GF32V_PLANES != 32
#error "encode.c requires GF32V_PLANES=32"
#endif

#if FAFFT_NP != 2048
#error "encode.c currently assumes FAFFT_NP=2048"
#endif

typedef uint64_t plane_t[GF32V_WORDS];

static inline void plane_copy(uint64_t dst[GF32V_WORDS],
                              const uint64_t src[GF32V_WORDS])
{
    for (size_t i = 0; i < GF32V_WORDS; i++) {
        dst[i] = src[i];
    }
}

static inline void plane_xor(uint64_t dst[GF32V_WORDS],
                             const uint64_t src[GF32V_WORDS])
{
    for (size_t i = 0; i < GF32V_WORDS; i++) {
        dst[i] ^= src[i];
    }
}

static void encode_circuit_u64(uint64_t out[32][GF32V_WORDS],
                               const uint64_t b[32][GF32V_WORDS])
{
    plane_t g0, g1, g2, g3, g4, g5, g6, g7, g8, g9, g10, g11;
    plane_t f0, f1, f2, f4, f5, f6, f7, f8, f9, f10, f11;
    plane_t f12, f14, f15, f16, f17, f18, f19, f20, f22, f23;
    plane_t f24, f26, f27, f28, f29, f30, f31;

    plane_copy(g0, b[0]);
    plane_copy(g1, b[1]);
    plane_copy(g2, b[2]);
    plane_xor(g0, g1);
    plane_xor(g0, g2);
    plane_copy(f0, g0);

    plane_copy(f1, b[14]);
    plane_copy(f2, b[13]);
    plane_copy(f4, b[4]);
    plane_copy(f5, b[6]);
    plane_copy(f6, b[12]);
    plane_copy(f7, b[1]);

    plane_copy(f8, b[4]);
    plane_xor(f8, b[12]);

    plane_copy(f9, b[8]);
    plane_copy(f10, b[10]);
    plane_copy(f11, b[11]);
    plane_copy(f12, b[2]);

    plane_copy(f14, b[10]);
    plane_xor(f14, b[11]);

    plane_copy(f15, b[4]);
    plane_xor(f15, b[14]);

    plane_copy(f17, b[15]);
    plane_copy(f19, b[3]);
    plane_copy(f20, b[9]);
    plane_copy(f22, b[3]);
    plane_copy(f24, b[5]);
    plane_copy(f26, b[9]);

    plane_copy(f27, b[11]);
    plane_xor(f27, b[7]);

    plane_copy(f28, b[3]);
    plane_copy(f31, b[7]);

    plane_copy(g0, f1);
    plane_copy(g1, f2);
    plane_copy(g3, f4);
    plane_copy(g4, f9);
    plane_copy(g5, f11);
    plane_copy(g6, f12);

    plane_copy(g10, f28);
    plane_copy(g11, f31);
    plane_xor(g10, g11);
    plane_copy(f28, g10);

    plane_copy(f16, f24);
    plane_copy(g2, f17);

    plane_xor(g3, g4);
    plane_xor(g2, g6);

    plane_copy(f29, g1);

    plane_copy(g10, f24);
    plane_xor(g10, g1);
    plane_copy(f24, g10);

    plane_copy(g10, f0);
    plane_xor(g10, g5);
    plane_copy(f0, g10);

    plane_copy(g10, f10);
    plane_xor(g10, g4);
    plane_copy(f10, g10);

    plane_copy(g11, f17);
    plane_xor(g4, g11);

    plane_copy(g9, f16);
    plane_copy(g8, f26);

    plane_copy(g11, f17);
    plane_xor(g8, g11);

    plane_copy(g10, f19);
    plane_xor(g10, g5);
    plane_copy(f19, g10);

    plane_copy(g11, f5);
    plane_xor(g6, g11);

    plane_copy(g11, f14);
    plane_xor(g2, g11);

    plane_copy(g10, f14);
    plane_copy(g11, f8);
    plane_xor(g10, g11);
    plane_copy(f14, g10);

    plane_copy(g10, f8);
    plane_copy(g11, f7);
    plane_xor(g10, g11);
    plane_copy(f8, g10);

    plane_copy(f18, g9);
    plane_copy(g7, f17);

    plane_copy(g10, f6);
    plane_xor(g10, g1);
    plane_copy(f6, g10);

    plane_copy(g11, f31);
    plane_xor(g0, g11);

    plane_copy(g10, f10);
    plane_xor(g10, g5);
    plane_copy(f10, g10);

    plane_copy(g11, f26);
    plane_xor(g0, g11);

    plane_copy(g11, f16);
    plane_xor(g0, g11);

    plane_copy(g10, f22);
    plane_copy(g11, f26);
    plane_xor(g10, g11);
    plane_copy(f22, g10);

    plane_xor(g4, g9);

    plane_copy(g10, f31);
    plane_xor(g10, g1);
    plane_copy(f31, g10);

    plane_copy(g10, f8);
    plane_xor(g10, g9);
    plane_copy(f8, g10);

    plane_copy(g10, f6);
    plane_xor(g10, g6);
    plane_copy(f6, g10);

    plane_copy(g10, f29);
    plane_xor(g10, g8);
    plane_copy(f29, g10);

    plane_copy(g11, f27);
    plane_xor(g6, g11);

    plane_copy(g10, f27);
    plane_xor(g10, g8);
    plane_copy(f27, g10);

    plane_copy(g10, f5);
    plane_xor(g10, g8);
    plane_copy(f5, g10);

    plane_copy(g11, f15);
    plane_xor(g1, g11);

    plane_copy(g11, f24);
    plane_xor(g5, g11);

    plane_copy(g10, f7);
    plane_copy(g11, f22);
    plane_xor(g10, g11);
    plane_copy(f7, g10);

    plane_copy(g11, f28);
    plane_xor(g9, g11);

    plane_copy(g10, f17);
    plane_copy(g11, f24);
    plane_xor(g10, g11);
    plane_copy(f17, g10);

    plane_copy(g11, f8);
    plane_xor(g1, g11);

    plane_copy(f23, f19);

    plane_copy(g10, f5);
    plane_xor(g10, g2);
    plane_copy(f5, g10);

    plane_copy(g10, f22);
    plane_copy(g11, f31);
    plane_xor(g10, g11);
    plane_copy(f22, g10);

    plane_copy(g11, f8);
    plane_xor(g0, g11);

    plane_copy(f30, g5);

    plane_copy(g11, f6);
    plane_xor(g5, g11);

    plane_copy(g11, f19);
    plane_xor(g7, g11);

    plane_copy(g10, f31);
    plane_xor(g10, g7);
    plane_copy(f31, g10);

    plane_xor(g7, g3);

    plane_copy(g10, f0);
    plane_xor(g10, g3);
    plane_copy(f0, g10);

    plane_copy(g10, f16);
    plane_copy(g11, f7);
    plane_xor(g10, g11);
    plane_copy(f16, g10);

    plane_copy(g11, f7);
    plane_xor(g3, g11);

    plane_copy(g10, f18);
    plane_copy(g11, f29);
    plane_xor(g10, g11);
    plane_copy(f18, g10);

    plane_copy(g10, f20);
    plane_xor(g10, g9);
    plane_copy(f20, g10);

    plane_copy(g10, f26);
    plane_copy(g11, f30);
    plane_xor(g10, g11);
    plane_copy(f26, g10);

    plane_xor(g1, g3);

    plane_copy(g10, f19);
    plane_copy(g11, f29);
    plane_xor(g10, g11);
    plane_copy(f19, g10);

    plane_copy(g10, f28);
    plane_copy(g11, f30);
    plane_xor(g10, g11);
    plane_copy(f28, g10);

    plane_copy(g10, f6);
    plane_xor(g10, g4);
    plane_copy(f6, g10);

    plane_copy(g10, f7);
    plane_xor(g10, g2);
    plane_copy(f7, g10);

    plane_xor(g3, g6);

    plane_copy(g10, f14);
    plane_copy(g11, f18);
    plane_xor(g10, g11);
    plane_copy(f14, g10);

    plane_xor(g7, g0);

    plane_copy(g10, f10);
    plane_copy(g11, f8);
    plane_xor(g10, g11);
    plane_copy(f10, g10);

    plane_xor(g2, g7);

    plane_copy(g11, f28);
    plane_xor(g8, g11);

    plane_xor(g5, g2);

    plane_copy(g10, f5);
    plane_xor(g10, g1);
    plane_copy(f5, g10);

    plane_copy(g11, f14);
    plane_xor(g6, g11);

    plane_xor(g4, g3);

    plane_copy(g10, f15);
    plane_xor(g10, g6);
    plane_copy(f15, g10);

    plane_copy(out[0], f0);
    plane_copy(out[1], g0);
    plane_copy(out[2], g1);
    plane_copy(out[3], g2);
    plane_copy(out[4], g3);
    plane_copy(out[5], f5);
    plane_copy(out[6], f6);
    plane_copy(out[7], f7);
    plane_copy(out[8], f8);
    plane_copy(out[9], g4);
    plane_copy(out[10], f10);
    plane_copy(out[11], g5);
    plane_copy(out[12], g6);
    plane_copy(out[13], g7);
    plane_copy(out[14], f14);
    plane_copy(out[15], f15);
    plane_copy(out[16], f16);
    plane_copy(out[17], f17);
    plane_copy(out[18], f18);
    plane_copy(out[19], f19);
    plane_copy(out[20], f20);
    plane_copy(out[21], g8);
    plane_copy(out[22], f22);
    plane_copy(out[23], f23);
    plane_copy(out[24], f24);
    plane_copy(out[25], g9);
    plane_copy(out[26], f26);
    plane_copy(out[27], f27);
    plane_copy(out[28], f28);
    plane_copy(out[29], f29);
    plane_copy(out[30], f30);
    plane_copy(out[31], f31);
}

void fafft_encode(gf32v_array *out, const uint64_t *poly)
{
    const uint64_t (*in_planes)[GF32V_WORDS];

    in_planes = (const uint64_t (*)[GF32V_WORDS])poly;
    encode_circuit_u64(out->plane, in_planes);
}

static void decode_circuit_u64(uint64_t out[32][GF32V_WORDS],
                               const uint64_t b[32][GF32V_WORDS])
{
    plane_t g0, g1, g2, g3, g4, g5, g6, g7, g8, g9, g10, g11;
    (void)sizeof(g11); //compiler complains otherwise
    plane_t f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11;
    plane_t f12, f13, f14, f15, f16, f17, f18, f19, f20, f21;
    plane_t f22, f23, f24, f25, f26, f27, f28, f29, f30, f31;

    plane_copy(g0, b[0]);
    plane_xor(g0, b[5]);
    plane_xor(g0, b[7]);
    plane_xor(g0, b[10]);
    plane_xor(g0, b[12]);
    plane_xor(g0, b[13]);
    plane_xor(g0, b[28]);
    plane_xor(g0, b[30]);
    plane_copy(f0, g0);

    plane_copy(g0, b[16]);
    plane_xor(g0, b[18]);
    plane_copy(f1, g0);

    plane_copy(g0, b[2]);
    plane_xor(g0, b[3]);
    plane_xor(g0, b[5]);
    plane_xor(g0, b[11]);
    plane_xor(g0, b[18]);
    plane_xor(g0, b[22]);
    plane_copy(f2, g0);

    plane_copy(g0, b[18]);
    plane_xor(g0, b[23]);
    plane_copy(f3, g0);

    plane_copy(g0, b[1]);
    plane_xor(g0, b[2]);
    plane_xor(g0, b[11]);
    plane_xor(g0, b[31]);
    plane_copy(f4, g0);

    plane_copy(g0, b[28]);
    plane_xor(g0, b[22]);
    plane_copy(f5, g0);

    plane_copy(g0, b[3]);
    plane_xor(g0, b[10]);
    plane_xor(g0, b[15]);
    plane_xor(g0, b[20]);
    plane_xor(g0, b[28]);
    plane_copy(f6, g0);

    plane_copy(f7, b[27]);

    plane_copy(g0, b[3]);
    plane_xor(g0, b[4]);
    plane_xor(g0, b[8]);
    plane_xor(g0, b[11]);
    plane_copy(f8, g0);

    plane_copy(g0, b[24]);
    plane_xor(g0, b[20]);
    plane_copy(f9, g0);

    plane_copy(g0, b[5]);
    plane_xor(g0, b[9]);
    plane_xor(g0, b[13]);
    plane_xor(g0, b[20]);
    plane_copy(f10, g0);

    plane_copy(g0, b[24]);
    plane_xor(g0, b[30]);
    plane_copy(f11, g0);

    plane_copy(g0, b[2]);
    plane_xor(g0, b[6]);
    plane_xor(g0, b[7]);
    plane_xor(g0, b[15]);
    plane_xor(g0, b[23]);
    plane_copy(f12, g0);

    plane_copy(g0, b[28]);
    plane_xor(g0, b[31]);
    plane_copy(f13, g0);

    plane_copy(g0, b[2]);
    plane_xor(g0, b[5]);
    plane_xor(g0, b[12]);
    plane_xor(g0, b[15]);
    plane_xor(g0, b[26]);
    plane_copy(f14, g0);

    plane_copy(f15, b[19]);

    plane_copy(g0, b[8]);
    plane_xor(g0, b[10]);
    plane_xor(g0, b[11]);
    plane_xor(g0, b[15]);
    plane_xor(g0, b[24]);
    plane_copy(f16, g0);

    plane_copy(g0, b[26]);
    plane_xor(g0, b[29]);
    plane_xor(g0, b[22]);
    plane_copy(f17, g0);

    plane_copy(g0, b[4]);
    plane_xor(g0, b[7]);
    plane_xor(g0, b[9]);
    plane_xor(g0, b[31]);
    plane_copy(f18, g0);

    plane_copy(g0, b[29]);
    plane_xor(g0, b[23]);
    plane_copy(f19, g0);

    plane_copy(g0, b[1]);
    plane_xor(g0, b[4]);
    plane_xor(g0, b[5]);
    plane_xor(g0, b[22]);
    plane_copy(f20, g0);

    plane_copy(f21, b[21]);

    plane_copy(g0, b[3]);
    plane_xor(g0, b[5]);
    plane_xor(g0, b[7]);
    plane_xor(g0, b[14]);
    plane_xor(g0, b[20]);
    plane_copy(f22, g0);

    plane_copy(f23, b[25]);

    plane_copy(g0, b[1]);
    plane_xor(g0, b[2]);
    plane_xor(g0, b[9]);
    plane_xor(g0, b[12]);
    plane_copy(f24, g0);

    plane_copy(g0, b[20]);
    plane_xor(g0, b[22]);
    plane_xor(g0, b[30]);
    plane_copy(f25, g0);

    plane_copy(g0, b[10]);
    plane_xor(g0, b[4]);
    plane_xor(g0, b[6]);
    plane_copy(f26, g0);

    plane_copy(g0, b[26]);
    plane_xor(g0, b[30]);
    plane_xor(g0, b[31]);
    plane_copy(f27, g0);

    plane_copy(g0, b[2]);
    plane_xor(g0, b[8]);
    plane_xor(g0, b[13]);
    plane_xor(g0, b[22]);
    plane_copy(f28, g0);

    plane_copy(g0, b[18]);
    plane_xor(g0, b[14]);
    plane_copy(f29, g0);

    plane_copy(g0, b[24]);
    plane_xor(g0, b[29]);
    plane_xor(g0, b[14]);
    plane_copy(f30, g0);

    plane_copy(f31, b[17]);

    plane_copy(g0, f4);
    plane_copy(g1, f5);
    plane_copy(g2, f15);
    plane_copy(g3, f18);
    plane_copy(g4, f21);
    plane_copy(g5, f22);
    plane_copy(g6, f23);
    plane_copy(g7, f26);
    plane_copy(g8, f27);
    plane_copy(g9, f30);

    plane_copy(g10, f19);
    plane_xor(g10, g2);
    plane_copy(f19, g10);

    plane_copy(g10, f6);
    plane_xor(g10, g2);
    plane_copy(f6, g10);

    plane_copy(g10, f20);
    plane_xor(g10, g2);
    plane_copy(f20, g10);

    plane_xor(g0, g8);

    plane_copy(g10, f12);
    plane_xor(g10, f7);
    plane_copy(f12, g10);

    plane_xor(g8, f7);
    plane_xor(g3, g7);
    plane_xor(g1, g6);

    plane_copy(g10, f28);
    plane_xor(g10, g2);
    plane_copy(f28, g10);

    plane_xor(g7, f7);

    plane_copy(g10, f7);
    plane_xor(g10, g2);
    plane_copy(f7, g10);

    plane_copy(g10, f3);
    plane_xor(g10, g6);
    plane_copy(f3, g10);

    plane_xor(g2, g6);
    plane_xor(g6, f31);
    plane_xor(g0, f14);

    plane_copy(g10, f14);
    plane_xor(g10, f29);
    plane_copy(f14, g10);

    plane_copy(g10, f14);
    plane_xor(g10, g5);
    plane_copy(f14, g10);

    plane_xor(g5, f25);

    plane_copy(g10, f10);
    plane_xor(g10, g2);
    plane_copy(f10, g10);

    plane_copy(g10, f24);
    plane_xor(g10, g2);
    plane_copy(f24, g10);

    plane_xor(g2, g1);

    plane_copy(g10, f20);
    plane_xor(g10, f31);
    plane_copy(f20, g10);

    plane_copy(g10, f11);
    plane_xor(g10, f31);
    plane_copy(f11, g10);

    plane_copy(g10, f31);
    plane_xor(g10, f13);
    plane_copy(f31, g10);

    plane_copy(g10, f13);
    plane_xor(g10, g1);
    plane_copy(f13, g10);

    plane_xor(g2, f13);

    plane_copy(g10, f13);
    plane_xor(g10, g4);
    plane_copy(f13, g10);

    plane_copy(g10, f0);
    plane_xor(g10, g4);
    plane_copy(f0, g10);

    plane_copy(g10, f8);
    plane_xor(g10, g1);
    plane_copy(f8, g10);

    plane_xor(g1, f17);

    plane_copy(g10, f17);
    plane_xor(g10, g4);
    plane_copy(f17, g10);

    plane_copy(g10, f2);
    plane_xor(g10, g4);
    plane_copy(f2, g10);

    plane_copy(g10, f20);
    plane_xor(g10, f29);
    plane_copy(f20, g10);

    plane_copy(g10, f29);
    plane_xor(g10, g9);
    plane_copy(f29, g10);

    plane_xor(g2, f3);
    plane_xor(g1, f29);

    plane_copy(g10, f8);
    plane_xor(g10, f25);
    plane_copy(f8, g10);

    plane_copy(g10, f28);
    plane_xor(g10, f19);
    plane_copy(f28, g10);

    plane_xor(g3, f12);

    plane_copy(g10, f12);
    plane_xor(g10, f1);
    plane_copy(f12, g10);

    plane_copy(g10, f16);
    plane_xor(g10, f1);
    plane_copy(f16, g10);

    plane_copy(g10, f1);
    plane_xor(g10, f29);
    plane_copy(f1, g10);

    plane_copy(g10, f17);
    plane_xor(g10, f19);
    plane_copy(f17, g10);

    plane_copy(g10, f3);
    plane_xor(g10, g4);
    plane_copy(f3, g10);

    plane_copy(g10, f3);
    plane_xor(g10, g1);
    plane_copy(f3, g10);

    plane_copy(g10, f10);
    plane_xor(g10, f19);
    plane_copy(f10, g10);

    plane_copy(g10, f1);
    plane_xor(g10, f3);
    plane_copy(f1, g10);

    plane_copy(g10, f3);
    plane_xor(g10, g2);
    plane_copy(f3, g10);

    plane_xor(g4, f9);
    plane_xor(g4, f11);

    plane_xor(g7, g8);
    plane_xor(g7, f25);

    plane_xor(g5, g0);
    plane_xor(g5, f6);

    plane_copy(g10, f6);
    plane_xor(g10, f19);
    plane_copy(f6, g10);

    plane_xor(g9, f19);

    plane_copy(g10, f11);
    plane_xor(g10, f29);
    plane_copy(f11, g10);

    plane_copy(g10, f9);
    plane_xor(g10, g6);
    plane_copy(f9, g10);

    plane_copy(g10, f2);
    plane_xor(g10, f16);
    plane_copy(f2, g10);

    plane_copy(g10, f16);
    plane_xor(g10, g0);
    plane_copy(f16, g10);

    plane_xor(g0, f11);
    plane_xor(g6, f7);

    plane_copy(g10, f0);
    plane_xor(g10, g8);
    plane_copy(f0, g10);

    plane_xor(g5, g4);

    plane_copy(g10, f7);
    plane_xor(g10, g8);
    plane_copy(f7, g10);

    plane_copy(g10, f6);
    plane_xor(g10, g9);
    plane_copy(f6, g10);

    plane_copy(g10, f16);
    plane_xor(g10, g9);
    plane_copy(f16, g10);

    plane_copy(g10, f24);
    plane_xor(g10, f9);
    plane_copy(f24, g10);

    plane_copy(g10, f12);
    plane_xor(g10, f11);
    plane_copy(f12, g10);

    plane_xor(g8, f3);

    plane_copy(g10, f20);
    plane_xor(g10, f7);
    plane_copy(f20, g10);

    plane_xor(g9, g3);
    plane_xor(g3, f28);

    plane_copy(g10, f11);
    plane_xor(g10, g1);
    plane_copy(f11, g10);

    plane_xor(g1, g6);
    plane_xor(g0, g6);
    plane_xor(g3, f2);
    plane_xor(g8, f13);

    plane_copy(g10, f14);
    plane_xor(g10, f13);
    plane_copy(f14, g10);

    plane_copy(g10, f13);
    plane_xor(g10, f19);
    plane_copy(f13, g10);

    plane_xor(g6, g2);

    plane_copy(g10, f24);
    plane_xor(g10, f28);
    plane_copy(f24, g10);

    plane_copy(g10, f24);
    plane_xor(g10, g7);
    plane_copy(f24, g10);

    plane_copy(g10, f10);
    plane_xor(g10, g2);
    plane_copy(f10, g10);

    plane_xor(g5, g9);

    plane_copy(g10, f12);
    plane_xor(g10, g3);
    plane_copy(f12, g10);

    plane_copy(g10, f31);
    plane_xor(g10, f29);
    plane_copy(f31, g10);

    plane_copy(g10, f28);
    plane_xor(g10, g4);
    plane_copy(f28, g10);

    plane_copy(g10, f29);
    plane_xor(g10, g4);
    plane_copy(f29, g10);

    plane_copy(g10, f28);
    plane_xor(g10, f20);
    plane_copy(f28, g10);

    plane_copy(g10, f0);
    plane_xor(g10, g8);
    plane_copy(f0, g10);

    plane_xor(g3, g0);

    plane_copy(g10, f17);
    plane_xor(g10, g8);
    plane_copy(f17, g10);

    plane_xor(g7, f8);

    plane_copy(g10, f25);
    plane_xor(g10, f11);
    plane_copy(f25, g10);

    plane_copy(g10, f10);
    plane_xor(g10, f16);
    plane_copy(f10, g10);

    plane_copy(g10, f2);
    plane_xor(g10, f11);
    plane_copy(f2, g10);

    plane_copy(g10, f20);
    plane_xor(g10, f12);
    plane_copy(f20, g10);

    plane_xor(g4, f13);

    plane_copy(g10, f9);
    plane_xor(g10, g2);
    plane_copy(f9, g10);

    plane_xor(g7, g9);

    plane_copy(g10, f19);
    plane_xor(g10, g6);
    plane_copy(f19, g10);

    plane_xor(g9, f10);

    plane_copy(g10, f6);
    plane_xor(g10, f17);
    plane_copy(f6, g10);

    plane_xor(g0, f2);

    plane_copy(g10, f16);
    plane_xor(g10, g5);
    plane_copy(f16, g10);

    plane_copy(g10, f8);
    plane_xor(g10, g5);
    plane_copy(f8, g10);

    plane_copy(out[0], f0);
    plane_copy(out[1], f1);
    plane_copy(out[2], f2);
    plane_copy(out[3], f3);
    plane_copy(out[4], g0);
    plane_copy(out[5], g1);
    plane_copy(out[6], f6);
    plane_copy(out[7], f7);
    plane_copy(out[8], f8);
    plane_copy(out[9], f9);
    plane_copy(out[10], f10);
    plane_copy(out[11], f11);
    plane_copy(out[12], f12);
    plane_copy(out[13], f13);
    plane_copy(out[14], f14);
    plane_copy(out[15], g2);
    plane_copy(out[16], f16);
    plane_copy(out[17], f17);
    plane_copy(out[18], g3);
    plane_copy(out[19], f19);
    plane_copy(out[20], f20);
    plane_copy(out[21], g4);
    plane_copy(out[22], g5);
    plane_copy(out[23], g6);
    plane_copy(out[24], f24);
    plane_copy(out[25], f25);
    plane_copy(out[26], g7);
    plane_copy(out[27], g8);
    plane_copy(out[28], f28);
    plane_copy(out[29], f29);
    plane_copy(out[30], g9);
    plane_copy(out[31], f31);
}

void fafft_decode(uint64_t *poly, const gf32v_array *in)
{
    uint64_t (*out_planes)[GF32V_WORDS];

    out_planes = (uint64_t (*)[GF32V_WORDS])poly;
    decode_circuit_u64(out_planes, in->plane);
}