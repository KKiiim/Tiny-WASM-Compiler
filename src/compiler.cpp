#include "compiler.hpp"

#include "src/common/ExecutableMemory.hpp"
#include "src/frontend/frontend.hpp"

ExecutableMemory &Compiler::compile(std::string const &wasmPath) {
  executableMemory_ = frontend_.startCompilation(wasmPath);
  return executableMemory_;
}