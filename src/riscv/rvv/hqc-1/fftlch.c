#include "fftlch.h"

#include <stdint.h>

#include "fftlch_consts.h"
#include "gf32v.h"
#include "gft_mul_vi_gf32v.h"

//might move these to fftlch_consts
#define LOW32   UINT64_C(0x00000000ffffffff)
#define HIGH32  UINT64_C(0xffffffff00000000)
#define FULL64  UINT64_C(0xffffffffffffffff)

void fafft_fftlch_inplace(gf32v_array *a)
{
    /*
     * Dummy placeholder only.
     * TODO: implement forward FFTLCH in-place.
     */
    (void)a;
}

void fafft_inverse_fftlch_inplace(gf32v_array *a)
{
    /*
     * Dummy placeholder only.
     * TODO: implement inverse FFTLCH in-place.
     */
    (void)a;
}

void fafft_fftlch(gf32v_array *out, const gf32v_array *in)
{
    /*
     * Dummy placeholder only.
     * TODO: implement forward FFTLCH.
     */
    gf32v_copy(out, in);
}

void fafft_inverse_fftlch(gf32v_array *out, const gf32v_array *in)
{
    /*
     * Dummy placeholder only.
     * TODO: implement inverse FFTLCH.
     */
    gf32v_copy(out, in);
}