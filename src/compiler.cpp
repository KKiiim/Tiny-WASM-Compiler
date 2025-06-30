#include "compiler.hpp"

#include "src/common/ExecutableMemory.hpp"
#include "src/frontend/frontend.hpp"

ExecutableMemory &Compiler::compile(std::string const &wasmPath) {
  executableMemory_ = frontend_.startCompilation(wasmPath);
  prepareFuncIndexToAddress();
  return executableMemory_;
}

void Compiler::prepareFuncIndexToAddress() {
  for (uint32_t i = 0; i < module_.functionInfos_.size(); ++i) {
    uint32_t const toCodeStartAddressOffset = frontend_.getFunctionStartAddress(i);
    funcIndexToCodeStartOffset_.set(toCodeStartAddressOffset);
  }
}