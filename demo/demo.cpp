#include <cassert>
#include <iostream>

#include "../src/compiler.hpp"

int main(int argc, char *argv[]) {
  assert(argc == 2);
  Compiler compiler;

  ExecutableMemory const execMemory = compiler.compile(argv[1]);
  execMemory.disassemble();
  compiler.initRuntime();

  void (*const func)() = execMemory.data<void (*)()>();
  func();

  std::cout << "after native call" << std::endl;
}