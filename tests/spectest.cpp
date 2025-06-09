#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "tests/json2testcase.hpp"

#include "src/common/constant.hpp"
#include "src/common/logger.hpp"
#include "src/compiler.hpp"

using namespace spec;
class DynamicArgs {
  std::vector<std::any> args_;

public:
  template <typename... Args> void addValues(Args &&...args) {
    (args_.emplace_back(std::forward<Args>(args)), ...);
  }

  const auto &getArgs() const {
    return args_;
  }
};

static void execTest(TestCase const &testCase, Compiler &compiler) {
  assert(testCase.commandType == Type::assert_return && "Expected command type to be assert_return");
  assert(testCase.action.actionType == Type::invoke && "Expected action type to be invoke");

  // [ref: doc-design]
  std::array<uint64_t, MaxParamsForWasmFunction> params{0, 0, 0, 0, 0, 0, 0, 0};
  testCase.setParams(params);
  auto const &signature = testCase.getSignature();
  if (testCase.expected.ret.empty()) {
    compiler.singleCallByName<void>(testCase.action.functionName, signature, params[0], params[1], params[2], params[3], params[4], params[5],
                                    params[6], params[7]);
  } else if (testCase.expected.ret[0].type == Type::i32) {
    uint32_t const ret32 = compiler.singleCallByName<uint32_t>(testCase.action.functionName, signature, params[0], params[1], params[2], params[3],
                                                               params[4], params[5], params[6], params[7]);
    EXPECT_EQ(ret32, testCase.expected.ret[0].value32);
  } else if (testCase.expected.ret[0].type == Type::i64) {
    uint64_t const ret64 = compiler.singleCallByName<uint64_t>(testCase.action.functionName, signature, params[0], params[1], params[2], params[3],
                                                               params[4], params[5], params[6], params[7]);
    EXPECT_EQ(ret64, testCase.expected.ret[0].value64);
  } else {
    assert(false && "Expected only i32/i64 return value for now");
  }
  LOG_GREEN << "Test case passed: " << testCase.action.functionName << " with signature: " << signature << std::endl;
}

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