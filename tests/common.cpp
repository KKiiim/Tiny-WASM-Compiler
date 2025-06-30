#include <cstdint>
#include <gtest/gtest.h>

#include "common.hpp"
#include "tests/json2testcase.hpp"

#include "src/common/constant.hpp"
#include "src/common/logger.hpp"
#include "src/runtime.hpp"

namespace spec {

void execTest(TestCase const &testCase, Compiler &compiler) {
  confirm(((testCase.commandType == Type::assert_return) || (testCase.commandType == Type::assert_trap)), "not supported command type");
  confirm(testCase.action.actionType == Type::invoke, "Expected action type to be invoke");

  // [ref: doc-design]
  std::array<uint64_t, MaxParamsForWasmFunction> params{0, 0, 0, 0, 0, 0, 0, 0};
  testCase.setParams(params);
  auto const &signature = testCase.getSignature();

  Runtime runtime{compiler};
  runtime.initialize();

  // TODO(): compare the trap message and the assert_trap text
  Runtime::CallReturn ret{};
  if (testCase.expected.ret.empty()) {
    ret = runtime.callByName<void>(testCase.action.functionName, signature, params[0], params[1], params[2], params[3], params[4], params[5],
                                   params[6], params[7]);
    if (testCase.commandType == Type::assert_trap) {
      EXPECT_TRUE(ret.hasTrapped);
    }
  } else if (testCase.expected.ret[0].type == Type::i32) {
    ret = runtime.callByName<uint32_t>(testCase.action.functionName, signature, params[0], params[1], params[2], params[3], params[4], params[5],
                                       params[6], params[7]);
    if (testCase.commandType == Type::assert_trap) {
      EXPECT_TRUE(ret.hasTrapped);
    } else {
      EXPECT_EQ(static_cast<uint32_t>(ret.returnValue), testCase.expected.ret[0].value32);
    }
  } else if (testCase.expected.ret[0].type == Type::i64) {
    ret = runtime.callByName<uint64_t>(testCase.action.functionName, signature, params[0], params[1], params[2], params[3], params[4], params[5],
                                       params[6], params[7]);
    if (testCase.commandType == Type::assert_trap) {
      EXPECT_TRUE(ret.hasTrapped);
    } else {
      EXPECT_EQ(ret.returnValue, testCase.expected.ret[0].value64);
    }
  } else {
    confirm(false, "Expected only i32/i64 return value for now");
  }
  if (ret.hasTrapped) {
    LOG_INFO << "Exception: " << runtime.getTrapMessage() << LOG_END;
  }
  LOG_GREEN << "Test case passed: " << testCase.action.functionName << " with signature: " << signature << LOG_END;
}

} // namespace spec