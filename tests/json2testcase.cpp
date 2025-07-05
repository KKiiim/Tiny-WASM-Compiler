#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>

#include "json2testcase.hpp"

#include "src/common/logger.hpp"

namespace spec {

Param::Param(nlohmann::json const &paramJson) {
  // only support i32 and i64 for now
  type = paramJson["type"].get<std::string>() == "i32" ? Type::i32 : Type::i64;
  uint64_t v{};

  ///< May not contain value becase of the assert_trap command
  if (paramJson.contains("value")) {
    std::istringstream ss(paramJson["value"].get<std::string>());
    ss >> v;
  }
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
  LOG_INFO << "Parse: " << sourceFilename << LOG_END;

  for (const auto &command : jsonData_["commands"]) {
    std::string const commandType = command["type"].get<std::string>();
    if (commandType == "module") {
      TestModule module;
      module.moduleFileName = command["filename"].get<std::string>();
      modules_.push_back(std::move(module));
    } else if ((commandType == "assert_return") || (commandType == "assert_trap") || (commandType == "assert_exhaustion")) {
      TestCase testcase;
      testcase.line = command["line"].get<uint32_t>();

      ///< parse action
      Action action;
      // TODO(): support more action types
      action.actionType = command["action"]["type"].get<std::string>() == "invoke" ? Type::invoke : Type::NONE;
      action.functionName = command["action"]["field"].get<std::string>();
      confirm(command["action"]["args"].is_array(), "Expected action args to be an array");
      for (const auto &arg : command["action"]["args"]) {
        Param const param(arg);
        action.args.push_back(param);
      }

      ///< parse expected
      Expected expectedParam;
      confirm(command["expected"].is_array(), "Expected expected to be an array");
      for (const auto &expected : command["expected"]) {
        Param const param(expected);
        expectedParam.ret.push_back(param);
      }
      confirm(((expectedParam.ret.size() == 0U) || (expectedParam.ret.size() == 1U)), "Expected only one return value for now");

      if (commandType == "assert_return") {
        testcase.commandType = Type::assert_return;
      } else { // assert_trap / assert_exhaustion
        testcase.commandType = Type::assert_trap;
        confirm(command.contains("text"), "assert_trap / assert_trap should has text");
        confirm(command["text"].is_string(), "Expected text to be string");
        testcase.text = command["text"].get<std::string>();
      }

      ///< set parsed sections to testcase
      testcase.action = std::move(action);
      testcase.expected = std::move(expectedParam);
      modules_[modules_.size() - 1].testCases.push_back(std::move(testcase));
    } else {
      LOG_ERROR << "Unknown command type: " << commandType << LOG_END;
      continue; // Skip unknown command types
    }
  }
}

void JsonReader::dump() const {
  static std::array<std::string, static_cast<size_t>(Type::NONE) + 1U> typeNames = {
      "module", "assert_return", "assert_trap", "assert_exhaustion", "invoke", "i32", "i64", "NONE",
  };
  for (const auto &module : modules_) {
    LOG_INFO << "Module: " << module.moduleFileName << LOG_END;
    for (const auto &testCase : module.testCases) {
      LOG_INFO << "  Test Case Type: " << typeNames[static_cast<size_t>(testCase.commandType)] << LOG_END;
      LOG_INFO << "  Test Case Line: " << testCase.line << LOG_END;
      LOG_INFO << "  Action Type: " << typeNames[static_cast<size_t>(testCase.action.actionType)] << ", Function: " << testCase.action.functionName
               << LOG_END;

      if (testCase.action.args.size() == 0) {
        LOG_INFO << "    No Args" << LOG_END;
      }
      for (const auto &arg : testCase.action.args) {
        LOG_INFO << "    Arg Type: " << typeNames[static_cast<size_t>(arg.type)] << ", Value: " << (arg.type == Type::i32 ? arg.value32 : arg.value64)
                 << LOG_END;
      }

      if (testCase.expected.ret.size() == 0) {
        LOG_INFO << "    No Expected Params" << LOG_END;
      }
      for (const auto &expected : testCase.expected.ret) {
        LOG_INFO << "  Expected Type: " << typeNames[static_cast<size_t>(expected.type)]
                 << ", Value: " << (expected.type == Type::i32 ? expected.value32 : expected.value64) << LOG_END;
      }

      LOG_INFO << LOG_END;
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
  confirm(params.size() >= action.args.size(), "Params array size must be at least as large as action args size");
  for (uint32_t i = 0; i < action.args.size(); i++) {
    params[i] = (action.args[i].type == Type::i32) ? static_cast<uint64_t>(action.args[i].value32) : static_cast<uint64_t>(action.args[i].value64);
  }
}

} // namespace spec