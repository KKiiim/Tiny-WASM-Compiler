#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>

#include "json2testcase.hpp"

namespace spec {

Param::Param(nlohmann::json const &paramJson) {
  // only support i32 and i64 for now
  type = paramJson["type"].get<std::string>() == "i32" ? Type::i32 : Type::i64;
  uint64_t v;
  std::istringstream ss(paramJson["value"].get<std::string>());
  ss >> v;
  if (type == Type::i32) {
    value32 = static_cast<uint32_t>(v);
  } else if (type == Type::i64) {
    value64 = v;
  }
}

JsonReader::JsonReader(const std::string &jsonPath) {
  std::ifstream jsonFile = std::ifstream(jsonPath);
  if (!jsonFile.is_open()) {
    throw std::runtime_error("Failed to open JSON file: " + jsonPath);
  }
  auto jsonData_ = nlohmann::json::parse(jsonFile);
  std::string const sourceFilename = jsonData_["source_filename"].get<std::string>();
  std::cout << "Parse: " << sourceFilename << std::endl;

  for (const auto &command : jsonData_["commands"]) {
    std::string const commandType = command["type"].get<std::string>();
    if (commandType == "module") {
      TestModule module;
      module.moduleFileName = command["filename"].get<std::string>();
      modules_.push_back(std::move(module));
    } else if (commandType == "assert_return") {
      TestCase testcase;
      testcase.commandType = Type::assert_return;
      testcase.line = command["line"].get<uint32_t>();

      ///< parse action
      Action action;
      // TODO(): support more action types
      action.actionType = command["action"]["type"].get<std::string>() == "invoke" ? Type::invoke : Type::NONE;
      action.functionName = command["action"]["field"].get<std::string>();
      assert(command["action"]["args"].is_array() && "Expected action args to be an array");
      for (const auto &arg : command["action"]["args"]) {
        Param const param(arg);
        action.args.push_back(param);
      }

      ///< parse expected
      Expected expectedParam;
      assert(command["expected"].is_array() && "Expected expected to be an array");
      for (const auto &expected : command["expected"]) {
        Param const param(expected);
        expectedParam.ret.push_back(param);
      }
      assert(((expectedParam.ret.size() == 0U) || (expectedParam.ret.size() == 1U)) && "Expected only one return value for now");

      ///< set parsed sections to testcase
      testcase.action = std::move(action);
      testcase.expected = std::move(expectedParam);
      modules_[modules_.size() - 1].testCases.push_back(std::move(testcase));
    } else {
      std::cerr << "Unknown command type: " << commandType << std::endl;
      continue; // Skip unknown command types
    }
  }
}

void JsonReader::dump() const {
  static std::array<std::string, static_cast<size_t>(Type::NONE) + 1U> typeNames = {
      "module", "assert_return", "invoke", "i32", "i64", "NONE",
  };
  for (const auto &module : modules_) {
    std::cout << "Module: " << module.moduleFileName << std::endl;
    for (const auto &testCase : module.testCases) {
      std::cout << "  Test Case Type: " << typeNames[static_cast<size_t>(testCase.commandType)] << std::endl;
      std::cout << "  Test Case Line: " << testCase.line << std::endl;
      std::cout << "  Action Type: " << typeNames[static_cast<size_t>(testCase.action.actionType)] << ", Function: " << testCase.action.functionName
                << std::endl;

      if (testCase.action.args.size() == 0) {
        std::cout << "    No Args" << std::endl;
      }
      for (const auto &arg : testCase.action.args) {
        std::cout << "    Arg Type: " << typeNames[static_cast<size_t>(arg.type)]
                  << ", Value: " << (arg.type == Type::i32 ? arg.value32 : arg.value64) << std::endl;
      }

      if (testCase.expected.ret.size() == 0) {
        std::cout << "    No Expected Params" << std::endl;
      }
      for (const auto &expected : testCase.expected.ret) {
        std::cout << "  Expected Type: " << typeNames[static_cast<size_t>(expected.type)]
                  << ", Value: " << (expected.type == Type::i32 ? expected.value32 : expected.value64) << std::endl;
      }

      std::cout << std::endl;
    }
  }
}

std::string TestCase::getSignature() const {
  std::string format{};
  if (!expected.ret.empty()) {
    format += expected.ret[0].type == Type::i32 ? "i" : "I";
  }
  format += "(";
  for (auto const &arg : action.args) {
    format += arg.type == Type::i32 ? "i" : "I";
  }
  format += ")";
  return format;
}

void TestCase::setParams(std::array<uint64_t, MaxParamsForWasmFunction> &params) const {
  assert(params.size() >= action.args.size() && "Params array size must be at least as large as action args size");
  for (uint32_t i = 0; i < action.args.size(); i++) {
    params[i] = (action.args[i].type == Type::i32) ? static_cast<uint64_t>(action.args[i].value32) : static_cast<uint64_t>(action.args[i].value64);
  }
}

} // namespace spec