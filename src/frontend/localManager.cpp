#include <cassert>

#include "localManager.hpp"

uint32_t LM::add(WasmType const localType) {
  uint32_t const startOffset = size_;
  switch (localType) {
  case WasmType::I32: {
    size_ += 4U;
    break;
  }
  case WasmType::F64: {
    size_ += 8U;
    break;
  }
  case WasmType::F32:
  case WasmType::I64:
  case WasmType::TVOID:
  case WasmType::INVALID:
  case WasmType::EXTERN_REF:
  case WasmType::FUNC_REF:
  case WasmType::VEC_TYPE:
    assert(false && "not supported");
    break;
  }

  return startOffset;
}
