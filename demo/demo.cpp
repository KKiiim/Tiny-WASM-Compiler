#include <cassert>
#include <iostream>

#include "../src/frontend/frontend.hpp"

int main(int argc, char *argv[]) {
  assert(argc == 2);
  Frontend Compiler{argv[1]};

  ExecutableMemory const execMemory = Compiler.startCompilation();
  Compiler.logParsedInfo();
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  void (*const func)() = execMemory.data<void (*)()>();
  func();

  std::cout << "after native call" << std::endl;
}