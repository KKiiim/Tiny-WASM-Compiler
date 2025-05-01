#include <cassert>

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
  backend_.emit.append(ldr_ar2r(ROP, static_cast<REG>(paramIndex), is64bit));
  backend_.emit.append(add_r_r_imm(ROP, ROP, is64bit ? 4U : 8U));
}
void OP::set_r_param(uint32_t const paramIndex, bool const is64bit, bool const isTee = false) {
  if (!isTee) {
    backend_.emit.append(sub_r_r_imm(ROP, ROP, is64bit ? 4U : 8U));
  }
  backend_.emit.append(str_r2ar(static_cast<REG>(paramIndex), ROP, is64bit));
}
void OP::get_ofsp_local(uint32_t const offset2SP, bool const is64bit) {
  // Use R9 as scratch register
  backend_.emit.append(add_r_r_imm(REG::R9, REG::SP, offset2SP));
  backend_.emit.append(str_r2ar(ROP, REG::R9, is64bit));
  backend_.emit.append(add_r_r_imm(ROP, ROP, is64bit ? 4U : 8U));
}
void OP::set_ofsp_local(uint32_t const offset2SP, bool const is64bit, bool const isTee = false) {
  // Use R9 as scratch register
  if (!isTee) {
    backend_.emit.append(sub_r_r_imm(ROP, ROP, is64bit ? 4U : 8U));
  }
  backend_.emit.append(add_r_r_imm(REG::R9, REG::SP, offset2SP));
  backend_.emit.append(str_r2ar(REG::R9, ROP, is64bit));
}