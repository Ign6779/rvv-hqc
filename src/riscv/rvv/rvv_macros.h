#ifndef RVV_MACROS_H
#define RVV_MACROS_H

/* e8 */
#define VSETVLI_E8_M1(rd, rs1)    .insn i 0x57, 0x7, rd, rs1, 0
#define VSETVLI_E8_M2(rd, rs1)    .insn i 0x57, 0x7, rd, rs1, 1
#define VSETVLI_E8_M4(rd, rs1)    .insn i 0x57, 0x7, rd, rs1, 2
#define VSETVLI_E8_M8(rd, rs1)    .insn i 0x57, 0x7, rd, rs1, 3

/* e16 */
#define VSETVLI_E16_M1(rd, rs1)   .insn i 0x57, 0x7, rd, rs1, 4
#define VSETVLI_E16_M2(rd, rs1)   .insn i 0x57, 0x7, rd, rs1, 5
#define VSETVLI_E16_M4(rd, rs1)   .insn i 0x57, 0x7, rd, rs1, 6
#define VSETVLI_E16_M8(rd, rs1)   .insn i 0x57, 0x7, rd, rs1, 7

/* e32 */
#define VSETVLI_E32_M1(rd, rs1)   .insn i 0x57, 0x7, rd, rs1, 8
#define VSETVLI_E32_M2(rd, rs1)   .insn i 0x57, 0x7, rd, rs1, 9
#define VSETVLI_E32_M4(rd, rs1)   .insn i 0x57, 0x7, rd, rs1, 10
#define VSETVLI_E32_M8(rd, rs1)   .insn i 0x57, 0x7, rd, rs1, 11

/* e64 */
#define VSETVLI_E64_M1(rd, rs1)   .insn i 0x57, 0x7, rd, rs1, 12
#define VSETVLI_E64_M2(rd, rs1)   .insn i 0x57, 0x7, rd, rs1, 13
#define VSETVLI_E64_M4(rd, rs1)   .insn i 0x57, 0x7, rd, rs1, 14
#define VSETVLI_E64_M8(rd, rs1)   .insn i 0x57, 0x7, rd, rs1, 15

#endif