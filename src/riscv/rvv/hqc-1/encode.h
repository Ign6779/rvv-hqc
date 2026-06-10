#ifndef FAFFT_ENCODE_H
#define FAFFT_ENCODE_H

#include <stdint.h>
#include "fafft_layout.h"

void fafft_encode(gf32v_array *out, const uint64_t *in);
void fafft_decode(uint64_t *out, const gf32v_array *in);

#endif