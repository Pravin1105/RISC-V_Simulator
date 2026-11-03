#include "register_file.hpp"
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace riscv {

RegisterFile::RegisterFile() {
    reset();
}

Word RegisterFile::read(RegId reg) const {
    if (reg >= NUM_REGISTERS) {
        throw std::out_of_range("Register index out of range: " + std::to_string(static_cast<int>(reg)));
    }
    if (reg == 0) {
        return 0;
    }
    return registers_[reg];
}

void RegisterFile::write(RegId reg, Word value) {
    if (reg >= NUM_REGISTERS) {
        throw std::out_of_range("Register index out of range: " + std::to_string(static_cast<int>(reg)));
    }
    if (reg == 0) {
        // x0 is hardwired to zero, writes are discarded
        return;
    }
    registers_[reg] = value;
}

void RegisterFile::reset() {
    registers_.fill(0);
}

std::string RegisterFile::dump() const {
    std::ostringstream oss;
    for (size_t i = 0; i < NUM_REGISTERS; ++i) {
        oss << "x" << std::dec << std::setw(2) << std::setfill(' ') << i << ": 0x"
            << std::hex << std::setw(8) << std::setfill('0') << registers_[i];
        if ((i + 1) % 4 == 0) {
            oss << "\n";
        } else {
            oss << "  ";
        }
    }
    return oss.str();
}

} // namespace riscv
