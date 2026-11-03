#ifndef RISCV_TEST_FRAMEWORK_HPP
#define RISCV_TEST_FRAMEWORK_HPP

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>

namespace riscv::test {

template <typename T>
inline std::string to_repr(const T& val) {
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

struct TestFailure {
    std::string test_name;
    std::string expression;
    std::string file;
    int line;
    std::string message;
};

class TestSuite {
public:
    static TestSuite& instance() {
        static TestSuite suite;
        return suite;
    }

    void register_test(const std::string& name, std::function<void()> func) {
        tests_.push_back({name, func});
    }

    void record_failure(const std::string& expr, const std::string& file, int line, const std::string& msg = "") {
        current_test_failed_ = true;
        failures_.push_back({current_test_name_, expr, file, line, msg});
    }

    bool run_all() {
        size_t passed = 0;
        size_t total = tests_.size();

        std::cout << "==========================================\n";
        std::cout << " Running " << total << " Test Cases\n";
        std::cout << "==========================================\n";

        for (const auto& t : tests_) {
            current_test_name_ = t.name;
            current_test_failed_ = false;
            try {
                t.func();
            } catch (const std::exception& e) {
                record_failure("Uncaught Exception", __FILE__, __LINE__, e.what());
            } catch (...) {
                record_failure("Uncaught Exception", __FILE__, __LINE__, "Unknown non-std exception");
            }

            if (!current_test_failed_) {
                passed++;
                std::cout << " [PASS] " << t.name << "\n";
            } else {
                std::cout << " [FAIL] " << t.name << "\n";
            }
        }

        std::cout << "==========================================\n";
        std::cout << " Summary: " << passed << " / " << total << " tests passed.\n";

        if (!failures_.empty()) {
            std::cout << "\n Failures:\n";
            for (const auto& f : failures_) {
                std::cout << "  - [" << f.test_name << "] " << f.file << ":" << f.line
                          << " -> " << f.expression;
                if (!f.message.empty()) {
                    std::cout << " (" << f.message << ")";
                }
                std::cout << "\n";
            }
            std::cout << "==========================================\n";
            return false;
        }

        std::cout << " All tests passed successfully!\n";
        std::cout << "==========================================\n";
        return true;
    }

private:
    struct TestEntry {
        std::string name;
        std::function<void()> func;
    };

    std::vector<TestEntry> tests_;
    std::vector<TestFailure> failures_;
    std::string current_test_name_;
    bool current_test_failed_{false};
};

struct TestRegistrar {
    TestRegistrar(const std::string& name, std::function<void()> func) {
        TestSuite::instance().register_test(name, func);
    }
};

} // namespace riscv::test

#define TEST_CASE(name) \
    static void name(); \
    static ::riscv::test::TestRegistrar registrar_##name(#name, name); \
    static void name()

#define ASSERT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            ::riscv::test::TestSuite::instance().record_failure(#cond, __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

#define ASSERT_FALSE(cond) \
    do { \
        if ((cond)) { \
            ::riscv::test::TestSuite::instance().record_failure("!(" #cond ")", __FILE__, __LINE__); \
            return; \
        } \
    } while (0)

#define ASSERT_EQ(actual, expected) \
    do { \
        auto a = (actual); \
        auto e = (expected); \
        if (a != e) { \
            ::riscv::test::TestSuite::instance().record_failure( \
                #actual " == " #expected, __FILE__, __LINE__, \
                "Actual: " + ::riscv::test::to_repr(a) + ", Expected: " + ::riscv::test::to_repr(e)); \
            return; \
        } \
    } while (0)

#endif // RISCV_TEST_FRAMEWORK_HPP
