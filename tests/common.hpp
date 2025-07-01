#ifndef TESTS_COMMON_HPP
#define TESTS_COMMON_HPP

#include "tests/json2testcase.hpp"

#include "src/compiler.hpp"
#include "src/runtime.hpp"

namespace spec {
void execTestModule(TestModule const &module);
void execTest(TestCase const &testCase, Runtime &runtime);
} // namespace spec

#endif // TESTS_COMMON_HPP