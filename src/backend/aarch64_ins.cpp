#include "aarch64_encoding.hpp"

OPCodeTemplate ldr_ar2r(REG const destReg, REG const addrReg, bool const is64bit) {
  OPCodeTemplate opcode = 0;

  if (is64bit) {
    // LDR Xt, [Xn]: 0xF8 00 00 00 | (Xn <<5) | Xt
    opcode = 0xF8400000;
  } else {
    // LDR Wt, [Xn]: 0xB8 00 00 00 | (Xn <<5) | Wt
    opcode = 0xB8400000;
  }

  opcode |= (static_cast<OPCodeTemplate>(addrReg) << 5U); // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);         // dest 0-4

  return opcode;
}