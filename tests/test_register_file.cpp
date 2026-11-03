#include "test_framework.hpp"
#include "register_file.hpp"

TEST_CASE(RegisterFile_InitialStateAndZero) {
    riscv::RegisterFile rf;
    for (riscv::RegId i = 0; i < 32; ++i) {
        ASSERT_EQ(rf.read(i), 0u);
    }

    // Write to x0 should be ignored
    rf.write(0, 0x12345678);
    ASSERT_EQ(rf.read(0), 0u);
}

TEST_CASE(RegisterFile_ReadWriteAll) {
    riscv::RegisterFile rf;
    for (riscv::RegId i = 1; i < 32; ++i) {
        riscv::Word val = 0x1000u + i;
        rf.write(i, val);
        ASSERT_EQ(rf.read(i), val);
    }
    // Verify x0 still 0
    ASSERT_EQ(rf.read(0), 0u);

    // Reset clears everything
    rf.reset();
    for (riscv::RegId i = 0; i < 32; ++i) {
        ASSERT_EQ(rf.read(i), 0u);
    }
}

TEST_CASE(RegisterFile_OutOfBounds) {
    riscv::RegisterFile rf;
    bool caught_read = false;
    try {
        rf.read(32);
    } catch (const std::out_of_range&) {
        caught_read = true;
    }
    ASSERT_TRUE(caught_read);

    bool caught_write = false;
    try {
        rf.write(32, 42);
    } catch (const std::out_of_range&) {
        caught_write = true;
    }
    ASSERT_TRUE(caught_write);
}
