#include "test_framework.hpp"
#include "decoder.hpp"

TEST_CASE(Decoder_RTypeAll) {
    riscv::DecodedInstruction inst;

    // ADD x1, x2, x3: opcode 0x33, funct3 0x0, funct7 0x00, rd 1, rs1 2, rs2 3
    riscv::Word add_inst = (0x00u << 25) | (3u << 20) | (2u << 15) | (0x0u << 12) | (1u << 7) | 0x33u;
    ASSERT_EQ(riscv::Decoder::decode(add_inst, inst), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(inst.id, riscv::InstructionId::ADD);
    ASSERT_EQ(inst.rd, 1u);
    ASSERT_EQ(inst.rs1, 2u);
    ASSERT_EQ(inst.rs2, 3u);

    // SUB x1, x2, x3: funct7 0x20
    riscv::Word sub_inst = (0x20u << 25) | (3u << 20) | (2u << 15) | (0x0u << 12) | (1u << 7) | 0x33u;
    ASSERT_EQ(riscv::Decoder::decode(sub_inst, inst), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(inst.id, riscv::InstructionId::SUB);

    // SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND
    riscv::Word sll_inst = (0x00u << 25) | (3u << 20) | (2u << 15) | (0x1u << 12) | (1u << 7) | 0x33u;
    ASSERT_EQ(riscv::Decoder::decode(sll_inst, inst), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(inst.id, riscv::InstructionId::SLL);

    riscv::Word slt_inst = (0x00u << 25) | (3u << 20) | (2u << 15) | (0x2u << 12) | (1u << 7) | 0x33u;
    ASSERT_EQ(riscv::Decoder::decode(slt_inst, inst), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(inst.id, riscv::InstructionId::SLT);

    riscv::Word sltu_inst = (0x00u << 25) | (3u << 20) | (2u << 15) | (0x3u << 12) | (1u << 7) | 0x33u;
    ASSERT_EQ(riscv::Decoder::decode(sltu_inst, inst), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(inst.id, riscv::InstructionId::SLTU);

    riscv::Word xor_inst = (0x00u << 25) | (3u << 20) | (2u << 15) | (0x4u << 12) | (1u << 7) | 0x33u;
    ASSERT_EQ(riscv::Decoder::decode(xor_inst, inst), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(inst.id, riscv::InstructionId::XOR);

    riscv::Word srl_inst = (0x00u << 25) | (3u << 20) | (2u << 15) | (0x5u << 12) | (1u << 7) | 0x33u;
    ASSERT_EQ(riscv::Decoder::decode(srl_inst, inst), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(inst.id, riscv::InstructionId::SRL);

    riscv::Word sra_inst = (0x20u << 25) | (3u << 20) | (2u << 15) | (0x5u << 12) | (1u << 7) | 0x33u;
    ASSERT_EQ(riscv::Decoder::decode(sra_inst, inst), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(inst.id, riscv::InstructionId::SRA);

    riscv::Word or_inst = (0x00u << 25) | (3u << 20) | (2u << 15) | (0x6u << 12) | (1u << 7) | 0x33u;
    ASSERT_EQ(riscv::Decoder::decode(or_inst, inst), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(inst.id, riscv::InstructionId::OR);

    riscv::Word and_inst = (0x00u << 25) | (3u << 20) | (2u << 15) | (0x7u << 12) | (1u << 7) | 0x33u;
    ASSERT_EQ(riscv::Decoder::decode(and_inst, inst), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(inst.id, riscv::InstructionId::AND);

    // Invalid funct7 for ADD (e.g. 0x01)
    riscv::Word bad_r = (0x01u << 25) | (3u << 20) | (2u << 15) | (0x0u << 12) | (1u << 7) | 0x33u;
    ASSERT_EQ(riscv::Decoder::decode(bad_r, inst), riscv::CpuStatus::UNSUPPORTED_INSTRUCTION);
}

TEST_CASE(Decoder_ITypeShiftValidation) {
    riscv::DecodedInstruction inst;

    // SLLI with valid funct7 0x00
    riscv::Word slli_good = (0x00u << 25) | (5u << 20) | (1u << 15) | (0x1u << 12) | (2u << 7) | 0x13u;
    ASSERT_EQ(riscv::Decoder::decode(slli_good, inst), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(inst.id, riscv::InstructionId::SLLI);
    ASSERT_EQ(inst.shamt, 5u);

    // SLLI with bad funct7 0x20 -> must be rejected as malformed
    riscv::Word slli_bad = (0x20u << 25) | (5u << 20) | (1u << 15) | (0x1u << 12) | (2u << 7) | 0x13u;
    ASSERT_EQ(riscv::Decoder::decode(slli_bad, inst), riscv::CpuStatus::MALFORMED_INSTRUCTION);

    // SRLI with funct7 0x00
    riscv::Word srli_good = (0x00u << 25) | (4u << 20) | (1u << 15) | (0x5u << 12) | (2u << 7) | 0x13u;
    ASSERT_EQ(riscv::Decoder::decode(srli_good, inst), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(inst.id, riscv::InstructionId::SRLI);

    // SRAI with funct7 0x20
    riscv::Word srai_good = (0x20u << 25) | (4u << 20) | (1u << 15) | (0x5u << 12) | (2u << 7) | 0x13u;
    ASSERT_EQ(riscv::Decoder::decode(srai_good, inst), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(inst.id, riscv::InstructionId::SRAI);

    // Shift with bad funct7 0x10
    riscv::Word shift_bad = (0x10u << 25) | (4u << 20) | (1u << 15) | (0x5u << 12) | (2u << 7) | 0x13u;
    ASSERT_EQ(riscv::Decoder::decode(shift_bad, inst), riscv::CpuStatus::MALFORMED_INSTRUCTION);
}

TEST_CASE(Decoder_UnsupportedOpcodes) {
    riscv::DecodedInstruction inst;

    // All zero
    ASSERT_EQ(riscv::Decoder::decode(0x00000000, inst), riscv::CpuStatus::UNSUPPORTED_INSTRUCTION);

    // All ones
    ASSERT_EQ(riscv::Decoder::decode(0xFFFFFFFF, inst), riscv::CpuStatus::UNSUPPORTED_INSTRUCTION);

    // SYSTEM opcode (ECALL/EBREAK) 0x73 -> out of scope
    ASSERT_EQ(riscv::Decoder::decode(0x00000073, inst), riscv::CpuStatus::UNSUPPORTED_INSTRUCTION);

    // FENCE opcode 0x0F -> out of scope
    ASSERT_EQ(riscv::Decoder::decode(0x0000000F, inst), riscv::CpuStatus::UNSUPPORTED_INSTRUCTION);

    // M-extension (MUL) 0x02000033 -> out of scope
    ASSERT_EQ(riscv::Decoder::decode(0x02000033, inst), riscv::CpuStatus::UNSUPPORTED_INSTRUCTION);
}
