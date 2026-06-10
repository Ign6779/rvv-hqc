#ifndef REDUCE_H
#define REDUCE_H

#include <stdint.h>
#include "parameters.h"
#include "fafft_params.h"

void fafft_reduce(uint64_t *out, const uint64_t *in);

#endif