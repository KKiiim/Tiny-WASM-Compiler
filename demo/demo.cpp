#include <cassert>
#include <cstdint>

#include "src/common/constant.hpp"
#include "src/common/logger.hpp"
#include "src/compiler.hpp"
#include "src/runtime.hpp"

int main(int argc, char *argv[]) {
  assert(argc == 2);
  Compiler compiler;

  ExecutableMemory const &execMemory = compiler.compile(argv[1]);
  execMemory.disassemble();

  {
    Runtime runtime{compiler};

    // [ref: doc-design]
    std::array<uint64_t, MaxParamsForWasmFunction> params{0, 0, 0, 0, 0, 0, 0};
    Runtime::CallReturn const ret =
        runtime.callByName<uint32_t>("div_s", "i(ii)", params[0], params[1], params[2], params[3], params[4], params[5], params[6]);
    if (ret.hasTrapped) {
      Trapcode const trapCode = runtime.getTrapCode();
      LOG_YELLOW << "TrapException caught with code: " << trapCode << LOG_END;
      LOG_YELLOW << "Exception: " << trapcodeString.at(trapCode) << LOG_END;
    } else {
      uint32_t const ret32 = static_cast<uint32_t>(ret.returnValue);
      LOG_GREEN << "div_s, i(ii) = " << ret32 << LOG_END;
    }
  }
}