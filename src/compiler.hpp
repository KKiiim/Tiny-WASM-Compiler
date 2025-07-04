#ifndef SRC_COMPILER_H
#define SRC_COMPILER_H

#include <cstdint>
#include <string>

#include "src/common/ExecutableMemory.hpp"
#include "src/common/stack.hpp"
#include "src/frontend/frontend.hpp"

class Compiler final {
public:
  explicit Compiler() : frontend_(module_, stack_){};

  ExecutableMemory &compile(std::string const &wasmPath);

  /// @brief R0~R6 for passing parameters to the wasm function, R7 is the called export function index.
  template <typename TRet, typename... Args> TRet singleCallWithIndex(Args &&...args) {
    using FuncPtr = TRet (*)(typename std::remove_reference_t<Args>...);
    auto *func = bit_cast<FuncPtr>(frontend_.nativeToJitWrapper);
    return func(std::forward<Args>(args)...);
  }
  inline ModuleInfo const &getModuleInfo() const {
    return module_;
  }

private:
  Stack stack_; ///< Compile time stack

  ModuleInfo module_;
  Frontend frontend_;
};

#endif