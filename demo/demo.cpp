#include <cassert>
#include <cstdint>

#include "src/common/logger.hpp"
#include "src/compiler.hpp"
#include "src/runtime.hpp"

int main(int argc, char *argv[]) {
  assert(argc == 2);
  Compiler compiler;

  ExecutableMemory const &execMemory = compiler.compile(argv[1]);
  execMemory.disassemble();

  Runtime runtime{compiler};
  runtime.registerSignalHandler();

  Runtime::CallReturn const ret = runtime.callByName<uint32_t>("div_s", "i(ii)", 0, 0);
  if (ret.hasTrapped) {
    LOG_YELLOW << "TrapException caught with code: " << runtime.getTrapCode() << LOG_END;
    LOG_YELLOW << "Exception: " << runtime.getTrapMessage() << LOG_END;
  } else {
    uint32_t const ret32 = static_cast<uint32_t>(ret.returnValue);
    LOG_GREEN << "div_s, i(ii) = " << ret32 << LOG_END;
  }
}