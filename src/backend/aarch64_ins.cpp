#include "aarch64_encoding.hpp"

// OPCodeTemplate mov_imm2r(REG const destReg, uint64_t const imm, bool const is64bit) {
//   return 0;
// }
OPCodeTemplate str_r2ar(REG const addrReg, REG const srcReg, bool const is64bit) {
  OPCodeTemplate opcode = is64bit ? 0xF9000000 : 0xB9000000; // STR Xn/Yn, [Xn/Yn]
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 5U);     // source 5-9
  opcode |= static_cast<OPCodeTemplate>(addrReg);            // addr 0-4
  return opcode;
}
OPCodeTemplate ldr_ar2r(REG const destReg, REG const addrReg, bool const is64bit) {
  OPCodeTemplate opcode = is64bit ? 0xF9400000 : 0xB9400000; // LDR Xd/Yd, [Xn/Yn]
  opcode |= (static_cast<OPCodeTemplate>(addrReg) << 5U);    // addr 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);            // dest 0-4
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
  OPCodeTemplate opcode = 0x4A000000;                    // SUB Xd, Xn, #imm
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 5U); // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);        // dest 0-4
  opcode |= (imm & 0xFFFU) << 10U;
  return opcode;
}
OPCodeTemplate sub_r_r_immReg(REG const destReg, REG const srcReg, REG const immReg) {
  OPCodeTemplate opcode = 0x4B000000; // SUB Xd, Xn, Xm
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
OPCodeTemplate mov_r_r(REG const destReg, REG const srcReg) {
  OPCodeTemplate opcode = 0x2A0003E0;                    // MOV Xd, Xn
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 5U); // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);        // dest 0-4
  return opcode;
}
OPCodeTemplate mov_r_imm(REG const destReg, uint64_t const imm) {
  OPCodeTemplate opcode = 0x2A000000;               // MOV Xd, #imm
  opcode |= (static_cast<OPCodeTemplate>(destReg)); // dest 0-4
  opcode |= (imm & 0xFFFU) << 10U;                  // immediate value 10-21
  return opcode;
}
// OPCodeTemplate str_sp_imm(REG const srcReg, uint32_t const spOffsetImm, bool const is64bit) {
//   OPCodeTemplate opcode = is64bit ? 0xF9000000 : 0xB9000000; // STR Xn, [SP, #imm] for 64-bit or 32-bit
//   opcode |= (static_cast<OPCodeTemplate>(srcReg) << 10U);    // source register 10-14
//   opcode |= (spOffsetImm & 0xFFFU) << 10U;                   // immediate offset 10-21
//   opcode |= (static_cast<OPCodeTemplate>(REG::SP) << 5U);    // SP register as base, 5-9
//   return opcode;
// }