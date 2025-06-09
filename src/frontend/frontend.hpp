#ifndef SRC_FRONTEND_FRONTEND_H
#define SRC_FRONTEND_FRONTEND_H

#include <cstdint>
#include <string>
#include <vector>

#include "src/backend/arm64Backend.hpp"
#include "src/common/ModuleInfo.hpp"
#include "src/common/logger.hpp"
#include "src/common/operand_stack.hpp"
#include "src/common/stack.hpp"
#include "src/common/wasm_type.hpp"
#include "src/frontend/byteCodeReader.hpp"

class Frontend {
public:
  explicit Frontend(ModuleInfo &module, Stack &stack, OperandStack &operandStack) : module_(module), stack_(stack), operandStack_(operandStack) {
  }

  ExecutableMemory startCompilation(std::string const &wasmPath);
  void logParsedInfo();

private:
  void validateMagicNumber();
  void validateVersion();
  void parseTypeSection();
  void parseFunctionSection();
  void parseExportSection();
  void parseCodeSection();
  void parseNameSection();

  void compile();

private:
  ModuleInfo &module_;

private:
  BytecodeReader br_;

private:
  Arm64Backend backend_;

  Stack &stack_;
  OperandStack &operandStack_;
};

#endif