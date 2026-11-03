#include "immediate.hpp"

namespace riscv {

static inline SignedWord sign_extend(Word val, unsigned bits) {
    Word mask = 1u << (bits - 1);
    if ((val & mask) != 0) {
        // Sign bit is set: sign extend with 1s
        Word sign_bits = ~((1u << bits) - 1u);
        return static_cast<SignedWord>(val | sign_bits);
    }
    // Sign bit is 0: mask lower bits
    return static_cast<SignedWord>(val & ((1u << bits) - 1u));
}

SignedWord ImmediateGenerator::extract_i(Word instruction) {
    Word raw = (instruction >> 20) & 0xFFF;
    return sign_extend(raw, 12);
}

SignedWord ImmediateGenerator::extract_s(Word instruction) {
    Word imm_11_5 = (instruction >> 25) & 0x7F;
    Word imm_4_0 = (instruction >> 7) & 0x1F;
    Word raw = (imm_11_5 << 5) | imm_4_0;
    return sign_extend(raw, 12);
}

SignedWord ImmediateGenerator::extract_b(Word instruction) {
    Word imm_12 = (instruction >> 31) & 0x1;
    Word imm_11 = (instruction >> 7) & 0x1;
    Word imm_10_5 = (instruction >> 25) & 0x3F;
    Word imm_4_1 = (instruction >> 8) & 0x0F;
    Word raw = (imm_12 << 12) | (imm_11 << 11) | (imm_10_5 << 5) | (imm_4_1 << 1);
    return sign_extend(raw, 13);
}

Word ImmediateGenerator::extract_u(Word instruction) {
    return instruction & 0xFFFFF000;
}

SignedWord ImmediateGenerator::extract_j(Word instruction) {
    Word imm_20 = (instruction >> 31) & 0x1;
    Word imm_19_12 = (instruction >> 12) & 0xFF;
    Word imm_11 = (instruction >> 20) & 0x1;
    Word imm_10_1 = (instruction >> 21) & 0x3FF;
    Word raw = (imm_20 << 20) | (imm_19_12 << 12) | (imm_11 << 11) | (imm_10_1 << 1);
    return sign_extend(raw, 21);
}

uint32_t ImmediateGenerator::extract_shamt(Word instruction) {
    return (instruction >> 20) & 0x1F;
}

} // namespace riscv
