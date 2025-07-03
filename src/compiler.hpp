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

  template <typename TRet, typename... Args> TRet singleCallByIndex(uint32_t const functionIndex, Args &&...args) {
    uintptr_t const funcStartAddress = frontend_.getFunctionStartAddress(functionIndex);
    using FuncPtr = TRet (*)(typename std::remove_reference_t<Args>...);
    auto *func = bit_cast<FuncPtr>(funcStartAddress);
    return func(std::forward<Args>(args)...);
  }
  inline ModuleInfo const &getModuleInfo() const {
    return module_;
  }
  inline uintptr_t getGlobalMemoryStartAddress() const {
    return frontend_.globalMemory.getStartAddr();
  }

private:
  Stack stack_; ///< Compile time stack

  ModuleInfo module_;
  Frontend frontend_;
};

#endif