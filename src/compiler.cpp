#include "compiler.hpp"

#include "src/common/ExecutableMemory.hpp"
#include "src/frontend/frontend.hpp"

ExecutableMemory &Compiler::compile(std::string const &wasmPath) {
  return frontend_.startCompilation(wasmPath);
}