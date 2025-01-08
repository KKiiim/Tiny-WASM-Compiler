#include <cassert>
#include <iostream>

#include "../src/compiler.hpp"

int main(int argc, char *argv[]) {
  assert(argc == 2);
  Compiler Compiler{argv[1]};

  ExecutableMemory execMemory = Compiler.startCompilation();
  Compiler.logParsedInfo();
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  void (*func)() = reinterpret_cast<void (*)()>(execMemory.data());
  func();

  std::cout << "after native call" << std::endl;
}