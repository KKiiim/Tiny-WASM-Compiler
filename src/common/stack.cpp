#include "stack.hpp"

#include "src/common/logger.hpp"

uint32_t Stack::push(StackElement const &element) {
  v_.emplace_back(element);
  return (v_.size() - 1U);
}

StackElement Stack::pop() {
  confirm(!v_.empty(), "");
  StackElement end = v_[v_.size() - 1U];
  v_.pop_back();
  return end;
}

StackElement &Stack::top() {
  confirm(!v_.empty(), "");
  return v_.back();
}

StackElement const &Stack::lastControlFlowElement() const {
  for (auto it = v_.rbegin(); it != v_.rend(); ++it) {
    if (it->isControlFlow()) {
      return *it;
    }
  }
  confirm(false, "no control flow element in stack");
  static StackElement const dummy{ElementType::NONE};
  // Unreachable. To avoid compiler warning
  return dummy;
}

void Stack::popToLastControlFlowElement() {
  while (!v_.empty() && !v_.back().isControlFlow()) {
    v_.pop_back();
  }
  // should pop the last control flow element itself
  confirm((!v_.empty()) && v_.back().isControlFlow(), "");
  v_.pop_back();
}

StackElement const &Stack::findTargetBlock(uint32_t const depth) const {
  // TODO(): not support if and block mixed
  uint32_t blockCounter = depth;
  // find from last to first
  for (auto it = v_.rbegin(); it != v_.rend(); ++it) {
    if (it->elementType_ == ElementType::BLOCK) {
      if (blockCounter == 0U) {
        return *it;
      }
      blockCounter--;
    }
  }
  confirm(false, "no target block found");
  static StackElement const dummy{ElementType::NONE};
  // Unreachable. To avoid compiler warning
  return dummy;
}