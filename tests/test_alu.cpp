#include "test_framework.hpp"
#include "alu.hpp"

TEST_CASE(Alu_ArithmeticWrapping) {
    // ADD wrapping
    ASSERT_EQ(riscv::Alu::add(0xFFFFFFFFu, 1), 0u);
    ASSERT_EQ(riscv::Alu::add(0x7FFFFFFF, 1), 0x80000000u);

    // SUB wrapping
    ASSERT_EQ(riscv::Alu::sub(0, 1), 0xFFFFFFFFu);
    ASSERT_EQ(riscv::Alu::sub(10, 5), 5u);
}

TEST_CASE(Alu_Bitwise) {
    ASSERT_EQ(riscv::Alu::bit_and(0xFF00FF00, 0x0F0F0F0F), 0x0F000F00u);
    ASSERT_EQ(riscv::Alu::bit_or(0xFF00FF00, 0x00FF00FF), 0xFFFFFFFFu);
    ASSERT_EQ(riscv::Alu::bit_xor(0xAAAAAAAA, 0x55555555), 0xFFFFFFFFu);
    ASSERT_EQ(riscv::Alu::bit_xor(0xAAAAAAAA, 0xAAAAAAAA), 0u);
}

TEST_CASE(Alu_Shifts) {
    // SLL
    ASSERT_EQ(riscv::Alu::sll(1, 0), 1u);
    ASSERT_EQ(riscv::Alu::sll(1, 4), 16u);
    ASSERT_EQ(riscv::Alu::sll(1, 31), 0x80000000u);
    ASSERT_EQ(riscv::Alu::sll(1, 32), 1u); // shamt & 0x1F

    // SRL
    ASSERT_EQ(riscv::Alu::srl(0x80000000u, 1), 0x40000000u);
    ASSERT_EQ(riscv::Alu::srl(0x80000000u, 31), 1u);

    // SRA (sign preservation)
    ASSERT_EQ(riscv::Alu::sra(0x80000000u, 1), 0xC0000000u);
    ASSERT_EQ(riscv::Alu::sra(0x80000000u, 31), 0xFFFFFFFFu);
    ASSERT_EQ(riscv::Alu::sra(0x40000000u, 1), 0x20000000u);
}

TEST_CASE(Alu_Comparisons) {
    riscv::Word pos = 5;
    riscv::Word neg = static_cast<riscv::Word>(-5);

    // SLT: signed comparison (-5 < 5 is true)
    ASSERT_EQ(riscv::Alu::slt(neg, pos), 1u);
    ASSERT_EQ(riscv::Alu::slt(pos, neg), 0u);
    ASSERT_EQ(riscv::Alu::slt(pos, pos), 0u);

    // SLTU: unsigned comparison (5 < 0xFFFFFFFB is true)
    ASSERT_EQ(riscv::Alu::sltu(pos, neg), 1u);
    ASSERT_EQ(riscv::Alu::sltu(neg, pos), 0u);
}

TEST_CASE(Alu_Branches) {
    riscv::Word a = 10;
    riscv::Word b = 20;
    riscv::Word neg1 = static_cast<riscv::Word>(-1);

    // BEQ / BNE
    ASSERT_TRUE(riscv::Alu::evaluate_branch(riscv::InstructionId::BEQ, a, a));
    ASSERT_FALSE(riscv::Alu::evaluate_branch(riscv::InstructionId::BEQ, a, b));
    ASSERT_TRUE(riscv::Alu::evaluate_branch(riscv::InstructionId::BNE, a, b));
    ASSERT_FALSE(riscv::Alu::evaluate_branch(riscv::InstructionId::BNE, a, a));

    // BLT / BGE (signed)
    ASSERT_TRUE(riscv::Alu::evaluate_branch(riscv::InstructionId::BLT, neg1, a));
    ASSERT_FALSE(riscv::Alu::evaluate_branch(riscv::InstructionId::BLT, a, neg1));
    ASSERT_TRUE(riscv::Alu::evaluate_branch(riscv::InstructionId::BGE, a, neg1));
    ASSERT_TRUE(riscv::Alu::evaluate_branch(riscv::InstructionId::BGE, a, a));

    // BLTU / BGEU (unsigned)
    ASSERT_TRUE(riscv::Alu::evaluate_branch(riscv::InstructionId::BLTU, a, neg1));
    ASSERT_FALSE(riscv::Alu::evaluate_branch(riscv::InstructionId::BLTU, neg1, a));
    ASSERT_TRUE(riscv::Alu::evaluate_branch(riscv::InstructionId::BGEU, neg1, a));
}
