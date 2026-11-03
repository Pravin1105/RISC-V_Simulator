#include "test_framework.hpp"
#include "memory.hpp"

TEST_CASE(Memory_ByteReadWrite) {
    riscv::Memory mem(1024, 0x1000);
    riscv::Byte b = 0;

    ASSERT_EQ(mem.write_byte(0x1000, 0xAB), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(mem.read_byte(0x1000, b), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(b, 0xAB);

    ASSERT_EQ(mem.write_byte(0x1001, 0xCD), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(mem.read_byte(0x1001, b), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(b, 0xCD);
}

TEST_CASE(Memory_EndiannessAndWidths) {
    riscv::Memory mem(1024, 0x0000);
    // Write 32-bit word 0x12345678
    ASSERT_EQ(mem.write_word(0x100, 0x12345678), riscv::CpuStatus::RUNNING);

    // Verify little-endian byte ordering
    riscv::Byte b0 = 0, b1 = 0, b2 = 0, b3 = 0;
    ASSERT_EQ(mem.read_byte(0x100, b0), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(mem.read_byte(0x101, b1), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(mem.read_byte(0x102, b2), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(mem.read_byte(0x103, b3), riscv::CpuStatus::RUNNING);

    ASSERT_EQ(b0, 0x78);
    ASSERT_EQ(b1, 0x56);
    ASSERT_EQ(b2, 0x34);
    ASSERT_EQ(b3, 0x12);

    // Halfword reads
    riscv::HalfWord h0 = 0, h1 = 0;
    ASSERT_EQ(mem.read_half(0x100, h0), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(mem.read_half(0x102, h1), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(h0, 0x5678);
    ASSERT_EQ(h1, 0x1234);

    // Word read
    riscv::Word w = 0;
    ASSERT_EQ(mem.read_word(0x100, w), riscv::CpuStatus::RUNNING);
    ASSERT_EQ(w, 0x12345678u);
}

TEST_CASE(Memory_AlignmentChecks) {
    riscv::Memory mem(1024, 0x0000);
    riscv::HalfWord h = 0;
    riscv::Word w = 0;

    // Halfword requires 2-byte alignment
    ASSERT_EQ(mem.read_half(0x101, h), riscv::CpuStatus::INVALID_MEMORY_ALIGNMENT);
    ASSERT_EQ(mem.write_half(0x101, 0x1234), riscv::CpuStatus::INVALID_MEMORY_ALIGNMENT);

    // Word requires 4-byte alignment
    ASSERT_EQ(mem.read_word(0x101, w), riscv::CpuStatus::INVALID_MEMORY_ALIGNMENT);
    ASSERT_EQ(mem.read_word(0x102, w), riscv::CpuStatus::INVALID_MEMORY_ALIGNMENT);
    ASSERT_EQ(mem.read_word(0x103, w), riscv::CpuStatus::INVALID_MEMORY_ALIGNMENT);
    ASSERT_EQ(mem.write_word(0x101, 0x12345678), riscv::CpuStatus::INVALID_MEMORY_ALIGNMENT);
}

TEST_CASE(Memory_InstructionFetchChecks) {
    riscv::Memory mem(1024, 0x0000);
    riscv::Word inst = 0;

    // Fetch aligned
    ASSERT_EQ(mem.fetch_instruction(0x00, inst), riscv::CpuStatus::RUNNING);

    // Fetch unaligned
    ASSERT_EQ(mem.fetch_instruction(0x01, inst), riscv::CpuStatus::INVALID_FETCH_ALIGNMENT);
    ASSERT_EQ(mem.fetch_instruction(0x02, inst), riscv::CpuStatus::INVALID_FETCH_ALIGNMENT);
    ASSERT_EQ(mem.fetch_instruction(0x03, inst), riscv::CpuStatus::INVALID_FETCH_ALIGNMENT);

    // Fetch out of bounds
    ASSERT_EQ(mem.fetch_instruction(1024, inst), riscv::CpuStatus::INVALID_FETCH_OUT_OF_BOUNDS);
}

TEST_CASE(Memory_OutOfBounds) {
    riscv::Memory mem(100, 0x1000);
    riscv::Byte b = 0;
    riscv::Word w = 0;

    // Before base
    ASSERT_EQ(mem.read_byte(0x0FFF, b), riscv::CpuStatus::INVALID_MEMORY_OUT_OF_BOUNDS);
    // Past end
    ASSERT_EQ(mem.read_byte(0x1000 + 100, b), riscv::CpuStatus::INVALID_MEMORY_OUT_OF_BOUNDS);
    // Word spilling past end
    ASSERT_EQ(mem.read_word(0x1000 + 98, w), riscv::CpuStatus::INVALID_MEMORY_ALIGNMENT);
    ASSERT_EQ(mem.read_word(0x1000 + 96 + 4, w), riscv::CpuStatus::INVALID_MEMORY_OUT_OF_BOUNDS);
}
