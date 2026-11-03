cmake_minimum_required(VERSION 3.12)
project(RV32I_Simulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Simulator Library
add_library(riscv_core STATIC
    src/register_file.cpp
    src/memory.cpp
    src/immediate.cpp
    src/decoder.cpp
    src/alu.cpp
    src/cpu.cpp
    src/program_loader.cpp
)

target_include_directories(riscv_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Simulator Executable
add_executable(rv32i_sim src/main.cpp)
target_link_libraries(rv32i_sim PRIVATE riscv_core)

# Tests
enable_testing()

add_executable(test_runner
    tests/test_main.cpp
    tests/test_register_file.cpp
    tests/test_memory.cpp
    tests/test_immediate.cpp
    tests/test_decoder.cpp
    tests/test_alu.cpp
    tests/test_instructions.cpp
    tests/test_programs.cpp
)
target_link_libraries(test_runner PRIVATE riscv_core)

add_test(NAME unit_and_instruction_tests COMMAND test_runner)
