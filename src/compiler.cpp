#include "compiler.hpp"
#include "frontend/frontend.hpp"

ExecutableMemory Compiler::compile(std::string const &wasmPath) {
  Frontend frontend{wasmPath, stack_, operandStack_};
  return frontend.startCompilation();
}

void Compiler::initRuntime() {
  // LDR r28, &operandStack_
}