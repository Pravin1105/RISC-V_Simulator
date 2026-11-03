#ifndef RISCV_DECODER_HPP
#define RISCV_DECODER_HPP

#include "types.hpp"
#include "instruction.hpp"

namespace riscv {

class Decoder {
public:
    static CpuStatus decode(Word raw, DecodedInstruction& out);
};

} // namespace riscv

#endif // RISCV_DECODER_HPP
