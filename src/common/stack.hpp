#ifndef SRC_COMMON_STACK_H
#define SRC_COMMON_STACK_H

#include <cstdint>
#include <vector>

#include "src/common/logger.hpp"
#include "src/common/wasm_type.hpp"

enum class ElementType : uint8_t {
  NONE,

  FUNC_START,
  IF,
  ELSE,
  END,

  I32,
  I64,
};
class StackElement final {
public:
  explicit StackElement(ElementType type) : elementType_(type) {
    confirm(type != ElementType::NONE, "must not be NONE");
  }
  inline bool isValue() const {
    return (elementType_ == ElementType::I32 || elementType_ == ElementType::I64);
  }
  inline bool isI64() const {
    confirm(isValue(), "must be value type");
    return elementType_ == ElementType::I64;
  }
  inline bool isControlFlow() const {
    return (elementType_ == ElementType::FUNC_START || elementType_ == ElementType::IF || elementType_ == ElementType::ELSE ||
            elementType_ == ElementType::END);
  }
  ElementType elementType_;

  WasmType returnType_ = WasmType::INVALID;

  ///< Point the start of the branch instruction as offset relative to the output binary.
  // Used for relocation patching in cross control flow circumstance
  uint32_t relpatchInsPos = 0;
};

class Stack final {
public:
  uint32_t push(StackElement const &element);
  StackElement pop();
  StackElement &top();
  inline bool empty() const {
    return v_.empty();
  }
  StackElement const &lastControlFlowElement() const;

  ///< Including thr last control flow element itself
  void popToLastControlFlowElement();

private:
  std::vector<StackElement> v_;
};

#endif