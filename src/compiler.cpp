#include "compiler.hpp"

#include "src/backend/aarch64Assembler.hpp"
#include "src/common/ExecutableMemory.hpp"
#include "src/frontend/frontend.hpp"

ExecutableMemory &Compiler::compile(std::string const &wasmPath) {
  executableMemory_ = frontend_.startCompilation(wasmPath);
  return executableMemory_;
}

void Compiler::initRuntime() {
  Assembler as{}; // temporary emitter
  as.emit_mov_x_imm64(REG::R28, operandStack_.getStartAddr());
  as.ret();
  ExecutableMemory const exec = as.getExecutableMemory();
  void (*const init)() = exec.data<void (*)()>();
  // FIXME: Maybe dangerous, since we don't know what happened between init and first function call
  // May influence the R28 ?
  init();
}