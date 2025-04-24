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

private:
  uint32_t const DefaultPageSize = static_cast<uint32_t>(1000U) * 2U;

  uint8_t *data_;
  uint32_t size_;
};

#endif