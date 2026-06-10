#ifndef FAFFT_ENCODE_H
#define FAFFT_ENCODE_H

#include <stdint.h>
#include "fafft_layout.h"

void fafft_encode(gf32v_array *out, const uint64_t *poly);
void fafft_decode(uint64_t *poly, const gf32v_array *in);

#endif