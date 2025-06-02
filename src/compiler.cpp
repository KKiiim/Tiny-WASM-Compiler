#include "backend/emit.hpp"
#include "common/ExecutableMemory.hpp"
#include "compiler.hpp"
#include "frontend/frontend.hpp"

ExecutableMemory &Compiler::compile(std::string const &wasmPath) {
  executableMemory_ = frontend_.startCompilation(wasmPath);
  return executableMemory_;
}

void Compiler::initRuntime() {
  Emit emit{}; // temporary emitter
  emit.emit_mov_r_imm64(REG::R28, operandStack_.getStartAddr());
  OPCodeTemplate const insRET = 0xd65f03c0; // big endian
  emit.append(insRET);
  ExecutableMemory const exec = emit.getExecutableMemory();
  void (*const init)() = exec.data<void (*)()>();
  // FIXME: Maybe dangerous, since we don't know what happened between init and first function call
  // May influence the R28 ?
  init();
}