#include "test_framework.hpp"
#include "cpu.hpp"
#include "program_loader.hpp"

// Encoders for test construction
namespace {
using namespace riscv;

Word enc_r(uint8_t opcode, uint8_t funct3, uint8_t funct7, RegId rd, RegId rs1, RegId rs2) {
    return (static_cast<Word>(funct7) << 25) |
           (static_cast<Word>(rs2) << 20) |
           (static_cast<Word>(rs1) << 15) |
           (static_cast<Word>(funct3) << 12) |
           (static_cast<Word>(rd) << 7) |
           opcode;
}

Word enc_i(uint8_t opcode, uint8_t funct3, RegId rd, RegId rs1, SignedWord imm) {
    Word imm12 = static_cast<Word>(imm) & 0xFFF;
    return (imm12 << 20) |
           (static_cast<Word>(rs1) << 15) |
           (static_cast<Word>(funct3) << 12) |
           (static_cast<Word>(rd) << 7) |
           opcode;
}

Word enc_s(uint8_t opcode, uint8_t funct3, RegId rs1, RegId rs2, SignedWord imm) {
    Word imm12 = static_cast<Word>(imm) & 0xFFF;
    Word imm_11_5 = (imm12 >> 5) & 0x7F;
    Word imm_4_0 = imm12 & 0x1F;
    return (imm_11_5 << 25) |
           (static_cast<Word>(rs2) << 20) |
           (static_cast<Word>(rs1) << 15) |
           (static_cast<Word>(funct3) << 12) |
           (imm_4_0 << 7) |
           opcode;
}

Word enc_b(uint8_t opcode, uint8_t funct3, RegId rs1, RegId rs2, SignedWord imm) {
    Word imm13 = static_cast<Word>(imm) & 0x1FFF;
    Word imm_12 = (imm13 >> 12) & 0x1;
    Word imm_11 = (imm13 >> 11) & 0x1;
    Word imm_10_5 = (imm13 >> 5) & 0x3F;
    Word imm_4_1 = (imm13 >> 1) & 0x0F;
    return (imm_12 << 31) |
           (imm_10_5 << 25) |
           (static_cast<Word>(rs2) << 20) |
           (static_cast<Word>(rs1) << 15) |
           (static_cast<Word>(funct3) << 12) |
           (imm_4_1 << 8) |
           (imm_11 << 7) |
           opcode;
}

Word enc_u(uint8_t opcode, RegId rd, Word u_imm) {
    return (u_imm & 0xFFFFF000) |
           (static_cast<Word>(rd) << 7) |
           opcode;
}

Word enc_j(uint8_t opcode, RegId rd, SignedWord imm) {
    Word imm21 = static_cast<Word>(imm) & 0x1FFFFF;
    Word imm_20 = (imm21 >> 20) & 0x1;
    Word imm_19_12 = (imm21 >> 12) & 0xFF;
    Word imm_11 = (imm21 >> 11) & 0x1;
    Word imm_10_1 = (imm21 >> 1) & 0x3FF;
    return (imm_20 << 31) |
           (imm_10_1 << 21) |
           (imm_11 << 20) |
           (imm_19_12 << 12) |
           (static_cast<Word>(rd) << 7) |
           opcode;
}

} // namespace

TEST_CASE(Instruction_Arithmetic) {
    // Test ADD, SUB, ADDI
    Memory mem(4096, 0);
    // ADDI x1, x0, 15
    // ADDI x2, x0, -5
    // ADD  x3, x1, x2  -> 10
    // SUB  x4, x1, x2  -> 20
    std::vector<Word> code = {
        enc_i(0x13, 0x0, 1, 0, 15),
        enc_i(0x13, 0x0, 2, 0, -5),
        enc_r(0x33, 0x0, 0x00, 3, 1, 2),
        enc_r(0x33, 0x0, 0x20, 4, 1, 2)
    };
    ProgramLoader::load_instructions(code, mem, 0);
    CpuConfig cfg;
    cfg.program_end_address = static_cast<Address>(code.size() * 4);
    Cpu cpu(mem, cfg);
    ASSERT_EQ(cpu.run(), CpuStatus::HALTED_END_OF_PROGRAM);

    ASSERT_EQ(cpu.registers().read(1), 15u);
    ASSERT_EQ(cpu.registers().read(2), static_cast<Word>(-5));
    ASSERT_EQ(cpu.registers().read(3), 10u);
    ASSERT_EQ(cpu.registers().read(4), 20u);
}

TEST_CASE(Instruction_Logical) {
    // AND, OR, XOR, ANDI, ORI, XORI
    Memory mem(4096, 0);
    std::vector<Word> code = {
        enc_i(0x13, 0x0, 1, 0, 0x0F),    // ADDI x1, x0, 0x0F
        enc_i(0x13, 0x7, 2, 1, 0x07),    // ANDI x2, x1, 0x07 -> 0x07
        enc_i(0x13, 0x6, 3, 1, 0xF0),    // ORI  x3, x1, 0xF0 -> 0xFF
        enc_i(0x13, 0x4, 4, 1, 0x0F),    // XORI x4, x1, 0x0F -> 0x00
        enc_r(0x33, 0x7, 0x00, 5, 2, 3), // AND  x5, x2, x3 -> 0x07 & 0xFF = 0x07
        enc_r(0x33, 0x6, 0x00, 6, 2, 3), // OR   x6, x2, x3 -> 0x07 | 0xFF = 0xFF
        enc_r(0x33, 0x4, 0x00, 7, 2, 3)  // XOR  x7, x2, x3 -> 0x07 ^ 0xFF = 0xF8
    };
    ProgramLoader::load_instructions(code, mem, 0);
    CpuConfig cfg;
    cfg.program_end_address = static_cast<Address>(code.size() * 4);
    Cpu cpu(mem, cfg);
    ASSERT_EQ(cpu.run(), CpuStatus::HALTED_END_OF_PROGRAM);

    ASSERT_EQ(cpu.registers().read(2), 0x07u);
    ASSERT_EQ(cpu.registers().read(3), 0xFFu);
    ASSERT_EQ(cpu.registers().read(4), 0x00u);
    ASSERT_EQ(cpu.registers().read(5), 0x07u);
    ASSERT_EQ(cpu.registers().read(6), 0xFFu);
    ASSERT_EQ(cpu.registers().read(7), 0xF8u);
}

TEST_CASE(Instruction_Shifts) {
    // SLL, SRL, SRA, SLLI, SRLI, SRAI
    Memory mem(4096, 0);
    std::vector<Word> code = {
        enc_i(0x13, 0x0, 1, 0, -16),      // ADDI x1, x0, -16 (0xFFFFFFF0)
        enc_i(0x13, 0x1, 2, 1, 2),        // SLLI x2, x1, 2
        enc_i(0x13, 0x5, 3, 1, 2),        // SRLI x3, x1, 2 (funct7=0x00)
        enc_r(0x13, 0x5, 0x20, 4, 1, 2),  // SRAI x4, x1, 2 (funct7=0x20, shamt=2 in rs2 field)
        enc_i(0x13, 0x0, 5, 0, 3),        // ADDI x5, x0, 3 (shift amount register)
        enc_r(0x33, 0x1, 0x00, 6, 1, 5),  // SLL  x6, x1, x5
        enc_r(0x33, 0x5, 0x00, 7, 1, 5),  // SRL  x7, x1, x5
        enc_r(0x33, 0x5, 0x20, 8, 1, 5)   // SRA  x8, x1, x5
    };
    ProgramLoader::load_instructions(code, mem, 0);
    CpuConfig cfg;
    cfg.program_end_address = static_cast<Address>(code.size() * 4);
    Cpu cpu(mem, cfg);
    ASSERT_EQ(cpu.run(), CpuStatus::HALTED_END_OF_PROGRAM);

    ASSERT_EQ(cpu.registers().read(2), 0xFFFFFFC0u); // SLLI by 2
    ASSERT_EQ(cpu.registers().read(3), 0x3FFFFFFCu); // SRLI by 2
    ASSERT_EQ(cpu.registers().read(4), 0xFFFFFFFCu); // SRAI by 2 (-4)
    ASSERT_EQ(cpu.registers().read(6), 0xFFFFFF80u); // SLL by 3
    ASSERT_EQ(cpu.registers().read(7), 0x1FFFFFFEu); // SRL by 3
    ASSERT_EQ(cpu.registers().read(8), 0xFFFFFFFEu); // SRA by 3 (-2)
}

TEST_CASE(Instruction_Comparison) {
    // SLT, SLTU, SLTI, SLTIU
    Memory mem(4096, 0);
    std::vector<Word> code = {
        enc_i(0x13, 0x0, 1, 0, -5),       // ADDI x1, x0, -5
        enc_i(0x13, 0x0, 2, 0, 5),        // ADDI x2, x0, 5
        enc_r(0x33, 0x2, 0x00, 3, 1, 2),  // SLT  x3, x1, x2 -> (-5 < 5) = 1
        enc_r(0x33, 0x3, 0x00, 4, 1, 2),  // SLTU x4, x1, x2 -> (0xFFFFFFFB < 5) = 0
        enc_i(0x13, 0x2, 5, 1, 0),        // SLTI x5, x1, 0  -> (-5 < 0) = 1
        enc_i(0x13, 0x3, 6, 1, 0)         // SLTIU x6, x1, 0 -> (0xFFFFFFFB < 0) = 0
    };
    ProgramLoader::load_instructions(code, mem, 0);
    CpuConfig cfg;
    cfg.program_end_address = static_cast<Address>(code.size() * 4);
    Cpu cpu(mem, cfg);
    ASSERT_EQ(cpu.run(), CpuStatus::HALTED_END_OF_PROGRAM);

    ASSERT_EQ(cpu.registers().read(3), 1u);
    ASSERT_EQ(cpu.registers().read(4), 0u);
    ASSERT_EQ(cpu.registers().read(5), 1u);
    ASSERT_EQ(cpu.registers().read(6), 0u);
}

TEST_CASE(Instruction_MemoryOperations) {
    // LB, LH, LW, LBU, LHU, SB, SH, SW
    Memory mem(4096, 0);
    // Data memory area starting at address 0x200
    std::vector<Word> code = {
        enc_i(0x13, 0x0, 1, 0, 0x200),     // ADDI x1, x0, 0x200 (base address)
        enc_i(0x13, 0x0, 2, 0, -2),        // ADDI x2, x0, -2 (0xFFFFFFFE)
        enc_s(0x23, 0x0, 1, 2, 0),         // SB x2, 0(x1)   -> writes 0xFE
        enc_i(0x03, 0x0, 3, 1, 0),         // LB x3, 0(x1)   -> sign extend 0xFE -> 0xFFFFFFFE (-2)
        enc_i(0x03, 0x4, 4, 1, 0),         // LBU x4, 0(x1)  -> zero extend 0xFE -> 0x000000FE (254)

        enc_i(0x13, 0x0, 5, 0, -1000),     // ADDI x5, x0, -1000
        enc_s(0x23, 0x1, 1, 5, 4),         // SH x5, 4(x1)
        enc_i(0x03, 0x1, 6, 1, 4),         // LH x6, 4(x1)   -> sign extended
        enc_i(0x03, 0x5, 7, 1, 4),         // LHU x7, 4(x1)  -> zero extended

        enc_u(0x37, 8, 0x12345000),        // LUI x8, 0x12345000
        enc_i(0x13, 0x0, 8, 8, 0x678),     // ADDI x8, x8, 0x678 -> 0x12345678
        enc_s(0x23, 0x2, 1, 8, 8),         // SW x8, 8(x1)
        enc_i(0x03, 0x2, 9, 1, 8)          // LW x9, 8(x1)
    };
    ProgramLoader::load_instructions(code, mem, 0);
    CpuConfig cfg;
    cfg.program_end_address = static_cast<Address>(code.size() * 4);
    Cpu cpu(mem, cfg);
    ASSERT_EQ(cpu.run(), CpuStatus::HALTED_END_OF_PROGRAM);

    ASSERT_EQ(cpu.registers().read(3), 0xFFFFFFFEu);
    ASSERT_EQ(cpu.registers().read(4), 0x000000FEu);
    ASSERT_EQ(cpu.registers().read(6), static_cast<Word>(static_cast<SignedWord>(-1000)));
    ASSERT_EQ(cpu.registers().read(7), static_cast<Word>(static_cast<uint16_t>(-1000)));
    ASSERT_EQ(cpu.registers().read(9), 0x12345678u);
}

TEST_CASE(Instruction_Branches) {
    // BEQ, BNE, BLT, BGE, BLTU, BGEU
    Memory mem(4096, 0);
    // Program: test taken and not-taken branches
    std::vector<Word> code = {
        enc_i(0x13, 0x0, 1, 0, 10),       // 0x00: ADDI x1, x0, 10
        enc_i(0x13, 0x0, 2, 0, 20),       // 0x04: ADDI x2, x0, 20
        enc_b(0x63, 0x0, 1, 2, 8),        // 0x08: BEQ x1, x2, +8 (not taken)
        enc_i(0x13, 0x0, 3, 0, 1),        // 0x0C: ADDI x3, x0, 1 (executed)
        enc_b(0x63, 0x1, 1, 2, 8),        // 0x10: BNE x1, x2, +8 (taken -> skip 0x14)
        enc_i(0x13, 0x0, 4, 0, 99),       // 0x14: ADDI x4, x0, 99 (skipped)
        enc_b(0x63, 0x4, 1, 2, 8),        // 0x18: BLT x1, x2, +8 (taken -> skip 0x1C)
        enc_i(0x13, 0x0, 5, 0, 99),       // 0x1C: ADDI x5, x0, 99 (skipped)
        enc_b(0x63, 0x5, 1, 2, 8),        // 0x20: BGE x1, x2, +8 (not taken)
        enc_i(0x13, 0x0, 6, 0, 2),        // 0x24: ADDI x6, x0, 2 (executed)
        enc_b(0x63, 0x6, 1, 2, 8),        // 0x28: BLTU x1, x2, +8 (taken -> skip 0x2C)
        enc_i(0x13, 0x0, 7, 0, 99),       // 0x2C: ADDI x7, x0, 99 (skipped)
        enc_b(0x63, 0x7, 1, 2, 8),        // 0x30: BGEU x1, x2, +8 (not taken)
        enc_i(0x13, 0x0, 8, 0, 3)         // 0x34: ADDI x8, x0, 3 (executed)
    };
    ProgramLoader::load_instructions(code, mem, 0);
    CpuConfig cfg;
    cfg.program_end_address = static_cast<Address>(code.size() * 4);
    Cpu cpu(mem, cfg);
    ASSERT_EQ(cpu.run(), CpuStatus::HALTED_END_OF_PROGRAM);

    ASSERT_EQ(cpu.registers().read(3), 1u);
    ASSERT_EQ(cpu.registers().read(4), 0u); // skipped
    ASSERT_EQ(cpu.registers().read(5), 0u); // skipped
    ASSERT_EQ(cpu.registers().read(6), 2u);
    ASSERT_EQ(cpu.registers().read(7), 0u); // skipped
    ASSERT_EQ(cpu.registers().read(8), 3u);
}

TEST_CASE(Instruction_JumpAndLink) {
    // JAL and JALR
    Memory mem(4096, 0);
    std::vector<Word> code = {
        // 0x00: JAL x1, +12 (target 0x0C, link x1 = 0x04)
        enc_j(0x6F, 1, 12),
        // 0x04: ADDI x2, x0, 99 (skipped)
        enc_i(0x13, 0x0, 2, 0, 99),
        // 0x08: ADDI x2, x0, 99 (skipped)
        enc_i(0x13, 0x0, 2, 0, 99),
        // 0x0C: ADDI x3, x0, 0x20
        enc_i(0x13, 0x0, 3, 0, 0x20),
        // 0x10: JALR x4, 4(x3) (target = (0x20 + 4) & ~1 = 0x24, link x4 = 0x14)
        enc_i(0x67, 0x0, 4, 3, 4)
    };
    ProgramLoader::load_instructions(code, mem, 0);

    // Target 0x24: ADDI x5, x0, 42
    mem.write_word(0x24, enc_i(0x13, 0x0, 5, 0, 42));

    CpuConfig cfg;
    cfg.program_end_address = 0x28;
    Cpu cpu(mem, cfg);
    ASSERT_EQ(cpu.run(), CpuStatus::HALTED_END_OF_PROGRAM);

    ASSERT_EQ(cpu.registers().read(1), 0x04u); // JAL link
    ASSERT_EQ(cpu.registers().read(2), 0u);    // skipped
    ASSERT_EQ(cpu.registers().read(4), 0x14u); // JALR link
    ASSERT_EQ(cpu.registers().read(5), 42u);   // reached target
}

TEST_CASE(Instruction_UpperImmediates) {
    // LUI and AUIPC
    Memory mem(4096, 0x1000);
    std::vector<Word> code = {
        // 0x1000: LUI x1, 0xABCDE000
        enc_u(0x37, 1, 0xABCDE000),
        // 0x1004: AUIPC x2, 0x00004000 -> 0x1004 + 0x4000 = 0x5004
        enc_u(0x17, 2, 0x00004000)
    };
    ProgramLoader::load_instructions(code, mem, 0x1000);
    CpuConfig cfg;
    cfg.reset_vector = 0x1000;
    cfg.program_end_address = 0x1008;
    Cpu cpu(mem, cfg);
    ASSERT_EQ(cpu.run(), CpuStatus::HALTED_END_OF_PROGRAM);

    ASSERT_EQ(cpu.registers().read(1), 0xABCDE000u);
    ASSERT_EQ(cpu.registers().read(2), 0x00005004u);
}

TEST_CASE(Instruction_ZeroRegisterInvariant) {
    Memory mem(4096, 0);
    std::vector<Word> code = {
        enc_i(0x13, 0x0, 0, 0, 100),       // ADDI x0, x0, 100
        enc_r(0x33, 0x0, 0x00, 0, 0, 0),   // ADD  x0, x0, x0
        enc_u(0x37, 0, 0x12345000),        // LUI  x0, 0x12345000
        enc_j(0x6F, 0, 4)                  // JAL  x0, +4
    };
    ProgramLoader::load_instructions(code, mem, 0);
    CpuConfig cfg;
    cfg.program_end_address = static_cast<Address>(code.size() * 4);
    Cpu cpu(mem, cfg);
    ASSERT_EQ(cpu.run(), CpuStatus::HALTED_END_OF_PROGRAM);

    ASSERT_EQ(cpu.registers().read(0), 0u);
}
