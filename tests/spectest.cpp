#include <gtest/gtest.h>
#include <string>

#include "tests/common.hpp"
#include "tests/json2testcase.hpp"

#include "src/common/logger.hpp"
#include "src/compiler.hpp"

using namespace spec;

TEST(Chapter02, local) {
  JsonReader const jsonReader("tests/testcases/tmp/local.json");
  LOG_YELLOW << "Testing local.json" << std::endl;
  for (const auto &module : jsonReader.modules_) {
    Compiler compiler;
    compiler.compile("tests/testcases/tmp/" + module.moduleFileName);

    LOG_YELLOW << ConsoleYellow << "Testing module " << module.moduleFileName << std::endl;
    for (const auto &testCase : module.testCases) {
      execTest(testCase, compiler);
    }
  }
}

TEST(Chapter03, arithmetic) {
  JsonReader const jsonReader("tests/testcases/tmp/arithmetic.json");
  LOG_YELLOW << ConsoleYellow << "Testing arithmetic.json" << std::endl;
  for (const auto &module : jsonReader.modules_) {
    Compiler compiler;
    compiler.compile("tests/testcases/tmp/" + module.moduleFileName);

    LOG_YELLOW << ConsoleYellow << "Testing module " << module.moduleFileName << std::endl;
    for (const auto &testCase : module.testCases) {
      execTest(testCase, compiler);
    }
  }
}

//////////////////////////////////////////////
///< Demo: test cases with hardcoded input params

// TEST(Chapter03, arithmetic0) {
//   Compiler compiler;
//   ExecutableMemory const &execMemory = compiler.compile("chapters/Chapter03/arithmetic.0.wasm");
//   // i32.add
//   EXPECT_EQ(2U, static_cast<uint32_t>(compiler.singleCallByName<uint32_t>("add", "i(ii)", 1U, 1U)));
//   EXPECT_EQ(1U, static_cast<uint32_t>(compiler.singleCallByName<uint32_t>("add", "i(ii)", 1U, 0U)));
//   EXPECT_EQ(-2, static_cast<uint32_t>(compiler.singleCallByName<uint32_t>("add", "i(ii)", -1, -1)));
//   EXPECT_EQ(0, static_cast<uint32_t>(compiler.singleCallByName<uint32_t>("add", "i(ii)", -1, 1U)));
//   EXPECT_EQ(0x80000000, static_cast<uint32_t>(compiler.singleCallByName<uint32_t>("add", "i(ii)", 0x7fffffff, 1U)));
//   EXPECT_EQ(0x7fffffff, static_cast<uint32_t>(compiler.singleCallByName<uint32_t>("add", "i(ii)", 0x80000000, -1)));
//   EXPECT_EQ(0, static_cast<uint32_t>(compiler.singleCallByName<uint32_t>("add", "i(ii)", 0x80000000, 0x80000000)));
//   EXPECT_EQ(0x40000000, static_cast<uint32_t>(compiler.singleCallByName<uint32_t>("add", "i(ii)", 0x3fffffff, 1U)));
// }

// TEST(Chapter03, arithmetic1) {
//   Compiler compiler;
//   ExecutableMemory const &execMemory = compiler.compile("chapters/Chapter03/arithmetic.1.wasm");

//   // i64.add
//   EXPECT_EQ(2U, static_cast<uint64_t>(compiler.singleCallByName<uint64_t>("add", "I(II)", 1U, 1U)));
//   EXPECT_EQ(1U, static_cast<uint64_t>(compiler.singleCallByName<uint64_t>("add", "I(II)", 1U, 0U)));
//   EXPECT_EQ(-2, static_cast<uint64_t>(compiler.singleCallByName<uint64_t>("add", "I(II)", static_cast<uint64_t>(-1), static_cast<uint64_t>(-1))));
//   EXPECT_EQ(0, static_cast<uint64_t>(compiler.singleCallByName<uint64_t>("add", "I(II)", static_cast<uint64_t>(-1), 1U)));
//   EXPECT_EQ(0x8000000000000000,
//             static_cast<uint64_t>(compiler.singleCallByName<uint64_t>("add", "I(II)", static_cast<uint64_t>(0x7fffffffffffffff), 1U)));
//   EXPECT_EQ(0x7fffffffffffffff,
//             static_cast<uint64_t>(compiler.singleCallByName<uint64_t>("add", "I(II)", 0x8000000000000000, static_cast<uint64_t>(-1))));
//   EXPECT_EQ(0, static_cast<uint64_t>(compiler.singleCallByName<uint64_t>("add", "I(II)", 0x8000000000000000, 0x8000000000000000)));
//   EXPECT_EQ(0x4000000000000000, static_cast<uint64_t>(compiler.singleCallByName<uint64_t>("add", "I(II)", 0x3fffffffffffffff, 1U)));
// }

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}