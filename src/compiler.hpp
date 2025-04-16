#ifndef SRC_COMPILER_H
#define SRC_COMPILER_H

#include <string>

#include "common/ExecutableMemory.hpp"
#include "common/stack.hpp"

class Compiler final {
public:
  explicit Compiler() : stack_(){};

  ExecutableMemory compile(std::string const &wasmPath);

private:
  Stack stack_; ///< Compiler stack
};

#endif