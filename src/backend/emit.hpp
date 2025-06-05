#ifndef SRC_BACKEND_EMIT_H
#define SRC_BACKEND_EMIT_H

// wasm to aarch64
#include <cstdint>
#include <cstdlib>

#include "aarch64_encoding.hpp"

#include "src/common/ExecutableMemory.hpp"
#include "src/common/wasm_type.hpp"

class Emit {
public:
  Emit();
  ~Emit();

  Emit(Emit const &) = delete;
  Emit &operator=(Emit const &) = delete;
  Emit(Emit &&) = delete;
  Emit &operator=(Emit &&) = delete;

  void append(OPCodeTemplate const ins);
  ExecutableMemory getExecutableMemory();
  uint32_t getCurrentOffset() const {
    return size_;
  }

  void emit_mov_x_imm64(REG const destReg, uint64_t const imm);
  void emit_mov_w_imm32(REG const destReg, uint32_t const imm);
  void decreaseSPWithClean(uint32_t const bytes);

private:
  uint8_t *data_;
  uint32_t size_;
};

#endif