#ifndef SRC_COMMON_OPERAND_STACK_HPP
#define SRC_COMMON_OPERAND_STACK_HPP

#include <cstdint>
#include <stack>

class OperandStack {
  // R28 -> start | operand stack start address

public:
  OperandStack() = default;
  inline void *getStartAddr() {
    return mem_;
  }

  enum class OperandType : uint8_t { I32 = 0, I64 };
  std::stack<OperandType> validationStack_{};

private:
  void *mem_ = nullptr;
};

#endif