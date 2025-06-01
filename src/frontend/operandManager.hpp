#ifndef SRC_BACKEND_OPERAND_MANAGER_H
#define SRC_BACKEND_OPERAND_MANAGER_H

#include <cstdint>

#include "../backend/arm64Backend.hpp"
#include "../common/wasm_type.hpp"

class OP {
  // R28 always point the next free space in the stack
  static constexpr const REG ROP = REG::R28;

public:
  explicit OP(Arm64Backend &backend) : size_(0U), backend_(backend) {
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
  void drop(bool const is64bit);

private:
  uint32_t size_;
  Arm64Backend &backend_;

  // Max number of local variables in a function
  static constexpr const uint32_t MAX_LOCAL_VARS = 8U;
};

#endif