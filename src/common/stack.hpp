#ifndef SRC_COMMON_STACK_H
#define SRC_COMMON_STACK_H

#include <cstdint>
#include <vector>

#include "src/common/wasm_type.hpp"

class StackElement final {
public:
  enum class ElementType : uint8_t {
    NONE,

    FUNC_START,
    IF,
    ELSE,
    END,
  };
  ElementType elementType_ = StackElement::ElementType::NONE;

  WasmType returnType_ = WasmType::INVALID;
  ///< Point the start of the branch instruction as offset relative to the output binary.
  // Used for relpatch
  uint32_t positionOffset_of_ConditionInstruction = 0;
};

class Stack final {
public:
  uint32_t push(StackElement const &element);
  StackElement pop();
  StackElement &top();
  inline bool empty() const {
    return v_.empty();
  }

private:
  std::vector<StackElement> v_;
};

#endif