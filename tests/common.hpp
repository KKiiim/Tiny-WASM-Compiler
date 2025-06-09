#ifndef TESTS_COMMON_HPP
#define TESTS_COMMON_HPP

#include "tests/json2testcase.hpp"

#include "src/compiler.hpp"

namespace spec {
void execTest(TestCase const &testCase, Compiler &compiler);
}

#endif // TESTS_COMMON_HPP