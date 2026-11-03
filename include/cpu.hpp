#ifndef RISCV_CPU_HPP
#define RISCV_CPU_HPP

#include "types.hpp"
#include "register_file.hpp"
#include "memory.hpp"
#include "instruction.hpp"
#include <optional>
#include <string>

namespace riscv {

struct CpuConfig {
    Address reset_vector{DEFAULT_RESET_VECTOR};
    std::optional<Address> program_end_address{std::nullopt};
    uint64_t max_instructions{1000000};
};

class Cpu {
public:
    explicit Cpu(Memory& memory, CpuConfig config = CpuConfig{});

    void reset();

    // Single step
    CpuStatus step();

    // Run until halt or limit
    CpuStatus run(uint64_t max_instructions = 0);

    // Getters
    Address pc() const { return pc_; }
    void set_pc(Address new_pc) { pc_ = new_pc; }
    uint64_t instruction_count() const { return instruction_count_; }
    uint64_t cycle_count() const { return cycle_count_; }
    CpuStatus status() const { return status_; }
    const RegisterFile& registers() const { return reg_file_; }
    RegisterFile& registers() { return reg_file_; }
    const Memory& memory() const { return memory_; }
    Memory& memory() { return memory_; }
    const DecodedInstruction& last_instruction() const { return last_instruction_; }

    void set_program_end(Address end_addr) { config_.program_end_address = end_addr; }

    std::string dump_state() const;

private:
    Memory& memory_;
    CpuConfig config_;
    RegisterFile reg_file_;
    Address pc_{DEFAULT_RESET_VECTOR};
    uint64_t instruction_count_{0};
    uint64_t cycle_count_{0};
    CpuStatus status_{CpuStatus::RUNNING};
    DecodedInstruction last_instruction_{};
};

} // namespace riscv

#endif // RISCV_CPU_HPP
