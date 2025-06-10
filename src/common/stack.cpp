#include <cassert>

#include "stack.hpp"

uint32_t Stack::push(StackElement const &element) {
  v_.emplace_back(element);
  return (v_.size() - 1U);
}

StackElement Stack::pop() {
  assert(!v_.empty());
  StackElement end = v_[v_.size() - 1U];
  v_.pop_back();
  return end;
}

StackElement &Stack::top() {
  assert(!v_.empty());
  return v_.back();
}