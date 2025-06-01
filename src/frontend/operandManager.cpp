#include <cassert>
#include <cstdint>
#include <iostream>

#include "operandManager.hpp"

uint32_t OP::add(WasmType const localType) {
  uint32_t const startOffset = size_;
  switch (localType) {
  case WasmType::I32: {
    size_ += 4U;
    break;
  }
  case WasmType::I64: {
    size_ += 8U;
    break;
  }
  case WasmType::F32:
  case WasmType::F64:
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

// TODO(): Stupid and inefficient implementation, but simple, let's do it this way first

void OP::get_r_param(uint32_t const paramIndex, bool const is64bit) {
  backend_.emit.append(ldr_simm_ar2r(ROP, static_cast<REG>(paramIndex), 0U, is64bit));
  backend_.emit.append(add_r_r_imm(ROP, ROP, is64bit ? 4U : 8U, true));
}
void OP::set_r_param(uint32_t const paramIndex, bool const is64bit, bool const isTee = false) {
  if (!isTee) {
    backend_.emit.append(sub_r_r_imm(ROP, ROP, is64bit ? 4U : 8U, true));
  }
  backend_.emit.append(str_r2ar_simm(static_cast<REG>(paramIndex), ROP, 0U, is64bit));
}
void OP::get_ofsp_local(uint32_t const offset2SP, bool const is64bit) {
  // Use R9 as scratch register
  assert(offset2SP <= MaxPositiveImmForLdrStr && "offset2SP too large");
  backend_.emit.append(ldr_simm_ar2r(REG::R9, REG::SP, static_cast<int32_t>(offset2SP & MaxPositiveImmForLdrStr), is64bit));
  backend_.emit.append(str_r2ar_simm(ROP, REG::R9, 0U, is64bit));
  backend_.emit.append(add_r_r_imm(ROP, ROP, is64bit ? 8U : 4U, true));
}
void OP::set_ofsp_local(uint32_t const offset2SP, bool const is64bit, bool const isTee = false) {
  // Use R9 as scratch register
  if (!isTee) {
    backend_.emit.append(sub_r_r_imm(ROP, ROP, is64bit ? 8U : 4U, true));
  }
  assert(offset2SP <= MaxPositiveImmForLdrStr && "offset2SP too large");
  backend_.emit.append(str_r2ar_simm(REG::SP, ROP, static_cast<int32_t>(offset2SP & MaxPositiveImmForLdrStr), is64bit));
}
void OP::drop(bool const is64bit) {
  backend_.emit.append(sub_r_r_imm(ROP, ROP, is64bit ? 8U : 4U, true));
}