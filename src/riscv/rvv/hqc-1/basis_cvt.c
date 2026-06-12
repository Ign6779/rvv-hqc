//need to include reference to BIKE implementation

#include "basis_cvt.h"
#include "fafft_params.h"

#include <stdint.h>
#include <string.h>

#if FAFFT_N_BITS != 65536
#error "basis_cvt currently supports only HQC-1 / FAFFT_N_BITS=65536"
#endif

/*
 * Private RVV helpers implemented in basis_cvt.S.
 *
 * These are flat-buffer XOR kernels, not gf32v_array kernels.
 */
extern void fafft_rvv_xor_up_u8(uint8_t *poly, size_t dst_off, size_t len, size_t diff);
extern void fafft_rvv_xor_down_u8(uint8_t *poly, size_t dst_end_off, size_t len, size_t diff);

static void bc_1_256(uint32_t *poly, unsigned n_256bit);
static void ibc_1_256(uint32_t *poly, unsigned n_256bit);

static void repr_s8_1_65536(uint32_t *poly);
static void irepr_s8_1_65536(uint32_t *poly);

static void cvt_to_fft_256_65536(uint32_t *poly);
static void icvt_to_poly_256_65536(uint32_t *poly);

void fafft_basis_cvt_inplace(uint64_t *poly) {
    uint32_t *p = (uint32_t *)poly;

    repr_s8_1_65536(p);
    cvt_to_fft_256_65536(p);
    bc_1_256(p, 65536 / 256);
}

void fafft_inverse_basis_cvt_inplace(uint64_t *poly) {
    uint32_t *p = (uint32_t *)poly;

    ibc_1_256(p, 65536 / 256);
    icvt_to_poly_256_65536(p);
    irepr_s8_1_65536(p);
}

void fafft_basis_cvt(uint64_t *out, const uint64_t *in) {
    if (out != in) {
        memcpy(out, in, FAFFT_N_WORDS * sizeof(uint64_t));
    }

    fafft_basis_cvt_inplace(out);
}

void fafft_inverse_basis_cvt(uint64_t *out, const uint64_t *in) {
    if (out != in) {
        memcpy(out, in, FAFFT_N_WORDS * sizeof(uint64_t));
    }

    fafft_inverse_basis_cvt_inplace(out);
}

static void bc_1_256( uint32_t *poly , unsigned n_256bit )
{
  for(int i=n_256bit;i>0;i--) {
    uint32_t p0 = poly[0];
    uint32_t p1 = poly[1];
    uint32_t p2 = poly[2];
    uint32_t p3 = poly[3];
    uint32_t p4 = poly[4];
    uint32_t p5 = poly[5];
    uint32_t p6 = poly[6];
    uint32_t p7 = poly[7];

//layer 13: [256] s4^8:(128,8) suggest unit:1
    p4 ^= p7>>24;
    p3 ^= (p7<<8)^(p6>>24);
    p2 ^= (p6<<8)^(p5>>24);
    p1 ^= (p5<<8)^(p4>>24);
    p0 ^= (p4<<8);
//layer 14: [128] s4^4:(64,4) suggest unit:1
    p2 ^= p3>>28;
    p1 ^= (p3<<4)^(p2>>28);
    p0 ^= (p2<<4);

    p6 ^= p7>>28;
    p5 ^= (p7<<4)^(p6>>28);
    p4 ^= (p6<<4);
//layer 15: [64] s4^2:(32,2) suggest unit:1
    p1 ^= p1>>30;
    p3 ^= p3>>30;
    p5 ^= p5>>30;
    p7 ^= p7>>30;
    p0 ^= p1<<2;
    p2 ^= p3<<2;
    p4 ^= p5<<2;
    p6 ^= p7<<2;

//layer 16: [32] s4^1:(16,1) suggest unit:1
    p0 ^= ((p0&0x80000000)>>15);
    p1 ^= ((p1&0x80000000)>>15);
    p2 ^= ((p2&0x80000000)>>15);
    p3 ^= ((p3&0x80000000)>>15);
    p4 ^= ((p4&0x80000000)>>15);
    p5 ^= ((p5&0x80000000)>>15);
    p6 ^= ((p6&0x80000000)>>15);
    p7 ^= ((p7&0x80000000)>>15);
    p0 ^= ((p0&0x7fff0000)>>15);
    p1 ^= ((p1&0x7fff0000)>>15);
    p2 ^= ((p2&0x7fff0000)>>15);
    p3 ^= ((p3&0x7fff0000)>>15);
    p4 ^= ((p4&0x7fff0000)>>15);
    p5 ^= ((p5&0x7fff0000)>>15);
    p6 ^= ((p6&0x7fff0000)>>15);
    p7 ^= ((p7&0x7fff0000)>>15);

//layer 17: [256] s2^2:(128,32) suggest unit:16
    p4 ^= p7;
    p3 ^= p6;
    p2 ^= p5;
    p1 ^= p4;
//layer 18: [128] s2^1:(64,16) suggest unit:16
    p2 ^= p3>>16;
    p1 ^= (p3<<16)^(p2>>16);
    p0 ^= (p2<<16);
    p6 ^= p7>>16;
    p5 ^= (p7<<16)^(p6>>16);
    p4 ^= (p6<<16);
//layer 19: [256] s1^1:(128,64) suggest unit:64
    p5 ^= p7;
    p4 ^= p6;
    p3 ^= p5;
    p2 ^= p4;
//layer 20: [64] s1^1:(32,16) suggest unit:16
    p1 ^= p1>>16;
    p3 ^= p3>>16;
    p5 ^= p5>>16;
    p7 ^= p7>>16;
    p0 ^= p1<<16;
    p2 ^= p3<<16;
    p4 ^= p5<<16;
    p6 ^= p7<<16;
//layer 21: [16] s2^2:(8,2) suggest unit:1
    p0 ^= (p0&0xc000c000)>>6;
    p1 ^= (p1&0xc000c000)>>6;
    p2 ^= (p2&0xc000c000)>>6;
    p3 ^= (p3&0xc000c000)>>6;
    p4 ^= (p4&0xc000c000)>>6;
    p5 ^= (p5&0xc000c000)>>6;
    p6 ^= (p6&0xc000c000)>>6;
    p7 ^= (p7&0xc000c000)>>6;
    p0 ^= (p0&0x3f003f00)>>6;
    p1 ^= (p1&0x3f003f00)>>6;
    p2 ^= (p2&0x3f003f00)>>6;
    p3 ^= (p3&0x3f003f00)>>6;
    p4 ^= (p4&0x3f003f00)>>6;
    p5 ^= (p5&0x3f003f00)>>6;
    p6 ^= (p6&0x3f003f00)>>6;
    p7 ^= (p7&0x3f003f00)>>6;
//layer 22: [8] s2^1:(4,1) suggest unit:1
    p0 ^= ((p0&0x80808080)>>3);
    p1 ^= ((p1&0x80808080)>>3);
    p2 ^= ((p2&0x80808080)>>3);
    p3 ^= ((p3&0x80808080)>>3);
    p4 ^= ((p4&0x80808080)>>3);
    p5 ^= ((p5&0x80808080)>>3);
    p6 ^= ((p6&0x80808080)>>3);
    p7 ^= ((p7&0x80808080)>>3);
    p0 ^= ((p0&0x70707070)>>3);
    p1 ^= ((p1&0x70707070)>>3);
    p2 ^= ((p2&0x70707070)>>3);
    p3 ^= ((p3&0x70707070)>>3);
    p4 ^= ((p4&0x70707070)>>3);
    p5 ^= ((p5&0x70707070)>>3);
    p6 ^= ((p6&0x70707070)>>3);
    p7 ^= ((p7&0x70707070)>>3);

//layer 23: [16] s1^1:(8,4) suggest unit:4
    p0 ^= ((p0&0xf000f000)>>4);
    p1 ^= ((p1&0xf000f000)>>4);
    p2 ^= ((p2&0xf000f000)>>4);
    p3 ^= ((p3&0xf000f000)>>4);
    p4 ^= ((p4&0xf000f000)>>4);
    p5 ^= ((p5&0xf000f000)>>4);
    p6 ^= ((p6&0xf000f000)>>4);
    p7 ^= ((p7&0xf000f000)>>4);
    p0 ^= ((p0&0x0f000f00)>>4);
    p1 ^= ((p1&0x0f000f00)>>4);
    p2 ^= ((p2&0x0f000f00)>>4);
    p3 ^= ((p3&0x0f000f00)>>4);
    p4 ^= ((p4&0x0f000f00)>>4);
    p5 ^= ((p5&0x0f000f00)>>4);
    p6 ^= ((p6&0x0f000f00)>>4);
    p7 ^= ((p7&0x0f000f00)>>4);

//layer 24: [4] s1^1:(2,1) suggest unit:1
    p0 ^= ((p0&0x88888888)>>1);
    p1 ^= ((p1&0x88888888)>>1);
    p2 ^= ((p2&0x88888888)>>1);
    p3 ^= ((p3&0x88888888)>>1);
    p4 ^= ((p4&0x88888888)>>1);
    p5 ^= ((p5&0x88888888)>>1);
    p6 ^= ((p6&0x88888888)>>1);
    p7 ^= ((p7&0x88888888)>>1);
    p0 ^= ((p0&0x44444444)>>1);
    p1 ^= ((p1&0x44444444)>>1);
    p2 ^= ((p2&0x44444444)>>1);
    p3 ^= ((p3&0x44444444)>>1);
    p4 ^= ((p4&0x44444444)>>1);
    p5 ^= ((p5&0x44444444)>>1);
    p6 ^= ((p6&0x44444444)>>1);
    p7 ^= ((p7&0x44444444)>>1);

    poly[0] = p0;
    poly[1] = p1;
    poly[2] = p2;
    poly[3] = p3;
    poly[4] = p4;
    poly[5] = p5;
    poly[6] = p6;
    poly[7] = p7;
    poly += 8;
  }
}

static void ibc_1_256( uint32_t *poly , unsigned n_256bit )
{

  for(int i=n_256bit;i>0;i--) {
    uint32_t p0 = poly[0];
    uint32_t p1 = poly[1];
    uint32_t p2 = poly[2];
    uint32_t p3 = poly[3];
    uint32_t p4 = poly[4];
    uint32_t p5 = poly[5];
    uint32_t p6 = poly[6];
    uint32_t p7 = poly[7];

//layer 7: [4] s1^1:(2,1) suggest unit:1
    p0 ^= ((p0&0xcccccccc)>>1);
    p1 ^= ((p1&0xcccccccc)>>1);
    p2 ^= ((p2&0xcccccccc)>>1);
    p3 ^= ((p3&0xcccccccc)>>1);
    p4 ^= ((p4&0xcccccccc)>>1);
    p5 ^= ((p5&0xcccccccc)>>1);
    p6 ^= ((p6&0xcccccccc)>>1);
    p7 ^= ((p7&0xcccccccc)>>1);

//layer 6: [16] s1^1:(8,4) suggest unit:4
    p0 ^= ((p0&0xff00ff00)>>4);
    p1 ^= ((p1&0xff00ff00)>>4);
    p2 ^= ((p2&0xff00ff00)>>4);
    p3 ^= ((p3&0xff00ff00)>>4);
    p4 ^= ((p4&0xff00ff00)>>4);
    p5 ^= ((p5&0xff00ff00)>>4);
    p6 ^= ((p6&0xff00ff00)>>4);
    p7 ^= ((p7&0xff00ff00)>>4);

//layer 5: [8] s2^1:(4,1) suggest unit:1
    p0 ^= ((p0&0xf0f0f0f0)>>3);
    p1 ^= ((p1&0xf0f0f0f0)>>3);
    p2 ^= ((p2&0xf0f0f0f0)>>3);
    p3 ^= ((p3&0xf0f0f0f0)>>3);
    p4 ^= ((p4&0xf0f0f0f0)>>3);
    p5 ^= ((p5&0xf0f0f0f0)>>3);
    p6 ^= ((p6&0xf0f0f0f0)>>3);
    p7 ^= ((p7&0xf0f0f0f0)>>3);

//layer 4: [16] s2^2:(8,2) suggest unit:1
    p0 ^= ((p0&0xff00ff00)>>6);
    p1 ^= ((p1&0xff00ff00)>>6);
    p2 ^= ((p2&0xff00ff00)>>6);
    p3 ^= ((p3&0xff00ff00)>>6);
    p4 ^= ((p4&0xff00ff00)>>6);
    p5 ^= ((p5&0xff00ff00)>>6);
    p6 ^= ((p6&0xff00ff00)>>6);
    p7 ^= ((p7&0xff00ff00)>>6);

//layer 3: [64] s1^1:(32,16) suggest unit:16
    p0 ^= p1<<16;
    p2 ^= p3<<16;
    p4 ^= p5<<16;
    p6 ^= p7<<16;

    p1 ^= p1>>16;
    p3 ^= p3>>16;
    p5 ^= p5>>16;
    p7 ^= p7>>16;

//layer 2: [256] s1^1:(128,64) suggest unit:64
    p3 ^= p5;
    p2 ^= p4;

    p5 ^= p7;
    p4 ^= p6;

//layer 1: [128] s2^1:(64,16) suggest unit:16
    p0 ^= (p2<<16);
    p1 ^= (p3<<16)^(p2>>16);
    p2 ^= p3>>16;

    p4 ^= (p6<<16);
    p5 ^= (p7<<16)^(p6>>16);
    p6 ^= p7>>16;

//layer 0: [256] s2^2:(128,32) suggest unit:16
    p1 ^= p4;
    p2 ^= p5;
    p3 ^= p6;
    p4 ^= p7;

//layer 16: [32] s4^1:(16,1) suggest unit:1
    p0 ^= ((p0&0xffff0000)>>15);
    p1 ^= ((p1&0xffff0000)>>15);
    p2 ^= ((p2&0xffff0000)>>15);
    p3 ^= ((p3&0xffff0000)>>15);
    p4 ^= ((p4&0xffff0000)>>15);
    p5 ^= ((p5&0xffff0000)>>15);
    p6 ^= ((p6&0xffff0000)>>15);
    p7 ^= ((p7&0xffff0000)>>15);

//layer 15: [64] s4^2:(32,2) suggest unit:1
    p0 ^= p1<<2;
    p2 ^= p3<<2;
    p4 ^= p5<<2;
    p6 ^= p7<<2;

    p1 ^= p1>>30;
    p3 ^= p3>>30;
    p5 ^= p5>>30;
    p7 ^= p7>>30;

//layer 14: [128] s4^4:(64,4) suggest unit:1
    p0 ^= (p2<<4);
    p1 ^= (p3<<4)^(p2>>28);
    p2 ^= p3>>28;

    p4 ^= (p6<<4);
    p5 ^= (p7<<4)^(p6>>28);
    p6 ^= p7>>28;

//layer 13: [256] s4^8:(128,8) suggest unit:1
    p0 ^= (p4<<8);
    p1 ^= (p5<<8)^(p4>>24);
    p2 ^= (p6<<8)^(p5>>24);
    p3 ^= (p7<<8)^(p6>>24);
    p4 ^= p7>>24;


    poly[0] = p0;
    poly[1] = p1;
    poly[2] = p2;
    poly[3] = p3;
    poly[4] = p4;
    poly[5] = p5;
    poly[6] = p6;
    poly[7] = p7;

    poly += 8;
  }
}

static inline void rvv_xor_layer_down_bits(uint32_t *ptr, size_t deg_h_bits, size_t deg_l_bits) {
  /*
    * Replaces this:
    *
    * for (int i = (deg_h / 32) - 4; i >= 0; i -= 4) {
    *     ptr[i + deg_l / 32 + 0] ^= ptr[i + deg_h / 32 + 0];
    *     ptr[i + deg_l / 32 + 1] ^= ptr[i + deg_h / 32 + 1];
    *     ptr[i + deg_l / 32 + 2] ^= ptr[i + deg_h / 32 + 2];
    *     ptr[i + deg_l / 32 + 3] ^= ptr[i + deg_h / 32 + 3];
    * }
    *
    * Since dst starts below src and ranges may overlap, the forward BasisCvt generated code processes from high addresses to low addresses.
    */
  const size_t dst_off = deg_l_bits / 8;
  const size_t src_off = deg_h_bits / 8;
  const size_t len     = deg_h_bits / 8;
  const size_t diff    = src_off - dst_off;

  fafft_rvv_xor_down_u8((uint8_t *)ptr, dst_off + len, len, diff);
}

static inline void rvv_xor_layer_up_bits(uint32_t *ptr, size_t deg_h_bits, size_t deg_l_bits) {
  // Used for inverse layers where processing from low to high
  const size_t dst_off = deg_l_bits / 8;
  const size_t src_off = deg_h_bits / 8;
  const size_t len     = deg_h_bits / 8;
  const size_t diff    = src_off - dst_off;

  fafft_rvv_xor_up_u8((uint8_t *)ptr, dst_off, len, diff);
}

static inline void rvv_xor_layer_blocks_down(uint32_t *poly, size_t total_bits, size_t deg_h_bits, size_t deg_l_bits) {
  uint32_t *ptr = poly;
  const size_t nblocks = total_bits / (deg_h_bits * 2);
  const size_t block_words = (deg_h_bits * 2) / 32;

  for (size_t j = 0; j < nblocks; j++) {
      rvv_xor_layer_down_bits(ptr, deg_h_bits, deg_l_bits);
      ptr += block_words;
  }
}

static inline void rvv_xor_layer_blocks_up(uint32_t *poly, size_t total_bits, size_t deg_h_bits, size_t deg_l_bits)
{
  uint32_t *ptr = poly;
  const size_t nblocks = total_bits / (deg_h_bits * 2);
  const size_t block_words = (deg_h_bits * 2) / 32;

  for (size_t j = 0; j < nblocks; j++) {
      rvv_xor_layer_up_bits(ptr, deg_h_bits, deg_l_bits);
      ptr += block_words;
  }
}

static void repr_s8_1_65536( uint32_t *poly )
{
  const unsigned w = 65536;
  uint32_t *ptr;

  // layer 0: [65536] s8^128:(32768,128)
    rvv_xor_layer_blocks_down(poly, w, 32768, 128);

  // layer 0: [32768] s8^64:(16384,64)
    rvv_xor_layer_blocks_down(poly, w, 16384, 64);

  // layer 0: [16384] s8^32:(8192,32)
    rvv_xor_layer_blocks_down(poly, w, 8192, 32);

//layer 1: [8192] s8^16:(4096,16) suggest unit:1
  ptr = poly;
  for(int j=(w/8192);j>0;j--) {
    uint32_t leftover = 0;
    for(int i=(4096/32)-4;i>=0;i-=4) {
      uint32_t p3 = ptr[i+(4096/32)+3];
      uint32_t p2 = ptr[i+(4096/32)+2];
      uint32_t p1 = ptr[i+(4096/32)+1];
      uint32_t p0 = ptr[i+(4096/32)];
      uint32_t q3 = ptr[i+1+3];
      uint32_t q2 = ptr[i+1+2];
      uint32_t q1 = ptr[i+1+1];
      uint32_t q0 = ptr[i+1];

      ptr[i+1+3] = q3^(p3>>16)^(leftover<<16);
      ptr[i+1+2] = q2^(p2>>16)^(p3<<16);
      ptr[i+1+1] = q1^(p1>>16)^(p2<<16);
      ptr[i+1] = q0^(p0>>16)^(p1<<16);
      leftover= p0;
    }
    ptr[0] ^= (leftover<<16);
    ptr += (8192/32);
  }

//layer 2: [4096] s8^8:(2048,8) suggest unit:1
  ptr = poly;
  for(int j=(w/4096);j>0;j--) {
    uint32_t leftover = 0;
    for(int i=(2048/32)-4;i>=0;i-=4) {
      uint32_t p3 = ptr[i+(2048/32)+3];
      uint32_t p2 = ptr[i+(2048/32)+2];
      uint32_t p1 = ptr[i+(2048/32)+1];
      uint32_t p0 = ptr[i+(2048/32)];
      uint32_t q3 = ptr[i+1+3];
      uint32_t q2 = ptr[i+1+2];
      uint32_t q1 = ptr[i+1+1];
      uint32_t q0 = ptr[i+1];

      ptr[i+1+3] = q3^(p3>>24)^(leftover<<8);
      ptr[i+1+2] = q2^(p2>>24)^(p3<<8);
      ptr[i+1+1] = q1^(p1>>24)^(p2<<8);
      ptr[i+1] = q0^(p0>>24)^(p1<<8);
      leftover= p0;
    }
    ptr[0] ^= (leftover<<8);
    ptr += (4096/32);
  }

//layer 3: [2048] s8^4:(1024,4) suggest unit:1
  ptr = poly;
  for(int j=(w/2048);j>0;j--) {
    uint32_t leftover = 0;
    for(int i=(1024/32)-4;i>=0;i-=4) {
      uint32_t p3 = ptr[i+(1024/32)+3];
      uint32_t p2 = ptr[i+(1024/32)+2];
      uint32_t p1 = ptr[i+(1024/32)+1];
      uint32_t p0 = ptr[i+(1024/32)];
      uint32_t q3 = ptr[i+1+3];
      uint32_t q2 = ptr[i+1+2];
      uint32_t q1 = ptr[i+1+1];
      uint32_t q0 = ptr[i+1];

      ptr[i+1+3] = q3^(p3>>28)^(leftover<<4);
      ptr[i+1+2] = q2^(p2>>28)^(p3<<4);
      ptr[i+1+1] = q1^(p1>>28)^(p2<<4);
      ptr[i+1] = q0^(p0>>28)^(p1<<4);
      leftover= p0;
    }
    ptr[0] ^= (leftover<<4);
    ptr += (2048/32);
  }

//layer 4: [1024] s8^2:(512,2) suggest unit:1
  ptr = poly;
  for(int j=(w/1024);j>0;j--) {
    uint32_t leftover = 0;
    for(int i=(512/32)-4;i>=0;i-=4) {
      uint32_t p3 = ptr[i+(512/32)+3];
      uint32_t p2 = ptr[i+(512/32)+2];
      uint32_t p1 = ptr[i+(512/32)+1];
      uint32_t p0 = ptr[i+(512/32)];
      uint32_t q3 = ptr[i+1+3];
      uint32_t q2 = ptr[i+1+2];
      uint32_t q1 = ptr[i+1+1];
      uint32_t q0 = ptr[i+1];

      ptr[i+1+3] = q3^(p3>>30)^(leftover<<2);
      ptr[i+1+2] = q2^(p2>>30)^(p3<<2);
      ptr[i+1+1] = q1^(p1>>30)^(p2<<2);
      ptr[i+1] = q0^(p0>>30)^(p1<<2);
      leftover= p0;
    }
    ptr[0] ^= (leftover<<2);
    ptr += (1024/32);
  }

//layer 5: [512] s8^1:(256,1) suggest unit:1
  ptr = poly;
  for(int j=(w/512);j>0;j--) {
    uint32_t leftover = 0;
    for(int i=(256/32)-4;i>=0;i-=4) {
      uint32_t p3 = ptr[i+(256/32)+3];
      uint32_t p2 = ptr[i+(256/32)+2];
      uint32_t p1 = ptr[i+(256/32)+1];
      uint32_t p0 = ptr[i+(256/32)];
      uint32_t q3 = ptr[i+1+3];
      uint32_t q2 = ptr[i+1+2];
      uint32_t q1 = ptr[i+1+1];
      uint32_t q0 = ptr[i+1];

      ptr[i+1+3] = q3^(p3>>31)^(leftover<<1);
      ptr[i+1+2] = q2^(p2>>31)^(p3<<1);
      ptr[i+1+1] = q1^(p1>>31)^(p2<<1);
      ptr[i+1] = q0^(p0>>31)^(p1<<1);
      leftover= p0;
    }
    ptr[0] ^= (leftover<<1);
    ptr += (512/32);
  }
}



static void irepr_s8_1_65536( uint32_t *poly )
{
  const unsigned w = 65536;
  uint32_t *ptr;

//layer 5: [512] s8^1:(256,1) suggest unit:1
  ptr = poly;
  for(int j=(w/512);j>0;j--) {
    uint32_t leftover = 0;
    for(int i=0;i<(256/32);i+=4) {
      uint32_t p3 = ptr[i+(256/32)+3];
      uint32_t p2 = ptr[i+(256/32)+2];
      uint32_t p1 = ptr[i+(256/32)+1];
      uint32_t p0 = ptr[i+(256/32)];
      uint32_t q3 = ptr[i+3];
      uint32_t q2 = ptr[i+2];
      uint32_t q1 = ptr[i+1];
      uint32_t q0 = ptr[i];

      ptr[i+3] = q3^(p2>>31)^(p3<<1);
      ptr[i+2] = q2^(p1>>31)^(p2<<1);
      ptr[i+1] = q1^(p0>>31)^(p1<<1);
      ptr[i] = q0^(leftover>>31)^(p0<<1);
      leftover= p3;
    }
    ptr[(256/32)] ^= (leftover>>31);
    ptr += (512/32);
  }

//layer 4: [1024] s8^2:(512,2) suggest unit:1
  ptr = poly;
  for(int j=(w/1024);j>0;j--) {
    uint32_t leftover = 0;
    for(int i=0;i<(512/32);i+=4) {
      uint32_t p3 = ptr[i+(512/32)+3];
      uint32_t p2 = ptr[i+(512/32)+2];
      uint32_t p1 = ptr[i+(512/32)+1];
      uint32_t p0 = ptr[i+(512/32)];
      uint32_t q3 = ptr[i+3];
      uint32_t q2 = ptr[i+2];
      uint32_t q1 = ptr[i+1];
      uint32_t q0 = ptr[i];

      ptr[i+3] = q3^(p2>>30)^(p3<<2);
      ptr[i+2] = q2^(p1>>30)^(p2<<2);
      ptr[i+1] = q1^(p0>>30)^(p1<<2);
      ptr[i] = q0^(leftover>>30)^(p0<<2);
      leftover= p3;
    }
    ptr[(512/32)] ^= (leftover>>30);
    ptr += (1024/32);
  }

//layer 3: [2048] s8^4:(1024,4) suggest unit:1
  ptr = poly;
  for(int j=(w/2048);j>0;j--) {
    uint32_t leftover = 0;
    for(int i=0;i<(1024/32);i+=4) {
      uint32_t p3 = ptr[i+(1024/32)+3];
      uint32_t p2 = ptr[i+(1024/32)+2];
      uint32_t p1 = ptr[i+(1024/32)+1];
      uint32_t p0 = ptr[i+(1024/32)];
      uint32_t q3 = ptr[i+3];
      uint32_t q2 = ptr[i+2];
      uint32_t q1 = ptr[i+1];
      uint32_t q0 = ptr[i];

      ptr[i+3] = q3^(p2>>28)^(p3<<4);
      ptr[i+2] = q2^(p1>>28)^(p2<<4);
      ptr[i+1] = q1^(p0>>28)^(p1<<4);
      ptr[i] = q0^(leftover>>28)^(p0<<4);
      leftover= p3;
    }
    ptr[(1024/32)] ^= (leftover>>28);
    ptr += (2048/32);
  }

//layer 2: [4096] s8^8:(2048,8) suggest unit:1
  ptr = poly;
  for(int j=(w/4096);j>0;j--) {
    uint32_t leftover = 0;
    for(int i=0;i<(2048/32);i+=4) {
      uint32_t p3 = ptr[i+(2048/32)+3];
      uint32_t p2 = ptr[i+(2048/32)+2];
      uint32_t p1 = ptr[i+(2048/32)+1];
      uint32_t p0 = ptr[i+(2048/32)];
      uint32_t q3 = ptr[i+3];
      uint32_t q2 = ptr[i+2];
      uint32_t q1 = ptr[i+1];
      uint32_t q0 = ptr[i];

      ptr[i+3] = q3^(p2>>24)^(p3<<8);
      ptr[i+2] = q2^(p1>>24)^(p2<<8);
      ptr[i+1] = q1^(p0>>24)^(p1<<8);
      ptr[i] = q0^(leftover>>24)^(p0<<8);
      leftover= p3;
    }
    ptr[(2048/32)] ^= (leftover>>24);
    ptr += (4096/32);
  }

//layer 1: [8192] s8^16:(4096,16) suggest unit:1
  ptr = poly;
  for(int j=(w/8192);j>0;j--) {
    uint32_t leftover = 0;
    for(int i=0;i<(4096/32);i+=4) {
      uint32_t p3 = ptr[i+(4096/32)+3];
      uint32_t p2 = ptr[i+(4096/32)+2];
      uint32_t p1 = ptr[i+(4096/32)+1];
      uint32_t p0 = ptr[i+(4096/32)];
      uint32_t q3 = ptr[i+3];
      uint32_t q2 = ptr[i+2];
      uint32_t q1 = ptr[i+1];
      uint32_t q0 = ptr[i];

      ptr[i+3] = q3^(p2>>16)^(p3<<16);
      ptr[i+2] = q2^(p1>>16)^(p2<<16);
      ptr[i+1] = q1^(p0>>16)^(p1<<16);
      ptr[i] = q0^(leftover>>16)^(p0<<16);
      leftover= p3;
    }
    ptr[(4096/32)] ^= (leftover>>16);
    ptr += (8192/32);
  }

  // layer 0: [16384] s8^32:(8192,32)
  rvv_xor_layer_blocks_up(poly, w, 8192, 32);

  // layer 0: [32768] s8^64:(16384,64)
  rvv_xor_layer_blocks_up(poly, w, 16384, 64);

  // layer 0: [65536] s8^128:(32768,128)
  rvv_xor_layer_blocks_up(poly, w, 32768, 128);

}


static void cvt_to_fft_256_65536( uint32_t *poly )
{
  const unsigned w = 65536;
  // layer 8: [65536] s4^8:(32768,2048)
  rvv_xor_layer_blocks_down(poly, w, 32768, 2048);

  // layer 9: [32768] s4^4:(16384,1024)
  rvv_xor_layer_blocks_down(poly, w, 16384, 1024);

  // layer 10: [16384] s4^2:(8192,512)
  rvv_xor_layer_blocks_down(poly, w, 8192, 512);

  // layer 11: [8192] s4^1:(4096,256)
  rvv_xor_layer_blocks_down(poly, w, 4096, 256);

  // layer 12: [65536] s2^2:(32768,8192)
  rvv_xor_layer_blocks_down(poly, w, 32768, 8192);

  // layer 13: [32768] s2^1:(16384,4096)
  rvv_xor_layer_blocks_down(poly, w, 16384, 4096);

  // layer 14: [65536] s1^1:(32768,16384)
  rvv_xor_layer_blocks_down(poly, w, 32768, 16384);

  // layer 15: [16384] s1^1:(8192,4096)
  rvv_xor_layer_blocks_down(poly, w, 8192, 4096);

  // layer 16: [4096] s2^2:(2048,512)
  rvv_xor_layer_blocks_down(poly, w, 2048, 512);

  // layer 17: [2048] s2^1:(1024,256)
  rvv_xor_layer_blocks_down(poly, w, 1024, 256);

  // layer 18: [4096] s1^1:(2048,1024)
  rvv_xor_layer_blocks_down(poly, w, 2048, 1024);

  // layer 19: [1024] s1^1:(512,256)
  rvv_xor_layer_blocks_down(poly, w, 512, 256);
}


static void icvt_to_poly_256_65536( uint32_t *poly )
{
  const unsigned w = 65536;

  // layer 19: [1024] s1^1:(512,256)
  rvv_xor_layer_blocks_up(poly, w, 512, 256);

  // layer 18: [4096] s1^1:(2048,1024)
  rvv_xor_layer_blocks_up(poly, w, 2048, 1024);

  // layer 17: [2048] s2^1:(1024,256)
  rvv_xor_layer_blocks_up(poly, w, 1024, 256);

  // layer 16: [4096] s2^2:(2048,512)
  rvv_xor_layer_blocks_up(poly, w, 2048, 512);

  // layer 15: [16384] s1^1:(8192,4096)
  rvv_xor_layer_blocks_up(poly, w, 8192, 4096);

  // layer 14: [65536] s1^1:(32768,16384)
  rvv_xor_layer_blocks_up(poly, w, 32768, 16384);

  // layer 13: [32768] s2^1:(16384,4096)
  rvv_xor_layer_blocks_up(poly, w, 16384, 4096);

  // layer 12: [65536] s2^2:(32768,8192)
  rvv_xor_layer_blocks_up(poly, w, 32768, 8192);

  // layer 11: [8192] s4^1:(4096,256)
  rvv_xor_layer_blocks_up(poly, w, 4096, 256);

  // layer 10: [16384] s4^2:(8192,512)
  rvv_xor_layer_blocks_up(poly, w, 8192, 512);

  // layer 9: [32768] s4^4:(16384,1024)
  rvv_xor_layer_blocks_up(poly, w, 16384, 1024);

  // layer 8: [65536] s4^8:(32768,2048)
  rvv_xor_layer_blocks_up(poly, w, 32768, 2048);
}