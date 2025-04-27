#ifndef SRC_COMMON_OPERAND_STACK_HPP
#define SRC_COMMON_OPERAND_STACK_HPP

#include <cstdint>
#include <stack>

#include "wasm_type.hpp"

class OperandStack {
  // R28 -> start | operand stack start address

public:
  OperandStack() = default;
  inline void *getStartAddr() {
    return mem_;
  }

  enum class OperandType : uint8_t { I32 = 0, I64 };
  WasmType toWasmType(OperandType const tOperand) const {
    switch (tOperand) {
    case OperandType::I32:
      return WasmType::I32;
    case OperandType::I64:
      return WasmType::I64;
    default:
      return WasmType::I32;
    }
  }
  std::stack<OperandType> validationStack_{};

private:
  void *mem_ = nullptr;
};

#endif