#ifndef SRC_COMMON_OPERAND_STACK_HPP
#define SRC_COMMON_OPERAND_STACK_HPP

#include <cstdint>
#include <cstdlib>

#include "ExecutableMemory.hpp"
#include "constant.hpp"
#include "util.hpp"
#include "wasm_type.hpp"

#include "src/common/logger.hpp"

class OperandStack {
  // R28 -> start | operand stack start address

public:
  OperandStack() : m_(bit_cast<void *>(malloc(DefaultPageSize))) {
    LOG_DEBUG << "malloc operand stack for runtime" << LOG_END;
  }
  ~OperandStack() {
    free(m_);
  }
  OperandStack(const OperandStack &) = delete;
  OperandStack &operator=(const OperandStack &) = delete;
  OperandStack(OperandStack &&) = delete;
  OperandStack &operator=(OperandStack &&) = delete;

  inline uint64_t getStartAddr() {
    return bit_cast<uint64_t>(m_);
  }

private:
  void *m_ = nullptr;
};

#endif