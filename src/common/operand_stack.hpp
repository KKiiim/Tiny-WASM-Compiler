#ifndef SRC_COMMON_OPERAND_STACK_HPP
#define SRC_COMMON_OPERAND_STACK_HPP

#include <cstdint>
#include <cstdlib>

#include "constant.hpp"
#include "util.hpp"

#include "src/common/logger.hpp"

class OperandStack {
  // R28 -> start | operand stack start address
public:
  OperandStack() : m_(bit_cast<void *>(malloc(DefaultPageSize))) {
    confirm(m_ != nullptr, "malloc for operand stack failed");
  }
  ~OperandStack() {
    free(m_);
  }
  OperandStack(const OperandStack &) = delete;
  OperandStack &operator=(const OperandStack &) = delete;
  OperandStack(OperandStack &&) = delete;
  OperandStack &operator=(OperandStack &&) = delete;

  inline uint64_t getStartAddr() {
    return static_cast<uint64_t>(bit_cast<uintptr_t>(m_));
  }

private:
  void *m_ = nullptr;
};

#endif