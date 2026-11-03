#include "cpu.hpp"
#include "decoder.hpp"
#include "alu.hpp"
#include <iomanip>
#include <sstream>

namespace riscv {

static inline SignedWord sign_extend_value(Word val, unsigned bits) {
    Word mask = 1u << (bits - 1);
    if ((val & mask) != 0) {
        Word sign_bits = ~((1u << bits) - 1u);
        return static_cast<SignedWord>(val | sign_bits);
    }
    return static_cast<SignedWord>(val & ((1u << bits) - 1u));
}

Cpu::Cpu(Memory& memory, CpuConfig config)
    : memory_(memory), config_(config), pc_(config.reset_vector) {}

void Cpu::reset() {
    pc_ = config_.reset_vector;
    instruction_count_ = 0;
    cycle_count_ = 0;
    status_ = CpuStatus::RUNNING;
    reg_file_.reset();
    last_instruction_ = DecodedInstruction{};
}

CpuStatus Cpu::step() {
    if (status_ != CpuStatus::RUNNING) {
        return status_;
    }

    // Check if PC reached or exceeded program boundary
    if (config_.program_end_address.has_value() && pc_ >= *config_.program_end_address) {
        status_ = CpuStatus::HALTED_END_OF_PROGRAM;
        return status_;
    }

    // Fetch
    Word raw_instruction = 0;
    CpuStatus fetch_status = memory_.fetch_instruction(pc_, raw_instruction);
    if (fetch_status != CpuStatus::RUNNING) {
        status_ = fetch_status;
        return status_;
    }

    // Decode
    DecodedInstruction inst;
    CpuStatus decode_status = Decoder::decode(raw_instruction, inst);
    if (decode_status != CpuStatus::RUNNING) {
        status_ = decode_status;
        return status_;
    }

    last_instruction_ = inst;

    Word rs1_val = reg_file_.read(inst.rs1);
    Word rs2_val = reg_file_.read(inst.rs2);
    Address next_pc = pc_ + 4;
    Address target_pc = next_pc;

    // Execute, Memory, Write-Back
    switch (inst.id) {
        // R-type Arithmetic & Logic
        case InstructionId::ADD:
            reg_file_.write(inst.rd, Alu::add(rs1_val, rs2_val));
            break;
        case InstructionId::SUB:
            reg_file_.write(inst.rd, Alu::sub(rs1_val, rs2_val));
            break;
        case InstructionId::SLL:
            reg_file_.write(inst.rd, Alu::sll(rs1_val, rs2_val & 0x1F));
            break;
        case InstructionId::SLT:
            reg_file_.write(inst.rd, Alu::slt(rs1_val, rs2_val));
            break;
        case InstructionId::SLTU:
            reg_file_.write(inst.rd, Alu::sltu(rs1_val, rs2_val));
            break;
        case InstructionId::XOR:
            reg_file_.write(inst.rd, Alu::bit_xor(rs1_val, rs2_val));
            break;
        case InstructionId::SRL:
            reg_file_.write(inst.rd, Alu::srl(rs1_val, rs2_val & 0x1F));
            break;
        case InstructionId::SRA:
            reg_file_.write(inst.rd, Alu::sra(rs1_val, rs2_val & 0x1F));
            break;
        case InstructionId::OR:
            reg_file_.write(inst.rd, Alu::bit_or(rs1_val, rs2_val));
            break;
        case InstructionId::AND:
            reg_file_.write(inst.rd, Alu::bit_and(rs1_val, rs2_val));
            break;

        // I-type ALU
        case InstructionId::ADDI:
            reg_file_.write(inst.rd, Alu::add(rs1_val, static_cast<Word>(inst.imm)));
            break;
        case InstructionId::SLTI:
            reg_file_.write(inst.rd, Alu::slt(rs1_val, static_cast<Word>(inst.imm)));
            break;
        case InstructionId::SLTIU:
            reg_file_.write(inst.rd, Alu::sltu(rs1_val, static_cast<Word>(inst.imm)));
            break;
        case InstructionId::XORI:
            reg_file_.write(inst.rd, Alu::bit_xor(rs1_val, static_cast<Word>(inst.imm)));
            break;
        case InstructionId::ORI:
            reg_file_.write(inst.rd, Alu::bit_or(rs1_val, static_cast<Word>(inst.imm)));
            break;
        case InstructionId::ANDI:
            reg_file_.write(inst.rd, Alu::bit_and(rs1_val, static_cast<Word>(inst.imm)));
            break;
        case InstructionId::SLLI:
            reg_file_.write(inst.rd, Alu::sll(rs1_val, inst.shamt));
            break;
        case InstructionId::SRLI:
            reg_file_.write(inst.rd, Alu::srl(rs1_val, inst.shamt));
            break;
        case InstructionId::SRAI:
            reg_file_.write(inst.rd, Alu::sra(rs1_val, inst.shamt));
            break;

        // I-type LOAD
        case InstructionId::LB: {
            Address addr = rs1_val + static_cast<Word>(inst.imm);
            Byte val = 0;
            CpuStatus st = memory_.read_byte(addr, val);
            if (st != CpuStatus::RUNNING) {
                status_ = st;
                return status_;
            }
            reg_file_.write(inst.rd, static_cast<Word>(sign_extend_value(val, 8)));
            break;
        }
        case InstructionId::LH: {
            Address addr = rs1_val + static_cast<Word>(inst.imm);
            HalfWord val = 0;
            CpuStatus st = memory_.read_half(addr, val);
            if (st != CpuStatus::RUNNING) {
                status_ = st;
                return status_;
            }
            reg_file_.write(inst.rd, static_cast<Word>(sign_extend_value(val, 16)));
            break;
        }
        case InstructionId::LW: {
            Address addr = rs1_val + static_cast<Word>(inst.imm);
            Word val = 0;
            CpuStatus st = memory_.read_word(addr, val);
            if (st != CpuStatus::RUNNING) {
                status_ = st;
                return status_;
            }
            reg_file_.write(inst.rd, val);
            break;
        }
        case InstructionId::LBU: {
            Address addr = rs1_val + static_cast<Word>(inst.imm);
            Byte val = 0;
            CpuStatus st = memory_.read_byte(addr, val);
            if (st != CpuStatus::RUNNING) {
                status_ = st;
                return status_;
            }
            reg_file_.write(inst.rd, static_cast<Word>(val));
            break;
        }
        case InstructionId::LHU: {
            Address addr = rs1_val + static_cast<Word>(inst.imm);
            HalfWord val = 0;
            CpuStatus st = memory_.read_half(addr, val);
            if (st != CpuStatus::RUNNING) {
                status_ = st;
                return status_;
            }
            reg_file_.write(inst.rd, static_cast<Word>(val));
            break;
        }

        // S-type STORE
        case InstructionId::SB: {
            Address addr = rs1_val + static_cast<Word>(inst.imm);
            CpuStatus st = memory_.write_byte(addr, static_cast<Byte>(rs2_val & 0xFF));
            if (st != CpuStatus::RUNNING) {
                status_ = st;
                return status_;
            }
            break;
        }
        case InstructionId::SH: {
            Address addr = rs1_val + static_cast<Word>(inst.imm);
            CpuStatus st = memory_.write_half(addr, static_cast<HalfWord>(rs2_val & 0xFFFF));
            if (st != CpuStatus::RUNNING) {
                status_ = st;
                return status_;
            }
            break;
        }
        case InstructionId::SW: {
            Address addr = rs1_val + static_cast<Word>(inst.imm);
            CpuStatus st = memory_.write_word(addr, rs2_val);
            if (st != CpuStatus::RUNNING) {
                status_ = st;
                return status_;
            }
            break;
        }

        // B-type BRANCH
        case InstructionId::BEQ:
        case InstructionId::BNE:
        case InstructionId::BLT:
        case InstructionId::BGE:
        case InstructionId::BLTU:
        case InstructionId::BGEU: {
            if (Alu::evaluate_branch(inst.id, rs1_val, rs2_val)) {
                target_pc = pc_ + static_cast<Word>(inst.imm);
            }
            break;
        }

        // J-type JAL
        case InstructionId::JAL: {
            reg_file_.write(inst.rd, next_pc);
            target_pc = pc_ + static_cast<Word>(inst.imm);
            break;
        }

        // I-type JALR
        case InstructionId::JALR: {
            reg_file_.write(inst.rd, next_pc);
            target_pc = (rs1_val + static_cast<Word>(inst.imm)) & ~static_cast<Address>(1);
            break;
        }

        // U-type LUI
        case InstructionId::LUI: {
            reg_file_.write(inst.rd, static_cast<Word>(inst.imm));
            break;
        }

        // U-type AUIPC
        case InstructionId::AUIPC: {
            reg_file_.write(inst.rd, pc_ + static_cast<Word>(inst.imm));
            break;
        }

        default:
            status_ = CpuStatus::UNSUPPORTED_INSTRUCTION;
            return status_;
    }

    pc_ = target_pc;
    instruction_count_++;
    cycle_count_++;

    return status_;
}

CpuStatus Cpu::run(uint64_t max_instructions) {
    uint64_t limit = (max_instructions > 0) ? max_instructions : config_.max_instructions;
    while (status_ == CpuStatus::RUNNING) {
        if (instruction_count_ >= limit) {
            status_ = CpuStatus::MAX_INSTRUCTIONS_REACHED;
            break;
        }
        step();
    }
    return status_;
}

std::string Cpu::dump_state() const {
    std::ostringstream oss;
    oss << "=== CPU Architectural State ===\n";
    oss << "PC: 0x" << std::hex << std::setw(8) << std::setfill('0') << pc_
        << " | Instructions: " << std::dec << instruction_count_
        << " | Cycles: " << cycle_count_
        << " | Status: " << to_string(status_) << "\n";
    oss << "Last Instruction: " << last_instruction_.to_string()
        << " (0x" << std::hex << std::setw(8) << std::setfill('0') << last_instruction_.raw << ")\n";
    oss << "Registers:\n" << reg_file_.dump();
    return oss.str();
}

} // namespace riscv
