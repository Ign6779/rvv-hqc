#define VSETVLI_E8_M1(rd, rs1)   .insn i 0x57, 0x7, rd, rs1, 0
#define VSETVLI_E16_M1(rd, rs1)  .insn i 0x57, 0x7, rd, rs1, 4
#define VSETVLI_E32_M1(rd, rs1)  .insn i 0x57, 0x7, rd, rs1, 8
#define VSETVLI_E64_M1(rd, rs1)  .insn i 0x57, 0x7, rd, rs1, 12

    .text
    .globl rvv_reduce
    .type rvv_reduce, @function

rvv_reduce:
    ret