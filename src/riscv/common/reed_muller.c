/**
 * @file reed_muller.c
 * @brief Constant time implementation of Reed-Muller code RM(1,7)
 */

#include "reed_muller.h"
#include <stdint.h>
#include <string.h>
#include "data_structures.h"
#include "parameters.h"

#define MULTIPLICITY CEIL_DIVIDE(PARAM_N2, 128)

extern void rm_encode_vect(uint32_t *temp, const uint8_t *msg, long n1);
extern void rm_hadamard_vect(int16_t *a, int16_t *b, long n1);
extern void rm_find_peaks_vect(int16_t *out, const int16_t *transform, long n1);

static void expand_transpose(int16_t *work, const rm_codeword_t *codeArray, long n1) {
    memset(work, 0, (size_t)128 * n1 * sizeof(int16_t));
    for (long l = 0; l < n1; l++) {
        for (long c = 0; c < MULTIPLICITY; c++) {
            const rm_codeword_t *cw = &codeArray[l * MULTIPLICITY + c];
            for (long part = 0; part < 4; part++) {
                uint32_t w = cw->u32[part];
                for (long bit = 0; bit < 32; bit++) {
                    work[(part * 32 + bit) * n1 + l] += (int16_t)((w >> bit) & 1);
                }
            }
        }
    }
}

void reed_muller_encode(uint64_t *cdw, const uint64_t *msg) {
    const uint8_t *message_array = (const uint8_t *)msg;
    rm_codeword_t *codeArray = (rm_codeword_t *)cdw;
    static uint32_t temp[4 * VEC_N1_SIZE_BYTES];

    rm_encode_vect(temp, message_array, VEC_N1_SIZE_BYTES);

    for (long i = 0; i < VEC_N1_SIZE_BYTES; i++) {
        rm_codeword_t cw;
        cw.u32[0] = temp[0 * VEC_N1_SIZE_BYTES + i];
        cw.u32[1] = temp[1 * VEC_N1_SIZE_BYTES + i];
        cw.u32[2] = temp[2 * VEC_N1_SIZE_BYTES + i];
        cw.u32[3] = temp[3 * VEC_N1_SIZE_BYTES + i];
        for (long c = 0; c < MULTIPLICITY; c++) {
            codeArray[i * MULTIPLICITY + c] = cw;
        }
    }
}

void reed_muller_decode(uint64_t *msg, const uint64_t *cdw) {
    uint8_t *message_array = (uint8_t *)msg;
    const rm_codeword_t *codeArray = (const rm_codeword_t *)cdw;
    const long n1 = VEC_N1_SIZE_BYTES;

    static int16_t bufA[128 * VEC_N1_SIZE_BYTES];
    static int16_t bufB[128 * VEC_N1_SIZE_BYTES];
    static int16_t peaks[VEC_N1_SIZE_BYTES];

    expand_transpose(bufA, codeArray, n1);
    rm_hadamard_vect(bufA, bufB, n1);

    for (long l = 0; l < n1; l++) {
        bufB[l] -= 64 * MULTIPLICITY;
    }

    rm_find_peaks_vect(peaks, bufB, n1);

    for (long l = 0; l < n1; l++) {
        message_array[l] = (uint8_t)peaks[l];
    }
}
