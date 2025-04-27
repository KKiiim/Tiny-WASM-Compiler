#ifndef SRC_COMPILER_H
#define SRC_COMPILER_H

#include <string>

#include "common/ExecutableMemory.hpp"
#include "common/operand_stack.hpp"
#include "common/stack.hpp"

class Compiler final {
public:
  explicit Compiler() : stack_(){};

  ExecutableMemory compile(std::string const &wasmPath);
  void initRuntime();

private:
  Stack stack_;               ///< Compiler stack
  OperandStack operandStack_; ///< JIT runtime stack for simulate WASM operand stack
};

#endif