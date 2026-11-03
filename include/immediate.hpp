#ifndef RISCV_IMMEDIATE_HPP
#define RISCV_IMMEDIATE_HPP

#include "types.hpp"

namespace riscv {

class ImmediateGenerator {
public:
    static SignedWord extract_i(Word instruction);
    static SignedWord extract_s(Word instruction);
    static SignedWord extract_b(Word instruction);
    static Word extract_u(Word instruction);
    static SignedWord extract_j(Word instruction);
    static uint32_t extract_shamt(Word instruction);
};

} // namespace riscv

#endif // RISCV_IMMEDIATE_HPP
