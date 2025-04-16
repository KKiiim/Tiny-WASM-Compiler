#include "compiler.hpp"
#include "frontend/frontend.hpp"

ExecutableMemory Compiler::compile(std::string const &wasmPath) {
  Frontend frontend{wasmPath, stack_};
  return frontend.startCompilation();
}