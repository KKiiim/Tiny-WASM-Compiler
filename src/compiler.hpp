#ifndef SRC_COMPILER_H
#define SRC_COMPILER_H

#include <cstdint>
#include <string>

#include "src/common/ExecutableMemory.hpp"
#include "src/common/logger.hpp"
#include "src/common/stack.hpp"
#include "src/frontend/frontend.hpp"
#include "src/frontend/runtimeBlock.hpp"

class Runtime;
class Compiler final {
  friend Runtime;

public:
  explicit Compiler() : frontend_(module_, stack_){};

  ExecutableMemory &compile(std::string const &wasmPath);

  template <typename T, typename... Args> T singleCallByName(std::string const &funcName, std::string const &signature, Args &&...args) {
    auto const &funcIndex = module_.exportFuncNameToIndex_.find(funcName);
    if (funcIndex == module_.exportFuncNameToIndex_.end()) {
      throw std::runtime_error("Function not found: " + funcName);
    }
    uint32_t const functionIndex = funcIndex->second;
    if (!module_.validateSignature(functionIndex, signature)) {
      throw std::runtime_error("Signature validation failed for function: " + funcName);
    }
    uintptr_t const funcStartAddress = frontend_.getFunctionStartAddress(functionIndex);
    using FuncPtr = T (*)(typename std::remove_reference_t<Args>...);
    auto *func = bit_cast<FuncPtr>(funcStartAddress);
    return func(std::forward<Args>(args)...);
  }

private:
  Stack stack_; ///< Compile time stack

  ModuleInfo module_;
  Frontend frontend_;

private:
  // For runtime

  RuntimeBlock operandStack_; ///< JIT runtime stack for simulate WASM operand stack
};

#endif