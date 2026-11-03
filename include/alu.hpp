#ifndef RISCV_ALU_HPP
#define RISCV_ALU_HPP

#include "types.hpp"

namespace riscv {

class Alu {
public:
    static Word add(Word a, Word b);
    static Word sub(Word a, Word b);
    static Word bit_and(Word a, Word b);
    static Word bit_or(Word a, Word b);
    static Word bit_xor(Word a, Word b);
    static Word sll(Word a, uint32_t shamt);
    static Word srl(Word a, uint32_t shamt);
    static Word sra(Word a, uint32_t shamt);
    static Word slt(Word a, Word b);
    static Word sltu(Word a, Word b);

    // Branch condition evaluations
    static bool evaluate_branch(InstructionId branch_id, Word rs1_val, Word rs2_val);
};

} // namespace riscv

#endif // RISCV_ALU_HPP
