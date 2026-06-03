#ifndef FAFFT_H
#define FAFFT_H
#include <stdint.h>

void fafft_mul_unreduced(uint64_t *out, const uint64_t *a, const uint64_t *b);

void fafft_mul_reduced(uint64_t *out, const uint64_t *a, const uint64_t *b);

#endif