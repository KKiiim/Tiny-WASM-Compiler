#ifndef SRC_COMMON_OPERAND_STACK_HPP
#define SRC_COMMON_OPERAND_STACK_HPP

#include <cstdint>
#include <cstdlib>
#include <iostream>

#include "ExecutableMemory.hpp"
#include "constant.hpp"
#include "util.hpp"
#include "wasm_type.hpp"

class OperandStack {
  // R28 -> start | operand stack start address

public:
  OperandStack() : m_(bit_cast<void *>(malloc(DefaultPageSize))), mem_(static_cast<uint8_t *>(m_), DefaultPageSize) {
    // std::cout << "mmap for operand stack" << std::endl;
  }
  inline uint64_t getStartAddr() {
    return mem_.data<uint64_t>();
  }

  enum class OperandType : uint8_t { I32 = 0, I64 };
  static WasmType toWasmType(OperandType const tOperand) {
    switch (tOperand) {
    case OperandType::I32:
      return WasmType::I32;
    case OperandType::I64:
      return WasmType::I64;
    default:
      return WasmType::I32;
    }
  }

private:
  void *m_ = nullptr;
  ExecutableMemory mem_;
};

#endif