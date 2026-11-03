#ifndef RISCV_PROGRAM_LOADER_HPP
#define RISCV_PROGRAM_LOADER_HPP

#include "types.hpp"
#include "memory.hpp"
#include <string>
#include <vector>

namespace riscv {

struct ProgramLoadResult {
    bool success{false};
    Address entry_point{0};
    Address end_address{0};
    size_t words_loaded{0};
    std::string error_message;
};

class ProgramLoader {
public:
    // Load from a text file containing hexadecimal instruction words (one per line, e.g. 0x00500113 or 00500113)
    // Supports comments starting with '#' or '//' and optional '@<hex_address>' address relocation directives.
    static ProgramLoadResult load_hex_file(const std::string& filepath, Memory& memory, Address default_base = 0x00000000);

    // Load from raw binary file
    static ProgramLoadResult load_binary_file(const std::string& filepath, Memory& memory, Address base_address = 0x00000000);

    // Load from an in-memory vector of 32-bit instruction words
    static ProgramLoadResult load_instructions(const std::vector<Word>& instructions, Memory& memory, Address base_address = 0x00000000);
};

} // namespace riscv

#endif // RISCV_PROGRAM_LOADER_HPP
