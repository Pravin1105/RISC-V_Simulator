#include "test_framework.hpp"
#include "immediate.hpp"

TEST_CASE(Immediate_IType) {
    // ADDI x1, x0, 1 -> imm = 1 (bits 31:20 = 0x001)
    riscv::Word inst_pos = 0x00100093;
    ASSERT_EQ(riscv::ImmediateGenerator::extract_i(inst_pos), 1);

    // ADDI x1, x0, -1 -> imm = -1 (bits 31:20 = 0xFFF)
    riscv::Word inst_neg = 0xFFF00093;
    ASSERT_EQ(riscv::ImmediateGenerator::extract_i(inst_neg), -1);

    // ADDI x1, x0, -2048 (min 12-bit signed: 0x800)
    riscv::Word inst_min = 0x80000093;
    ASSERT_EQ(riscv::ImmediateGenerator::extract_i(inst_min), -2048);

    // ADDI x1, x0, 2047 (max 12-bit signed: 0x7FF)
    riscv::Word inst_max = 0x7FF00093;
    ASSERT_EQ(riscv::ImmediateGenerator::extract_i(inst_max), 2047);
}

TEST_CASE(Immediate_SType) {
    // SW x2, 4(x1) -> imm = 4
    // imm[11:5] = 0, imm[4:0] = 4 (0b00100 << 7 = 0x200)
    riscv::Word inst_pos = 0x0020a223; // SW x2, 4(x1)
    ASSERT_EQ(riscv::ImmediateGenerator::extract_s(inst_pos), 4);

    // SW x2, -4(x1) -> imm = -4 (12-bit: 0xFFC)
    // imm[11:5] = 0b1111111 = 0x7F, imm[4:0] = 0b11100 = 0x1C
    riscv::Word inst_neg = (0x7Fu << 25) | (2u << 20) | (1u << 15) | (0x2u << 12) | (0x1Cu << 7) | 0x23u;
    ASSERT_EQ(riscv::ImmediateGenerator::extract_s(inst_neg), -4);
}

TEST_CASE(Immediate_BType) {
    // BEQ x1, x2, 16 -> imm = +16 (0x10)
    // imm[12]=0, imm[11]=0, imm[10:5]=0, imm[4:1]=0b1000 (8), imm[0]=0
    riscv::Word inst_pos = (0u << 31) | (0u << 25) | (2u << 20) | (1u << 15) | (0u << 12) | (8u << 8) | (0u << 7) | 0x63u;
    ASSERT_EQ(riscv::ImmediateGenerator::extract_b(inst_pos), 16);

    // BEQ x1, x2, -16 -> imm = -16 (13-bit: 0x1FF0)
    // imm[12]=1, imm[11]=1, imm[10:5]=0b111111, imm[4:1]=0b1000 (8), imm[0]=0
    riscv::Word inst_neg = (1u << 31) | (0x3Fu << 25) | (2u << 20) | (1u << 15) | (0u << 12) | (8u << 8) | (1u << 7) | 0x63u;
    ASSERT_EQ(riscv::ImmediateGenerator::extract_b(inst_neg), -16);
}

TEST_CASE(Immediate_UType) {
    // LUI x1, 0x12345 -> imm = 0x12345000
    riscv::Word inst = 0x123450b7;
    ASSERT_EQ(riscv::ImmediateGenerator::extract_u(inst), 0x12345000u);
}

TEST_CASE(Immediate_JType) {
    // JAL x1, +8 -> imm = 8
    // imm[20]=0, imm[19:12]=0, imm[11]=0, imm[10:1]=4, imm[0]=0
    riscv::Word inst_pos = (0u << 31) | (4u << 21) | (0u << 20) | (0u << 12) | (1u << 7) | 0x6Fu;
    ASSERT_EQ(riscv::ImmediateGenerator::extract_j(inst_pos), 8);

    // JAL x1, -8 -> imm = -8
    // -8 in 21-bit signed = 0x1FFFF8
    // imm[20]=1, imm[19:12]=0xFF, imm[11]=1, imm[10:1]=0x3FC (1020), imm[0]=0
    riscv::Word inst_neg = (1u << 31) | (0x3FCu << 21) | (1u << 20) | (0xFFu << 12) | (1u << 7) | 0x6Fu;
    ASSERT_EQ(riscv::ImmediateGenerator::extract_j(inst_neg), -8);
}

TEST_CASE(Immediate_Shamt) {
    // SLLI x1, x2, 5 -> shamt = 5
    riscv::Word inst = (0x00u << 25) | (5u << 20) | (2u << 15) | (1u << 12) | (1u << 7) | 0x13u;
    ASSERT_EQ(riscv::ImmediateGenerator::extract_shamt(inst), 5u);
}
