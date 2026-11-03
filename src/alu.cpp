#include "alu.hpp"

namespace riscv {

Word Alu::add(Word a, Word b) {
    return a + b; // Unsigned addition wraps modulo 2^32 in C++
}

Word Alu::sub(Word a, Word b) {
    return a - b; // Unsigned subtraction wraps modulo 2^32 in C++
}

Word Alu::bit_and(Word a, Word b) {
    return a & b;
}

Word Alu::bit_or(Word a, Word b) {
    return a | b;
}

Word Alu::bit_xor(Word a, Word b) {
    return a ^ b;
}

Word Alu::sll(Word a, uint32_t shamt) {
    shamt &= 0x1F;
    if (shamt == 0) return a;
    return a << shamt;
}

Word Alu::srl(Word a, uint32_t shamt) {
    shamt &= 0x1F;
    if (shamt == 0) return a;
    return a >> shamt;
}

Word Alu::sra(Word a, uint32_t shamt) {
    shamt &= 0x1F;
    if (shamt == 0) return a;
    Word shifted = a >> shamt;
    if ((a & 0x80000000u) != 0) {
        Word sign_mask = ~0u << (32 - shamt);
        shifted |= sign_mask;
    }
    return shifted;
}

Word Alu::slt(Word a, Word b) {
    SignedWord sa = static_cast<SignedWord>(a);
    SignedWord sb = static_cast<SignedWord>(b);
    return (sa < sb) ? 1 : 0;
}

Word Alu::sltu(Word a, Word b) {
    return (a < b) ? 1 : 0;
}

bool Alu::evaluate_branch(InstructionId branch_id, Word rs1_val, Word rs2_val) {
    switch (branch_id) {
        case InstructionId::BEQ:
            return rs1_val == rs2_val;
        case InstructionId::BNE:
            return rs1_val != rs2_val;
        case InstructionId::BLT:
            return static_cast<SignedWord>(rs1_val) < static_cast<SignedWord>(rs2_val);
        case InstructionId::BGE:
            return static_cast<SignedWord>(rs1_val) >= static_cast<SignedWord>(rs2_val);
        case InstructionId::BLTU:
            return rs1_val < rs2_val;
        case InstructionId::BGEU:
            return rs1_val >= rs2_val;
        default:
            return false;
    }
}

} // namespace riscv
