#include <cassert>
#include <iostream>

#include "../src/compiler.hpp"

int main(int argc, char *argv[]) {
  assert(argc == 2);
  Compiler compiler;

  ExecutableMemory const execMemory = compiler.compile(argv[1]);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  void (*const func)() = execMemory.data<void (*)()>();
  func();

  std::cout << "after native call" << std::endl;
}