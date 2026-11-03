#ifndef RISCV_INSTRUCTION_HPP
#define RISCV_INSTRUCTION_HPP

#include "types.hpp"
#include <string>

namespace riscv {

struct DecodedInstruction {
    Word raw{0};
    InstructionId id{InstructionId::UNKNOWN};
    InstructionFormat format{InstructionFormat::UNKNOWN};
    uint8_t opcode{0};
    RegId rd{0};
    uint8_t funct3{0};
    RegId rs1{0};
    RegId rs2{0};
    uint8_t funct7{0};
    SignedWord imm{0};
    uint32_t shamt{0};

    std::string to_string() const;
};

} // namespace riscv

#endif // RISCV_INSTRUCTION_HPP
