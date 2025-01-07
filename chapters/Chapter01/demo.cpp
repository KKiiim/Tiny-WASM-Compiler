#include <cassert>

#include "src/compiler.hpp"

int main(int argc, char *argv[]) {
  assert(argc == 2);
  Compiler Compiler{argv[1]};

  Compiler.startCompilation();

  Compiler.logParsedInfo();
}