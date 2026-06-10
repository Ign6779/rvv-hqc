#ifndef FAFFT_GFT_MUL_VI_H
#define FAFFT_GFT_MUL_VI_H

#include <stdint.h>

typedef void (*fafft_gft_mul_vi_fn)(uint64_t out[32], const uint64_t in[32]);

void fafft_gft_mul_v17(uint64_t out[32], const uint64_t in[32]);
void fafft_gft_mul_v18(uint64_t out[32], const uint64_t in[32]);
void fafft_gft_mul_v19(uint64_t out[32], const uint64_t in[32]);
void fafft_gft_mul_v20(uint64_t out[32], const uint64_t in[32]);
void fafft_gft_mul_v21(uint64_t out[32], const uint64_t in[32]);
void fafft_gft_mul_v22(uint64_t out[32], const uint64_t in[32]);
void fafft_gft_mul_v23(uint64_t out[32], const uint64_t in[32]);
void fafft_gft_mul_v24(uint64_t out[32], const uint64_t in[32]);
void fafft_gft_mul_v25(uint64_t out[32], const uint64_t in[32]);
void fafft_gft_mul_v26(uint64_t out[32], const uint64_t in[32]);
void fafft_gft_mul_v27(uint64_t out[32], const uint64_t in[32]);

#endif