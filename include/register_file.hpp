#ifndef RISCV_REGISTER_FILE_HPP
#define RISCV_REGISTER_FILE_HPP

#include "types.hpp"
#include <array>
#include <string>

namespace riscv {

class RegisterFile {
public:
    RegisterFile();

    Word read(RegId reg) const;
    void write(RegId reg, Word value);
    void reset();

    // Inspect registers for tests/debugging
    const std::array<Word, NUM_REGISTERS>& raw_registers() const { return registers_; }
    std::string dump() const;

private:
    std::array<Word, NUM_REGISTERS> registers_{};
};

} // namespace riscv

#endif // RISCV_REGISTER_FILE_HPP
