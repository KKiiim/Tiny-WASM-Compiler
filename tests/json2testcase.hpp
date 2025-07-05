#ifndef TESTS_JSON2TESTCASE_HPP
#define TESTS_JSON2TESTCASE_HPP

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "src/common/constant.hpp"

namespace spec {

enum class Type : uint8_t {
  // command type
  module,
  assert_return,
  assert_trap,
  assert_exhaustion, // alias for assert_trap yet for test stack overflow spec test

  // action type
  invoke,

  // param type(in args, expected)
  i32,
  i64,

  NONE,
};

class Param {
public:
  explicit Param(nlohmann::json const &paramJson);

  Type type;
  // [ref: doc-design]
  // Actually, value32 and value64 can be merged to one object. Since we always cast as u64 params for spec test
  // But for future use, such as a refactor that params serilized in memory in a call wrapper.
  uint32_t value32;
  uint64_t value64;
};

class Expected {
public:
  // Only support one return value for now
  std::vector<Param> ret;
};

class Action {
public:
  Type actionType = Type::NONE;
  std::string functionName;
  std::vector<Param> args;
};

class TestCase {
public:
  Type commandType = Type::NONE;
  uint32_t line{0U};
  Action action;
  Expected expected;
  std::string text; ///< If is assert_trap type, this is the trap message

  std::string getSignature() const;
  void setParams(std::array<uint64_t, MaxParamsForWasmFunction> &params) const;

private:
};

class TestModule {
public:
  std::vector<TestCase> testCases;

  std::string moduleFileName;
};

// one json -> multiple modules
// one module -> multiple test commands
class JsonReader {
public:
  explicit JsonReader(const std::string &jsonPath);
  void dump() const;

  std::vector<TestModule> modules_;
};

} // namespace spec

#endif // TESTS_JSON2TESTCASE_HPP