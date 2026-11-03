#include "program_loader.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace riscv {

static inline std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

static inline std::string strip_comment(const std::string& line) {
    size_t hash_pos = line.find('#');
    size_t slash_pos = line.find("//");
    size_t cut = std::min(hash_pos, slash_pos);
    if (cut != std::string::npos) {
        return line.substr(0, cut);
    }
    return line;
}

ProgramLoadResult ProgramLoader::load_hex_file(const std::string& filepath, Memory& memory, Address default_base) {
    ProgramLoadResult result;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        result.success = false;
        result.error_message = "Failed to open file: " + filepath;
        return result;
    }

    Address cur_addr = default_base;
    result.entry_point = default_base;
    size_t count = 0;
    std::string line;

    while (std::getline(file, line)) {
        line = trim(strip_comment(line));
        if (line.empty()) {
            continue;
        }

        // Check for address directive, e.g. @00001000
        if (line[0] == '@') {
            std::string addr_str = line.substr(1);
            try {
                cur_addr = static_cast<Address>(std::stoul(addr_str, nullptr, 16));
                if (count == 0) {
                    result.entry_point = cur_addr;
                }
            } catch (...) {
                result.success = false;
                result.error_message = "Invalid address directive in file: " + line;
                return result;
            }
            continue;
        }

        // Parse hex instruction word
        Word word = 0;
        try {
            word = static_cast<Word>(std::stoul(line, nullptr, 16));
        } catch (...) {
            result.success = false;
            result.error_message = "Malformed hex instruction: " + line;
            return result;
        }

        CpuStatus st = memory.write_word(cur_addr, word);
        if (st != CpuStatus::RUNNING) {
            result.success = false;
            result.error_message = "Memory write error at 0x" + std::to_string(cur_addr) + ": " + to_string(st);
            return result;
        }

        cur_addr += 4;
        count++;
    }

    result.success = true;
    result.words_loaded = count;
    result.end_address = cur_addr;
    return result;
}

ProgramLoadResult ProgramLoader::load_binary_file(const std::string& filepath, Memory& memory, Address base_address) {
    ProgramLoadResult result;
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        result.success = false;
        result.error_message = "Failed to open binary file: " + filepath;
        return result;
    }

    std::streamsize file_size = file.tellg();
    if (file_size < 0) {
        result.success = false;
        result.error_message = "Invalid file size for: " + filepath;
        return result;
    }

    file.seekg(0, std::ios::beg);
    std::vector<Byte> buffer(static_cast<size_t>(file_size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), file_size)) {
        result.success = false;
        result.error_message = "Failed to read binary data from: " + filepath;
        return result;
    }

    if (!memory.write_bytes(base_address, buffer.data(), buffer.size())) {
        result.success = false;
        result.error_message = "Memory write failed when loading binary at base address: " + std::to_string(base_address);
        return result;
    }

    result.success = true;
    result.entry_point = base_address;
    result.words_loaded = (buffer.size() + 3) / 4;
    result.end_address = base_address + static_cast<Address>(buffer.size());
    return result;
}

ProgramLoadResult ProgramLoader::load_instructions(const std::vector<Word>& instructions, Memory& memory, Address base_address) {
    ProgramLoadResult result;
    Address cur_addr = base_address;
    result.entry_point = base_address;

    for (size_t i = 0; i < instructions.size(); ++i) {
        CpuStatus st = memory.write_word(cur_addr, instructions[i]);
        if (st != CpuStatus::RUNNING) {
            result.success = false;
            result.error_message = "Failed writing instruction at index " + std::to_string(i) + " address 0x" + std::to_string(cur_addr);
            return result;
        }
        cur_addr += 4;
    }

    result.success = true;
    result.words_loaded = instructions.size();
    result.end_address = cur_addr;
    return result;
}

} // namespace riscv
