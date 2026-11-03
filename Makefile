CXX ?= clang++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -Werror -O2 -Iinclude

SRCS = src/register_file.cpp \
       src/memory.cpp \
       src/immediate.cpp \
       src/decoder.cpp \
       src/alu.cpp \
       src/cpu.cpp \
       src/program_loader.cpp

OBJS = $(SRCS:.cpp=.o)

TEST_SRCS = tests/test_main.cpp \
            tests/test_register_file.cpp \
            tests/test_memory.cpp \
            tests/test_immediate.cpp \
            tests/test_decoder.cpp \
            tests/test_alu.cpp \
            tests/test_instructions.cpp \
            tests/test_programs.cpp

TEST_OBJS = $(TEST_SRCS:.cpp=.o)

TARGET = rv32i_sim
TEST_TARGET = test_runner

.PHONY: all clean test

all: $(TARGET) $(TEST_TARGET)

$(TARGET): $(OBJS) src/main.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_TARGET): $(OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(OBJS) src/main.o $(TEST_OBJS) $(TARGET) $(TEST_TARGET)
