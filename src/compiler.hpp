#ifndef SRC_COMPILER_H
#define SRC_COMPILER_H

#include <string>

#include "src/common/ExecutableMemory.hpp"
#include "src/common/stack.hpp"
#include "src/frontend/frontend.hpp"
#include "src/frontend/runtimeBlock.hpp"

class Runtime;
class Compiler final {
  friend Runtime;

public:
  explicit Compiler() : frontend_(module_, stack_, operandStack_, funcIndexToSignatureIndex_){};

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
    uint32_t const startAddressOffset = frontend_.getFunctionStartAddress(functionIndex);
    const uint32_t *const execStart = executableMemory_.data<uint32_t *>();
    const uint32_t *const funcStartAddress = execStart + (startAddressOffset / sizeof(uint32_t));

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

  ExecutableMemory executableMemory_;      ///< Executable memory for compiled code
  RuntimeBlock operandStack_;              ///< JIT runtime stack for simulate WASM operand stack
  RuntimeBlock funcIndexToSignatureIndex_; ///< 4 bytes signatureIndex array by function index
  RuntimeBlock funcIndexToAddress_;        ///< 8 bytes functionStartAddress array by function index
};

#endif