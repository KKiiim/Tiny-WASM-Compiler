#include <cassert>

#include "src/common/logger.hpp"
#include "src/compiler.hpp"

int main(int argc, char *argv[]) {
  assert(argc == 2);
  Compiler compiler;

  ExecutableMemory const &execMemory = compiler.compile(argv[1]);
  execMemory.disassemble();

  uint32_t const ret32 = compiler.singleCallByName<uint32_t>("singular", "i(i)", 0U);
  LOG_GREEN << "singular, i(i) = " << ret32 << LOG_END;
}