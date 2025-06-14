#ifndef SRC_COMPILER_H
#define SRC_COMPILER_H

#include <string>

#include "src/common/ExecutableMemory.hpp"
#include "src/common/operand_stack.hpp"
#include "src/common/stack.hpp"
#include "src/frontend/frontend.hpp"

class Compiler final {
public:
  explicit Compiler() : frontend_(module_, stack_, operandStack_){};

  ExecutableMemory &compile(std::string const &wasmPath);

  template <typename T, typename... Args> T singleCallByName(std::string const &funcName, std::string const &signature, Args &&...args) {
    auto const &funcIdex = module_.exportFuncNameToIndex_.find(funcName);
    if (funcIdex == module_.exportFuncNameToIndex_.end()) {
      throw std::runtime_error("Function not found: " + funcName);
    }
    uint32_t const functionIndex = funcIdex->second;
    if (!module_.validateSignature(functionIndex, signature)) {
      throw std::runtime_error("Signature validation failed for function: " + funcName);
    }
    uint32_t const startAddressOffset = module_.functionInfos_[functionIndex].startAddressOffset;
    const uint32_t *const execStart = executableMemory_.data<uint32_t *>();
    const uint32_t *const funcStartAddress = execStart + (startAddressOffset / sizeof(uint32_t));

    using FuncPtr = T (*)(typename std::remove_reference_t<Args>...);
    auto *func = bit_cast<FuncPtr>(funcStartAddress);
    initOperandStack();
    return func(std::forward<Args>(args)...);
  }

private:
  void initOperandStack();

private:
  Stack stack_;               ///< Compiler stack
  OperandStack operandStack_; ///< JIT runtime stack for simulate WASM operand stack

  ModuleInfo module_;
  Frontend frontend_;

private:
  ExecutableMemory executableMemory_; ///< Executable memory for compiled code
};

#endif