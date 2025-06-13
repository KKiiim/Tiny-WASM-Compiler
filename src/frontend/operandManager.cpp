#include <cstdint>

#include "operandManager.hpp"

#include "src/common/logger.hpp"

WasmType toWasmType(OperandType const tOperand) {
  switch (tOperand) {
  case OperandType::I32:
    return WasmType::I32;
  case OperandType::I64:
    return WasmType::I64;
  default:
    return WasmType::I32;
  }
}

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
    confirm(false, "not supported");
    break;
  }

  return startOffset;
}

// TODO(): Stupid and inefficient implementation, but simple, let's do it this way first

void OP::get_r_param(uint32_t const paramIndex, bool const is64bit) {
  as_.str_base_off(ROP, static_cast<REG>(paramIndex), 0U, is64bit);
  as_.add_r_r_imm(ROP, ROP, is64bit ? 8U : 4U, true);
}
void OP::set_r_param(uint32_t const paramIndex, bool const is64bit, bool const isTee = false) {
  subROP(is64bit);
  as_.ldr_base_off(static_cast<REG>(paramIndex), ROP, 0U, is64bit);
  // TODO(): reduce ins, if tee don't sub sp, can use signed offset load(not support yet)
  if (isTee) {
    // restore the ROP
    as_.add_r_r_imm(ROP, ROP, is64bit ? 8U : 4U, true);
  }
}
void OP::get_ofsp_local(uint32_t const offset2SP, bool const is64bit) {
  // Use R9 as scratch register
  confirm(offset2SP <= MaxPositiveImmForLdrStr, "offset2SP too large");
  as_.ldr_base_off(REG::R9, REG::SP, static_cast<int32_t>(offset2SP & MaxPositiveImmForLdrStr), is64bit);
  as_.str_base_off(ROP, REG::R9, 0U, is64bit);
  as_.add_r_r_imm(ROP, ROP, is64bit ? 8U : 4U, true);
}
void OP::set_ofsp_local(uint32_t const offset2SP, bool const is64bit, bool const isTee = false) {
  // Use R9 as scratch register
  subROP(is64bit);
  confirm(offset2SP <= MaxPositiveImmForLdrStr, "offset2SP too large");
  as_.ldr_base_off(REG::R9, ROP, 0U, is64bit);
  as_.str_base_off(REG::SP, REG::R9, static_cast<int32_t>(offset2SP & MaxPositiveImmForLdrStr), is64bit);
  // TODO(): reduce ins, if tee don't sub sp, can use signed offset load(not support yet)
  if (isTee) {
    // restore the ROP
    as_.add_r_r_imm(ROP, ROP, is64bit ? 8U : 4U, true);
  }
}