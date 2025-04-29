#include "aarch64_encoding.hpp"

// OPCodeTemplate mov_imm2r(REG const destReg, uint64_t const imm, bool const is64bit) {
//   return 0;
// }
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
OPCodeTemplate str_imm2ar(REG const addrReg, uint32_t const imm) {
  OPCodeTemplate opcode = 0xF9000000;                     // STR Xt, [Xn, #imm]
  opcode |= (static_cast<OPCodeTemplate>(addrReg) << 5U); // source 5-9
  opcode |= (imm & 0xFFFU) << 10U;
  return opcode;
}
OPCodeTemplate add_r_r_imm(REG const destReg, REG const srcReg, uint32_t const imm) {
  OPCodeTemplate opcode = 0x2A000000;                    // ADD Xd, Xn, #imm
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 5U); // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);        // dest 0-4
  opcode |= (imm & 0xFFFU) << 10U;
  return opcode;
}
OPCodeTemplate add_r_r_immReg(REG const destReg, REG const srcReg, REG const immReg) {
  OPCodeTemplate opcode = 0x2B000000;                    // ADD Xd, Xn, Xm
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 5U); // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);        // dest 0-4
  opcode |= (static_cast<OPCodeTemplate>(immReg) << 16U);
  return opcode;
}
OPCodeTemplate sub_r_r_imm(REG const destReg, REG const srcReg, uint32_t const imm) {
  OPCodeTemplate opcode = 0x2A000000;                    // SUB Xd, Xn, #imm
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 5U); // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);        // dest 0-4
  opcode |= (imm & 0xFFFU) << 10U;
  return opcode;
}
OPCodeTemplate sub_r_r_immReg(REG const destReg, REG const srcReg, REG const immReg) {
  OPCodeTemplate opcode = 0x2B000000; // SUB Xd, Xn, Xm
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 5U);
  opcode |= static_cast<OPCodeTemplate>(destReg);
  opcode |= (static_cast<OPCodeTemplate>(immReg) << 16U);
  return opcode;
}
OPCodeTemplate inc_sp(uint32_t const imm) {
  return add_r_r_imm(REG::SP, REG::SP, imm);
}
OPCodeTemplate dec_sp(uint32_t const imm) {
  return sub_r_r_imm(REG::SP, REG::SP, imm);
}