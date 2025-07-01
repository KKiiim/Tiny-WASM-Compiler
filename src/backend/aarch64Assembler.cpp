#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <sys/mman.h>

#include "aarch64Assembler.hpp"

void Assembler::append(OPCodeTemplate const ins) {
  // convert ins to little endian
  uint8_t const *const insPtr = bit_cast<uint8_t const *>(&ins);
  OPCodeTemplate littleEndianIns = 0;
  for (size_t i = 0; i < sizeof(OPCodeTemplate); i++) {
    littleEndianIns |= static_cast<OPCodeTemplate>(insPtr[i]) << (i * 8U);
  }

  outputBinary_.append(ins);
}