#include "test_framework.hpp"
#include "cpu.hpp"
#include "program_loader.hpp"
#include <fstream>
#include <cstdio>

// Helper encoder from test_instructions
namespace {
using namespace riscv;

Word make_i(uint8_t opcode, uint8_t funct3, RegId rd, RegId rs1, SignedWord imm) {
    Word imm12 = static_cast<Word>(imm) & 0xFFF;
    return (imm12 << 20) |
           (static_cast<Word>(rs1) << 15) |
           (static_cast<Word>(funct3) << 12) |
           (static_cast<Word>(rd) << 7) |
           opcode;
}

Word make_r(uint8_t opcode, uint8_t funct3, uint8_t funct7, RegId rd, RegId rs1, RegId rs2) {
    return (static_cast<Word>(funct7) << 25) |
           (static_cast<Word>(rs2) << 20) |
           (static_cast<Word>(rs1) << 15) |
           (static_cast<Word>(funct3) << 12) |
           (static_cast<Word>(rd) << 7) |
           opcode;
}

Word make_b(uint8_t opcode, uint8_t funct3, RegId rs1, RegId rs2, SignedWord imm) {
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

Word make_j(uint8_t opcode, RegId rd, SignedWord imm) {
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

TEST_CASE(Program_SumLoop) {
    // Computes sum 1..10 = 55
    Memory mem(4096, 0);
    std::vector<Word> code = {
        // 0x00: ADDI x1, x0, 10   (counter)
        make_i(0x13, 0x0, 1, 0, 10),
        // 0x04: ADDI x2, x0, 0    (sum)
        make_i(0x13, 0x0, 2, 0, 0),
        // 0x08: ADD  x2, x2, x1   (sum += counter)
        make_r(0x33, 0x0, 0x00, 2, 2, 1),
        // 0x0C: ADDI x1, x1, -1   (counter--)
        make_i(0x13, 0x0, 1, 1, -1),
        // 0x10: BNE  x1, x0, -8   (branch to 0x08 if counter != 0)
        make_b(0x63, 0x1, 1, 0, -8)
    };
    ProgramLoader::load_instructions(code, mem, 0);
    CpuConfig cfg;
    cfg.program_end_address = 0x14;
    Cpu cpu(mem, cfg);
    ASSERT_EQ(cpu.run(), CpuStatus::HALTED_END_OF_PROGRAM);

    ASSERT_EQ(cpu.registers().read(1), 0u);
    ASSERT_EQ(cpu.registers().read(2), 55u);
}

TEST_CASE(Program_Determinism) {
    // Run the sum program twice from scratch and verify identical state
    auto run_once = []() {
        Memory mem(4096, 0);
        std::vector<Word> code = {
            make_i(0x13, 0x0, 1, 0, 10),
            make_i(0x13, 0x0, 2, 0, 0),
            make_r(0x33, 0x0, 0x00, 2, 2, 1),
            make_i(0x13, 0x0, 1, 1, -1),
            make_b(0x63, 0x1, 1, 0, -8)
        };
        ProgramLoader::load_instructions(code, mem, 0);
        CpuConfig cfg;
        cfg.program_end_address = 0x14;
        Cpu cpu(mem, cfg);
        cpu.run();
        return std::make_pair(cpu.pc(), cpu.registers().read(2));
    };

    auto res1 = run_once();
    auto res2 = run_once();
    ASSERT_EQ(res1.first, res2.first);
    ASSERT_EQ(res1.second, res2.second);
}

TEST_CASE(Program_MaxInstructionsLimit) {
    // Infinite loop: 0x00: JAL x0, 0
    Memory mem(4096, 0);
    std::vector<Word> code = {
        make_j(0x6F, 0, 0)
    };
    ProgramLoader::load_instructions(code, mem, 0);
    CpuConfig cfg;
    cfg.max_instructions = 100;
    Cpu cpu(mem, cfg);
    ASSERT_EQ(cpu.run(100), CpuStatus::MAX_INSTRUCTIONS_REACHED);
    ASSERT_EQ(cpu.instruction_count(), 100u);
    ASSERT_EQ(cpu.pc(), 0x00u);
}

TEST_CASE(Program_HexFileLoadAndRun) {
    const std::string tmp_file = "temp_test_prog.hex";
    {
        std::ofstream out(tmp_file);
        out << "# Test program\n";
        out << "00a00093 # ADDI x1, x0, 10\n";
        out << "01400113 # ADDI x2, x0, 20\n";
        out << "002081b3 # ADD  x3, x1, x2\n";
    }

    Memory mem(4096, 0);
    auto load_res = ProgramLoader::load_hex_file(tmp_file, mem, 0);
    ASSERT_TRUE(load_res.success);
    ASSERT_EQ(load_res.words_loaded, 3u);

    CpuConfig cfg;
    cfg.program_end_address = load_res.end_address;
    Cpu cpu(mem, cfg);
    ASSERT_EQ(cpu.run(), CpuStatus::HALTED_END_OF_PROGRAM);

    ASSERT_EQ(cpu.registers().read(1), 10u);
    ASSERT_EQ(cpu.registers().read(2), 20u);
    ASSERT_EQ(cpu.registers().read(3), 30u);

    std::remove(tmp_file.c_str());
}

TEST_CASE(Program_DiagnosticsUnsupportedInstruction) {
    Memory mem(4096, 0);
    // Invalid instruction word: 0x00000000 (all zeroes)
    mem.write_word(0, 0x00000000);
    CpuConfig cfg;
    Cpu cpu(mem, cfg);
    ASSERT_EQ(cpu.step(), CpuStatus::UNSUPPORTED_INSTRUCTION);
}

TEST_CASE(Program_DiagnosticsMisalignedMemory) {
    Memory mem(4096, 0);
    // ADDI x1, x0, 0x101 (unaligned address for LW)
    // LW x2, 0(x1)
    std::vector<Word> code = {
        make_i(0x13, 0x0, 1, 0, 0x101),
        make_i(0x03, 0x2, 2, 1, 0)
    };
    ProgramLoader::load_instructions(code, mem, 0);
    CpuConfig cfg;
    Cpu cpu(mem, cfg);
    cpu.step(); // execute ADDI
    ASSERT_EQ(cpu.step(), CpuStatus::INVALID_MEMORY_ALIGNMENT);
}

TEST_CASE(Program_DiagnosticsMisalignedBranchTarget) {
    Memory mem(4096, 0);
    // JAL x1, +2 (misaligned target: PC + 2 = 0x02)
    std::vector<Word> code = {
        make_j(0x6F, 1, 2)
    };
    ProgramLoader::load_instructions(code, mem, 0);
    CpuConfig cfg;
    Cpu cpu(mem, cfg);
    cpu.step(); // execute JAL, PC is now 0x02
    ASSERT_EQ(cpu.step(), CpuStatus::INVALID_FETCH_ALIGNMENT);
}
