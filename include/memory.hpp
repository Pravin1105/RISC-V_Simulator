#ifndef RISCV_MEMORY_HPP
#define RISCV_MEMORY_HPP

#include "types.hpp"
#include <vector>
#include <string>

namespace riscv {

class Memory {
public:
    explicit Memory(size_t size_bytes = 1024 * 1024, Address base_address = 0x00000000);

    Address base_address() const { return base_address_; }
    size_t size() const { return data_.size(); }

    void reset();

    // Raw initialization (e.g. for loading program binaries)
    bool write_bytes(Address addr, const Byte* src, size_t length);

    // Fetch instruction
    CpuStatus fetch_instruction(Address pc, Word& instruction) const;

    // Load operations (checked)
    CpuStatus read_byte(Address addr, Byte& out) const;
    CpuStatus read_half(Address addr, HalfWord& out) const;
    CpuStatus read_word(Address addr, Word& out) const;

    // Store operations (checked)
    CpuStatus write_byte(Address addr, Byte val);
    CpuStatus write_half(Address addr, HalfWord val);
    CpuStatus write_word(Address addr, Word val);

    // Diagnostics / dump
    std::string dump_range(Address start, size_t length) const;

private:
    bool is_in_bounds(Address addr, size_t access_size) const;
    size_t to_offset(Address addr) const;

    Address base_address_;
    std::vector<Byte> data_;
};

} // namespace riscv

#endif // RISCV_MEMORY_HPP
