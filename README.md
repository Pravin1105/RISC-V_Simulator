# RV32I RISC-V Processor Simulator

A modular, deterministic, single-cycle RV32I RISC-V processor simulator in C++17 implementing the exact 37-instruction subset specified in Section 3 of the PRD.

## Supported Instructions (Exactly 37)

| Category | Instructions |
|---|---|
| Arithmetic | `ADD`, `SUB`, `ADDI` |
| Logical | `AND`, `OR`, `XOR`, `ANDI`, `ORI`, `XORI` |
| Shifts | `SLL`, `SRL`, `SRA`, `SLLI`, `SRLI`, `SRAI` |
| Comparison | `SLT`, `SLTU`, `SLTI`, `SLTIU` |
| Memory | `LB`, `LH`, `LW`, `LBU`, `LHU`, `SB`, `SH`, `SW` |
| Branch | `BEQ`, `BNE`, `BLT`, `BGE`, `BLTU`, `BGEU` |
| Jump | `JAL`, `JALR` |
| Upper Immediate | `LUI`, `AUIPC` |

*Out-of-scope opcodes (M, A, F, D, C, CSR, system, fence) are rejected with clear diagnostic status codes (`UNSUPPORTED_INSTRUCTION` or `MALFORMED_INSTRUCTION`).*

## Building and Running Tests

### Using Make:
```bash
make
make test
```

### Using CMake:
```bash
mkdir build && cd build
cmake ..
cmake --build .
ctest --output-on-failure
```

## Running the Simulator

```bash
./rv32i_sim [options] <program_file>
```

### Options:
- `--binary`: Load file as raw binary machine code (default: hex text format).
- `--base <addr>`: Set base/load address (default: `0x00000000`).
- `--max-inst <N>`: Maximum instructions to execute before halting (default: `1000000`).
- `--dump-regs`: Print register state and CPU diagnostic summary upon halting.
- `--dump-mem <addr> <length>`: Dump memory region in hex upon halting.

### Hex Input Format:
Hex text files contain one 32-bit hex instruction word per line:
```text
# Comments start with '#' or '//'
00a00093 # ADDI x1, x0, 10
00000113 # ADDI x2, x0, 0
00110133 # ADD  x2, x2, x1
fff08093 # ADDI x1, x1, -1
fe009ce3 # BNE  x1, x0, -8
```
Optional `@<address>` directives can be used to relocate code to a specific byte address.

## Implementation Details

All architecture details and step-by-step logs are tracked in [`memory.md`](memory.md).
