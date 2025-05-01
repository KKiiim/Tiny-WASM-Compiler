#ifndef SRC_FRONTEND_FRONTEND_H
#define SRC_FRONTEND_FRONTEND_H

#include <cstdint>
#include <string>
#include <vector>

#include "../backend/arm64Backend.hpp"
#include "../common/ModuleInfo.hpp"
#include "../common/wasm_type.hpp"
#include "../compiler.hpp"
#include "../frontend/byteCodeReader.hpp"

#define LOGGER std::cout
#define LOGGER_END std::endl
class Frontend {
public:
  explicit Frontend(std::string const &wasmPath, Stack &stack, OperandStack &operandStack);

  ExecutableMemory startCompilation();
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
  ModuleInfo module_;

private:
  BytecodeReader br_;

private:
  Arm64Backend backend_;

  Stack &stack_;
  OperandStack &operandStack_;
};

#endif