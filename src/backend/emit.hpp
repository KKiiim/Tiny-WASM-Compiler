#ifndef SRC_BACKEND_EMIT_H
#define SRC_BACKEND_EMIT_H

// wasm to aarch64
#include <cstdint>
#include <cstdlib>

#include "../common/ExecutableMemory.hpp"
#include "../common/wasm_type.hpp"
#include "aarch64_encoding.hpp"

class Emit {
public:
  Emit();
  ~Emit();

  void append(OPCodeTemplate const ins);
  ExecutableMemory getExecutableMemory();
  uint32_t getCurrentOffset() const {
    return size_;
  }

  void emit_mov_r_imm64(REG const destReg, uint64_t const imm);

private:
  uint8_t *data_;
  uint32_t size_;
};

#endif