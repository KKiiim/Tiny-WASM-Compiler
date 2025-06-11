#include <gtest/gtest.h>

#include "common.hpp"

#include "src/common/logger.hpp"

namespace spec {

void execTest(TestCase const &testCase, Compiler &compiler) {
  confirm(testCase.commandType == Type::assert_return, "Expected command type to be assert_return");
  confirm(testCase.action.actionType == Type::invoke, "Expected action type to be invoke");

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
    confirm(false, "Expected only i32/i64 return value for now");
  }
  LOG_GREEN << "Test case passed: " << testCase.action.functionName << " with signature: " << signature << LOG_END;
}

} // namespace spec