#include <cassert>

#include "src/common/logger.hpp"
#include "src/compiler.hpp"

int main(int argc, char *argv[]) {
  assert(argc == 2);
  Compiler compiler;

  ExecutableMemory const &execMemory = compiler.compile(argv[1]);
  execMemory.disassemble();

  uint32_t ret32 = compiler.singleCallByName<uint32_t>("type-local-i32", "i()");
  LOG_GREEN << "type-local-i32, i() = " << ret32 << std::endl;
  uint64_t ret64 = compiler.singleCallByName<uint64_t>("type-local-i64", "I()");
  LOG_GREEN << "type-local-i64, I() = " << ret64 << std::endl;
  ret32 = compiler.singleCallByName<uint32_t>("type-param-i32", "i(i)", 2U);
  LOG_GREEN << "type-param-i32, i(i) = " << ret32 << std::endl;
  ret64 = compiler.singleCallByName<uint64_t>("type-param-i64", "I(I)", 3U);
  LOG_GREEN << "type-param-i64, I(I) = " << ret64 << std::endl;
}