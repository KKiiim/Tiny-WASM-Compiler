#ifndef SRC_COMMON_STACK_H
#define SRC_COMMON_STACK_H

#include <cstdint>

class Stack final {
public:
  Stack() = default;

private:
  void *mem_;
  uint32_t offset_;
};

#endif