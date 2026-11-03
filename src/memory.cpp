#include "memory.hpp"
#include <iomanip>
#include <sstream>
#include <cstring>

namespace riscv {

Memory::Memory(size_t size_bytes, Address base_address)
    : base_address_(base_address), data_(size_bytes, 0) {}

void Memory::reset() {
    std::fill(data_.begin(), data_.end(), 0);
}

bool Memory::is_in_bounds(Address addr, size_t access_size) const {
    if (addr < base_address_) {
        return false;
    }
    uint64_t offset = static_cast<uint64_t>(addr) - base_address_;
    return (offset + access_size) <= data_.size();
}

size_t Memory::to_offset(Address addr) const {
    return static_cast<size_t>(addr - base_address_);
}

bool Memory::write_bytes(Address addr, const Byte* src, size_t length) {
    if (!is_in_bounds(addr, length)) {
        return false;
    }
    std::memcpy(&data_[to_offset(addr)], src, length);
    return true;
}

CpuStatus Memory::fetch_instruction(Address pc, Word& instruction) const {
    if ((pc % 4) != 0) {
        return CpuStatus::INVALID_FETCH_ALIGNMENT;
    }
    if (!is_in_bounds(pc, 4)) {
        return CpuStatus::INVALID_FETCH_OUT_OF_BOUNDS;
    }
    size_t off = to_offset(pc);
    instruction = static_cast<Word>(data_[off]) |
                  (static_cast<Word>(data_[off + 1]) << 8) |
                  (static_cast<Word>(data_[off + 2]) << 16) |
                  (static_cast<Word>(data_[off + 3]) << 24);
    return CpuStatus::RUNNING;
}

CpuStatus Memory::read_byte(Address addr, Byte& out) const {
    if (!is_in_bounds(addr, 1)) {
        return CpuStatus::INVALID_MEMORY_OUT_OF_BOUNDS;
    }
    out = data_[to_offset(addr)];
    return CpuStatus::RUNNING;
}

CpuStatus Memory::read_half(Address addr, HalfWord& out) const {
    if ((addr % 2) != 0) {
        return CpuStatus::INVALID_MEMORY_ALIGNMENT;
    }
    if (!is_in_bounds(addr, 2)) {
        return CpuStatus::INVALID_MEMORY_OUT_OF_BOUNDS;
    }
    size_t off = to_offset(addr);
    out = static_cast<HalfWord>(data_[off]) |
          (static_cast<HalfWord>(data_[off + 1]) << 8);
    return CpuStatus::RUNNING;
}

CpuStatus Memory::read_word(Address addr, Word& out) const {
    if ((addr % 4) != 0) {
        return CpuStatus::INVALID_MEMORY_ALIGNMENT;
    }
    if (!is_in_bounds(addr, 4)) {
        return CpuStatus::INVALID_MEMORY_OUT_OF_BOUNDS;
    }
    size_t off = to_offset(addr);
    out = static_cast<Word>(data_[off]) |
          (static_cast<Word>(data_[off + 1]) << 8) |
          (static_cast<Word>(data_[off + 2]) << 16) |
          (static_cast<Word>(data_[off + 3]) << 24);
    return CpuStatus::RUNNING;
}

CpuStatus Memory::write_byte(Address addr, Byte val) {
    if (!is_in_bounds(addr, 1)) {
        return CpuStatus::INVALID_MEMORY_OUT_OF_BOUNDS;
    }
    data_[to_offset(addr)] = val;
    return CpuStatus::RUNNING;
}

CpuStatus Memory::write_half(Address addr, HalfWord val) {
    if ((addr % 2) != 0) {
        return CpuStatus::INVALID_MEMORY_ALIGNMENT;
    }
    if (!is_in_bounds(addr, 2)) {
        return CpuStatus::INVALID_MEMORY_OUT_OF_BOUNDS;
    }
    size_t off = to_offset(addr);
    data_[off]     = static_cast<Byte>(val & 0xFF);
    data_[off + 1] = static_cast<Byte>((val >> 8) & 0xFF);
    return CpuStatus::RUNNING;
}

CpuStatus Memory::write_word(Address addr, Word val) {
    if ((addr % 4) != 0) {
        return CpuStatus::INVALID_MEMORY_ALIGNMENT;
    }
    if (!is_in_bounds(addr, 4)) {
        return CpuStatus::INVALID_MEMORY_OUT_OF_BOUNDS;
    }
    size_t off = to_offset(addr);
    data_[off]     = static_cast<Byte>(val & 0xFF);
    data_[off + 1] = static_cast<Byte>((val >> 8) & 0xFF);
    data_[off + 2] = static_cast<Byte>((val >> 16) & 0xFF);
    data_[off + 3] = static_cast<Byte>((val >> 24) & 0xFF);
    return CpuStatus::RUNNING;
}

std::string Memory::dump_range(Address start, size_t length) const {
    std::ostringstream oss;
    for (size_t i = 0; i < length; ++i) {
        Address cur = start + static_cast<Address>(i);
        if (i % 16 == 0) {
            if (i > 0) oss << "\n";
            oss << "0x" << std::hex << std::setw(8) << std::setfill('0') << cur << ": ";
        }
        if (is_in_bounds(cur, 1)) {
            oss << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(data_[to_offset(cur)]) << " ";
        } else {
            oss << "?? ";
        }
    }
    oss << "\n";
    return oss.str();
}

} // namespace riscv
