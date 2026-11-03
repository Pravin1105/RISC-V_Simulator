#include "test_framework.hpp"

int main() {
    bool ok = riscv::test::TestSuite::instance().run_all();
    return ok ? 0 : 1;
}
