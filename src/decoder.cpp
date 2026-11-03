#include "decoder.hpp"
#include "immediate.hpp"
#include <iomanip>
#include <sstream>

namespace riscv {

std::string DecodedInstruction::to_string() const {
    std::ostringstream oss;
    oss << riscv::to_string(id) << " ";
    switch (format) {
        case InstructionFormat::R_TYPE:
            oss << "x" << static_cast<int>(rd) << ", x"
                << static_cast<int>(rs1) << ", x"
                << static_cast<int>(rs2);
            break;
        case InstructionFormat::I_TYPE:
            if (id == InstructionId::SLLI || id == InstructionId::SRLI || id == InstructionId::SRAI) {
                oss << "x" << static_cast<int>(rd) << ", x"
                    << static_cast<int>(rs1) << ", " << shamt;
            } else if (id == InstructionId::LB || id == InstructionId::LH || id == InstructionId::LW ||
                       id == InstructionId::LBU || id == InstructionId::LHU || id == InstructionId::JALR) {
                oss << "x" << static_cast<int>(rd) << ", " << imm << "(x"
                    << static_cast<int>(rs1) << ")";
            } else {
                oss << "x" << static_cast<int>(rd) << ", x"
                    << static_cast<int>(rs1) << ", " << imm;
            }
            break;
        case InstructionFormat::S_TYPE:
            oss << "x" << static_cast<int>(rs2) << ", " << imm << "(x"
                << static_cast<int>(rs1) << ")";
            break;
        case InstructionFormat::B_TYPE:
            oss << "x" << static_cast<int>(rs1) << ", x"
                << static_cast<int>(rs2) << ", " << imm;
            break;
        case InstructionFormat::U_TYPE:
            oss << "x" << static_cast<int>(rd) << ", 0x" << std::hex
                << (static_cast<Word>(imm) >> 12);
            break;
        case InstructionFormat::J_TYPE:
            oss << "x" << static_cast<int>(rd) << ", " << imm;
            break;
        default:
            oss << "UNKNOWN";
            break;
    }
    return oss.str();
}

CpuStatus Decoder::decode(Word raw, DecodedInstruction& out) {
    out.raw = raw;
    out.opcode = static_cast<uint8_t>(raw & 0x7F);
    out.rd = static_cast<RegId>((raw >> 7) & 0x1F);
    out.funct3 = static_cast<uint8_t>((raw >> 12) & 0x07);
    out.rs1 = static_cast<RegId>((raw >> 15) & 0x1F);
    out.rs2 = static_cast<RegId>((raw >> 20) & 0x1F);
    out.funct7 = static_cast<uint8_t>((raw >> 25) & 0x7F);
    out.imm = 0;
    out.shamt = 0;
    out.id = InstructionId::UNKNOWN;
    out.format = InstructionFormat::UNKNOWN;

    switch (out.opcode) {
        // R-type (0x33)
        case 0x33: {
            out.format = InstructionFormat::R_TYPE;
            switch (out.funct3) {
                case 0x0:
                    if (out.funct7 == 0x00) out.id = InstructionId::ADD;
                    else if (out.funct7 == 0x20) out.id = InstructionId::SUB;
                    else return CpuStatus::UNSUPPORTED_INSTRUCTION;
                    break;
                case 0x1:
                    if (out.funct7 == 0x00) out.id = InstructionId::SLL;
                    else return CpuStatus::UNSUPPORTED_INSTRUCTION;
                    break;
                case 0x2:
                    if (out.funct7 == 0x00) out.id = InstructionId::SLT;
                    else return CpuStatus::UNSUPPORTED_INSTRUCTION;
                    break;
                case 0x3:
                    if (out.funct7 == 0x00) out.id = InstructionId::SLTU;
                    else return CpuStatus::UNSUPPORTED_INSTRUCTION;
                    break;
                case 0x4:
                    if (out.funct7 == 0x00) out.id = InstructionId::XOR;
                    else return CpuStatus::UNSUPPORTED_INSTRUCTION;
                    break;
                case 0x5:
                    if (out.funct7 == 0x00) out.id = InstructionId::SRL;
                    else if (out.funct7 == 0x20) out.id = InstructionId::SRA;
                    else return CpuStatus::UNSUPPORTED_INSTRUCTION;
                    break;
                case 0x6:
                    if (out.funct7 == 0x00) out.id = InstructionId::OR;
                    else return CpuStatus::UNSUPPORTED_INSTRUCTION;
                    break;
                case 0x7:
                    if (out.funct7 == 0x00) out.id = InstructionId::AND;
                    else return CpuStatus::UNSUPPORTED_INSTRUCTION;
                    break;
                default:
                    return CpuStatus::UNSUPPORTED_INSTRUCTION;
            }
            return CpuStatus::RUNNING;
        }

        // I-type ALU (0x13)
        case 0x13: {
            out.format = InstructionFormat::I_TYPE;
            out.imm = ImmediateGenerator::extract_i(raw);
            out.shamt = ImmediateGenerator::extract_shamt(raw);

            switch (out.funct3) {
                case 0x0: out.id = InstructionId::ADDI; break;
                case 0x2: out.id = InstructionId::SLTI; break;
                case 0x3: out.id = InstructionId::SLTIU; break;
                case 0x4: out.id = InstructionId::XORI; break;
                case 0x6: out.id = InstructionId::ORI; break;
                case 0x7: out.id = InstructionId::ANDI; break;
                case 0x1:
                    // SLLI: bits [31:25] must be 0x00
                    if (out.funct7 != 0x00) return CpuStatus::MALFORMED_INSTRUCTION;
                    out.id = InstructionId::SLLI;
                    break;
                case 0x5:
                    // SRLI: funct7 0x00; SRAI: funct7 0x20
                    if (out.funct7 == 0x00) {
                        out.id = InstructionId::SRLI;
                    } else if (out.funct7 == 0x20) {
                        out.id = InstructionId::SRAI;
                    } else {
                        return CpuStatus::MALFORMED_INSTRUCTION;
                    }
                    break;
                default:
                    return CpuStatus::UNSUPPORTED_INSTRUCTION;
            }
            return CpuStatus::RUNNING;
        }

        // I-type LOAD (0x03)
        case 0x03: {
            out.format = InstructionFormat::I_TYPE;
            out.imm = ImmediateGenerator::extract_i(raw);
            switch (out.funct3) {
                case 0x0: out.id = InstructionId::LB; break;
                case 0x1: out.id = InstructionId::LH; break;
                case 0x2: out.id = InstructionId::LW; break;
                case 0x4: out.id = InstructionId::LBU; break;
                case 0x5: out.id = InstructionId::LHU; break;
                default: return CpuStatus::UNSUPPORTED_INSTRUCTION;
            }
            return CpuStatus::RUNNING;
        }

        // S-type STORE (0x23)
        case 0x23: {
            out.format = InstructionFormat::S_TYPE;
            out.imm = ImmediateGenerator::extract_s(raw);
            switch (out.funct3) {
                case 0x0: out.id = InstructionId::SB; break;
                case 0x1: out.id = InstructionId::SH; break;
                case 0x2: out.id = InstructionId::SW; break;
                default: return CpuStatus::UNSUPPORTED_INSTRUCTION;
            }
            return CpuStatus::RUNNING;
        }

        // B-type BRANCH (0x63)
        case 0x63: {
            out.format = InstructionFormat::B_TYPE;
            out.imm = ImmediateGenerator::extract_b(raw);
            switch (out.funct3) {
                case 0x0: out.id = InstructionId::BEQ; break;
                case 0x1: out.id = InstructionId::BNE; break;
                case 0x4: out.id = InstructionId::BLT; break;
                case 0x5: out.id = InstructionId::BGE; break;
                case 0x6: out.id = InstructionId::BLTU; break;
                case 0x7: out.id = InstructionId::BGEU; break;
                default: return CpuStatus::UNSUPPORTED_INSTRUCTION;
            }
            return CpuStatus::RUNNING;
        }

        // JALR (0x67)
        case 0x67: {
            if (out.funct3 != 0x0) {
                return CpuStatus::UNSUPPORTED_INSTRUCTION;
            }
            out.format = InstructionFormat::I_TYPE;
            out.imm = ImmediateGenerator::extract_i(raw);
            out.id = InstructionId::JALR;
            return CpuStatus::RUNNING;
        }

        // JAL (0x6F)
        case 0x6F: {
            out.format = InstructionFormat::J_TYPE;
            out.imm = ImmediateGenerator::extract_j(raw);
            out.id = InstructionId::JAL;
            return CpuStatus::RUNNING;
        }

        // LUI (0x37)
        case 0x37: {
            out.format = InstructionFormat::U_TYPE;
            out.imm = static_cast<SignedWord>(ImmediateGenerator::extract_u(raw));
            out.id = InstructionId::LUI;
            return CpuStatus::RUNNING;
        }

        // AUIPC (0x17)
        case 0x17: {
            out.format = InstructionFormat::U_TYPE;
            out.imm = static_cast<SignedWord>(ImmediateGenerator::extract_u(raw));
            out.id = InstructionId::AUIPC;
            return CpuStatus::RUNNING;
        }

        default:
            return CpuStatus::UNSUPPORTED_INSTRUCTION;
    }
}

} // namespace riscv
