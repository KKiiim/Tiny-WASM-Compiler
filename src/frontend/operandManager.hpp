#ifndef SRC_BACKEND_OPERAND_MANAGER_H
#define SRC_BACKEND_OPERAND_MANAGER_H

#include <cstdint>

#include "src/backend/aarch64Assembler.hpp"
#include "src/common/wasm_type.hpp"

class OP {
public:
  explicit OP(Assembler &as) : size_(0U), as_(as) {
  }
  /// @brief store i32 and i64
  /// @return record offset in from SP
  uint32_t add(WasmType const localType);
  inline uint32_t getAlignedSize() const {
    constexpr uint32_t alignment = 16U;
    return (size_ + alignment - 1) & ~(alignment - 1);
  }
  /// @brief Push param in a register to the operand stack
  void get_r_param(uint32_t const paramIndex, bool const is64bit);
  /// @brief Pop param from the operand stack to its register
  void set_r_param(uint32_t const paramIndex, bool const is64bit, bool const isTee);
  /// @brief Push local variable to the operand stack
  /// @param isTee whether to store the value in the operand stack
  void get_ofsp_local(uint32_t const offset2SP, bool const is64bit);
  /// @brief Pop local variable from the operand stack
  void set_ofsp_local(uint32_t const offset2SP, bool const is64bit, bool const isTee);
  /// @brief drop a value from the operand stack
  inline void subROP(bool const is64bit) {
    as_.sub_r_r_imm(ROP, ROP, is64bit ? 8U : 4U, true);
  }
  inline void addROP(bool const is64bit) {
    as_.add_r_r_imm(ROP, ROP, is64bit ? 8U : 4U, true);
  }

private:
  uint32_t size_;
  Assembler &as_;
};

#endif