// wasm to aarch64
#include <cstdint>
#include <cstdlib>

#include "../common/ExecutableMemory.hpp"
#include "../common/wasm_type.hpp"

class Emit {
public:
  Emit();

  void append(OPCode const opcode);
  ExecutableMemory getExecutableMemory();

private:
  uint32_t const DefaultPageSize = static_cast<uint32_t>(1000U) * 2U;

  uint8_t *data_;
  uint32_t size_;
};