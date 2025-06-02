#include <cassert>
#include <iostream>

#include "../src/compiler.hpp"

int main(int argc, char *argv[]) {
  assert(argc == 2);
  Compiler compiler;

  ExecutableMemory const &execMemory = compiler.compile(argv[1]);
  execMemory.disassemble();

  // uint32_t const ret = compiler.singleCallByName<uint32_t>("type-local-i32", "i()");
  uint32_t const ret = compiler.singleCallByName<uint32_t>("type-param-i32", "i(i)", 3U);

  std::cout << "after native call, ret = " << ret << std::endl;
}