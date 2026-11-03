#ifndef RISCV_TYPES_HPP
#define RISCV_TYPES_HPP

#include <cstdint>
#include <string>
#include <ostream>

namespace riscv {

using Word = uint32_t;
using SignedWord = int32_t;
using HalfWord = uint16_t;
using SignedHalfWord = int16_t;
using Byte = uint8_t;
using SignedByte = int8_t;
using Address = uint32_t;
using RegId = uint8_t;

constexpr size_t NUM_REGISTERS = 32;
constexpr Address DEFAULT_RESET_VECTOR = 0x00000000;
constexpr size_t INSTRUCTION_SIZE = 4;

enum class InstructionFormat {
    R_TYPE,
    I_TYPE,
    S_TYPE,
    B_TYPE,
    U_TYPE,
    J_TYPE,
    UNKNOWN
};

enum class InstructionId {
    // Arithmetic
    ADD,
    SUB,
    ADDI,
    // Logical
    AND,
    OR,
    XOR,
    ANDI,
    ORI,
    XORI,
    // Shifts
    SLL,
    SRL,
    SRA,
    SLLI,
    SRLI,
    SRAI,
    // Comparison
    SLT,
    SLTU,
    SLTI,
    SLTIU,
    // Memory
    LB,
    LH,
    LW,
    LBU,
    LHU,
    SB,
    SH,
    SW,
    // Branch
    BEQ,
    BNE,
    BLT,
    BGE,
    BLTU,
    BGEU,
    // Jump
    JAL,
    JALR,
    // Upper immediate
    LUI,
    AUIPC,
    // Unknown/Unsupported
    UNKNOWN
};

enum class CpuStatus {
    RUNNING,
    HALTED_END_OF_PROGRAM,
    MAX_INSTRUCTIONS_REACHED,
    INVALID_FETCH_ALIGNMENT,
    INVALID_FETCH_OUT_OF_BOUNDS,
    UNSUPPORTED_INSTRUCTION,
    MALFORMED_INSTRUCTION,
    INVALID_MEMORY_ALIGNMENT,
    INVALID_MEMORY_OUT_OF_BOUNDS
};

inline std::string to_string(InstructionId id) {
    switch (id) {
        case InstructionId::ADD: return "ADD";
        case InstructionId::SUB: return "SUB";
        case InstructionId::ADDI: return "ADDI";
        case InstructionId::AND: return "AND";
        case InstructionId::OR: return "OR";
        case InstructionId::XOR: return "XOR";
        case InstructionId::ANDI: return "ANDI";
        case InstructionId::ORI: return "ORI";
        case InstructionId::XORI: return "XORI";
        case InstructionId::SLL: return "SLL";
        case InstructionId::SRL: return "SRL";
        case InstructionId::SRA: return "SRA";
        case InstructionId::SLLI: return "SLLI";
        case InstructionId::SRLI: return "SRLI";
        case InstructionId::SRAI: return "SRAI";
        case InstructionId::SLT: return "SLT";
        case InstructionId::SLTU: return "SLTU";
        case InstructionId::SLTI: return "SLTI";
        case InstructionId::SLTIU: return "SLTIU";
        case InstructionId::LB: return "LB";
        case InstructionId::LH: return "LH";
        case InstructionId::LW: return "LW";
        case InstructionId::LBU: return "LBU";
        case InstructionId::LHU: return "LHU";
        case InstructionId::SB: return "SB";
        case InstructionId::SH: return "SH";
        case InstructionId::SW: return "SW";
        case InstructionId::BEQ: return "BEQ";
        case InstructionId::BNE: return "BNE";
        case InstructionId::BLT: return "BLT";
        case InstructionId::BGE: return "BGE";
        case InstructionId::BLTU: return "BLTU";
        case InstructionId::BGEU: return "BGEU";
        case InstructionId::JAL: return "JAL";
        case InstructionId::JALR: return "JALR";
        case InstructionId::LUI: return "LUI";
        case InstructionId::AUIPC: return "AUIPC";
        default: return "UNKNOWN";
    }
}

inline std::string to_string(CpuStatus status) {
    switch (status) {
        case CpuStatus::RUNNING: return "RUNNING";
        case CpuStatus::HALTED_END_OF_PROGRAM: return "HALTED_END_OF_PROGRAM";
        case CpuStatus::MAX_INSTRUCTIONS_REACHED: return "MAX_INSTRUCTIONS_REACHED";
        case CpuStatus::INVALID_FETCH_ALIGNMENT: return "INVALID_FETCH_ALIGNMENT";
        case CpuStatus::INVALID_FETCH_OUT_OF_BOUNDS: return "INVALID_FETCH_OUT_OF_BOUNDS";
        case CpuStatus::UNSUPPORTED_INSTRUCTION: return "UNSUPPORTED_INSTRUCTION";
        case CpuStatus::MALFORMED_INSTRUCTION: return "MALFORMED_INSTRUCTION";
        case CpuStatus::INVALID_MEMORY_ALIGNMENT: return "INVALID_MEMORY_ALIGNMENT";
        case CpuStatus::INVALID_MEMORY_OUT_OF_BOUNDS: return "INVALID_MEMORY_OUT_OF_BOUNDS";
        default: return "UNKNOWN_STATUS";
    }
}

inline std::ostream& operator<<(std::ostream& os, InstructionId id) {
    return os << to_string(id);
}

inline std::ostream& operator<<(std::ostream& os, CpuStatus status) {
    return os << to_string(status);
}

} // namespace riscv

#endif // RISCV_TYPES_HPP
