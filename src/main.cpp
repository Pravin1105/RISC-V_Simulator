#include "cpu.hpp"
#include "program_loader.hpp"
#include <iostream>
#include <string>
#include <vector>

void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " [options] <program_file>\n"
              << "Options:\n"
              << "  --binary                 Load file as raw binary (default: hex text)\n"
              << "  --base <addr>            Base/load address in hex or dec (default: 0x0)\n"
              << "  --max-inst <count>       Maximum instructions to execute (default: 1000000)\n"
              << "  --dump-regs              Print register contents upon termination\n"
              << "  --dump-mem <addr> <len>  Dump memory region (hex)\n"
              << "  --help                   Show this help message\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string filepath;
    bool is_binary = false;
    riscv::Address base_addr = 0x00000000;
    uint64_t max_inst = 1000000;
    bool dump_regs = false;
    bool dump_mem = false;
    riscv::Address mem_dump_addr = 0;
    size_t mem_dump_len = 0;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--binary") {
            is_binary = true;
        } else if (arg == "--dump-regs") {
            dump_regs = true;
        } else if (arg == "--base" && i + 1 < argc) {
            base_addr = static_cast<riscv::Address>(std::stoul(argv[++i], nullptr, 0));
        } else if (arg == "--max-inst" && i + 1 < argc) {
            max_inst = std::stoull(argv[++i]);
        } else if (arg == "--dump-mem" && i + 2 < argc) {
            dump_mem = true;
            mem_dump_addr = static_cast<riscv::Address>(std::stoul(argv[++i], nullptr, 0));
            mem_dump_len = static_cast<size_t>(std::stoul(argv[++i], nullptr, 0));
        } else if (arg[0] != '-') {
            filepath = arg;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    if (filepath.empty()) {
        std::cerr << "Error: No program file specified.\n";
        return 1;
    }

    riscv::Memory memory(1024 * 1024, base_addr); // 1 MB memory
    riscv::ProgramLoadResult load_res;

    if (is_binary) {
        load_res = riscv::ProgramLoader::load_binary_file(filepath, memory, base_addr);
    } else {
        load_res = riscv::ProgramLoader::load_hex_file(filepath, memory, base_addr);
    }

    if (!load_res.success) {
        std::cerr << "Error loading program: " << load_res.error_message << "\n";
        return 1;
    }

    std::cout << "Loaded " << load_res.words_loaded << " words. Entry: 0x"
              << std::hex << load_res.entry_point << ", End: 0x" << load_res.end_address << "\n";

    riscv::CpuConfig config;
    config.reset_vector = load_res.entry_point;
    config.program_end_address = load_res.end_address;
    config.max_instructions = max_inst;

    riscv::Cpu cpu(memory, config);
    riscv::CpuStatus final_status = cpu.run(max_inst);

    std::cout << "\nExecution finished with status: " << riscv::to_string(final_status) << "\n";
    std::cout << "Executed " << std::dec << cpu.instruction_count() << " instructions in "
              << cpu.cycle_count() << " cycles. Final PC: 0x"
              << std::hex << cpu.pc() << "\n\n";

    if (dump_regs) {
        std::cout << cpu.dump_state() << "\n";
    }

    if (dump_mem) {
        std::cout << "Memory dump [0x" << std::hex << mem_dump_addr << " - 0x"
                  << (mem_dump_addr + mem_dump_len) << "]:\n";
        std::cout << memory.dump_range(mem_dump_addr, mem_dump_len) << "\n";
    }

    if (final_status == riscv::CpuStatus::HALTED_END_OF_PROGRAM ||
        final_status == riscv::CpuStatus::MAX_INSTRUCTIONS_REACHED) {
        return 0;
    }

    return 2;
}
