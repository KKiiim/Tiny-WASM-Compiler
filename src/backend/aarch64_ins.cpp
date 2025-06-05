#include <cassert>
#include <cstdint>

#include "aarch64_encoding.hpp"

OPCodeTemplate str_base_off(REG const addrReg, REG const srcReg, uint32_t const offset, bool const is64bit) {
  // Encoding for the 32-bit variant
  // Applies when (size == 10)
  // STR <Wt>, [<Xn|SP>{, #<pimm>}]
  // Encoding for the 64-bit variant
  // Applies when (size == 11)
  // STR <Xt>, [<Xn|SP>{, #<pimm>}]

  // 1x 11 1001 00 imm12 Rn Rt
  // F9000000
  assert(offset <= 0xFFFU && "Immediate out of range");
  OPCodeTemplate opcode = is64bit ? 0xF9000000 : 0xB9000000;
  opcode |= static_cast<OPCodeTemplate>(srcReg);
  opcode |= (static_cast<OPCodeTemplate>(addrReg) << 5U);
  opcode |= (bit_cast<uint32_t>(offset) & 0xFFFU) << 10U;
  return opcode;
}
OPCodeTemplate ldr_base_off(REG const destReg, REG const addrReg, uint32_t const offset, bool const is64bit) {
  // Applies when (size == 10)
  // LDR <Wt>, [<Xn|SP>{, #<pimm>}]
  // Applies when (size == 11)
  // LDR <Xt>, [<Xn|SP>{, #<pimm>}]

  // 1x 111001 01 imm12 Rn Rt
  // F9400000 B9400000
  assert(offset <= 0xFFFU && "Immediate out of range");
  OPCodeTemplate opcode = is64bit ? 0xF9400000 : 0xB9400000;
  opcode |= (static_cast<OPCodeTemplate>(addrReg) << 5U); // addr 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);         // dest 0-4
  opcode |= (bit_cast<uint32_t>(offset) & 0xFFFU) << 10U;
  return opcode;
}
OPCodeTemplate add_r_r_imm(REG const destReg, REG const srcReg, uint32_t const uimm, bool const is64bit) {
  // sf 0 0 100010 sh imm12 Rn Rd
  // 0001 0001 00 imm12 00000 00000
  // 11000000
  // shift default set 0
  assert(is64bit && "currently always use full register 64bits");
  OPCodeTemplate opcode = is64bit ? 0x91000000 : 0x11000000; // ADD Xd, Xn, #imm
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 5U);     // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);            // dest 0-4
  opcode |= (uimm & 0xFFFU) << 10U;                          // imm12
  return opcode;
}
OPCodeTemplate add_r_r_r(REG const destReg, REG const firstSrcReg, REG const secondSrcReg, bool const is64bit) {
  // TODO(): can support shift and other extend features
  // sf 000 1011 001 Rm option(3) immShift(3) Rn Rd
  // 0b200000
  OPCodeTemplate opcode = is64bit ? 0x8b200000 : 0x0b200000;
  opcode |= (static_cast<OPCodeTemplate>(secondSrcReg) << 16U); // source 16-20
  opcode |= (static_cast<OPCodeTemplate>(firstSrcReg) << 5U);   // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);               // dest 0-4
  return opcode;
}
OPCodeTemplate sub_r_r_imm(REG const destReg, REG const srcReg, uint32_t const imm, bool const is64bit) {
  // sf 1 0 100010 sh imm12 Rn Rd
  // 0101 0001 0000
  // 61000000
  // 1101 0001 0000
  // d1000000
  assert(is64bit && "currently always use full register 64bits");
  OPCodeTemplate opcode = 0xd1000000;                    // SUB Xd, Xn, #imm
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 5U); // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);        // dest 0-4
  opcode |= (imm & 0xFFFU) << 10U;
  return opcode;
}
OPCodeTemplate sub_r_r_immReg(REG const destReg, REG const srcReg, REG const immReg) {
  assert(false && "not implemented");

  OPCodeTemplate opcode = 0x4B000000; // SUB Xd, Xn, Xm
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 5U);
  opcode |= static_cast<OPCodeTemplate>(destReg);
  opcode |= (static_cast<OPCodeTemplate>(immReg) << 16U);
  return opcode;
}
OPCodeTemplate inc_sp(uint32_t const imm) {
  return add_r_r_imm(REG::SP, REG::SP, imm, true);
}
OPCodeTemplate dec_sp(uint32_t const imm) {
  return sub_r_r_imm(REG::SP, REG::SP, imm, true);
}
OPCodeTemplate mov_r_r(REG const destReg, REG const srcReg) {
  assert(false && "not implemented");

  // OPCodeTemplate opcode = is64bit ? 0xAA0003E0 : 0x2A0003E0;
  OPCodeTemplate opcode = 0x2A0003E0;                    // MOV Xd, Xn
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 5U); // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);        // dest 0-4
  return opcode;
}
OPCodeTemplate movk_r_imm16(REG const destReg, uint16_t const imm, uint8_t const shift, bool const is64bit) {
  /*
    MOVK (keep)
    sf 11100101 hw2 imm16 Rd(5)
    Encoding for the 32-bit variant
    Applies when (sf == 0 && hw == 0x)
    MOVK <Wd>, #<imm>{, LSL #<shift>}

    Encoding for the 64-bit variant
    Applies when (sf == 1) => f2800000
    MOVK <Xd>, #<imm>{, LSL #<shift>}

    <shift>
  "32-bit": 0 (the default) or 16, encoded in the "hw" field as <shift>/16.
  "64-bit": 0 (the default), 16, 32 or 48, encoded in the "hw" field as <shift>/16.
  */
  assert(imm <= 0xFFFFU && "Immediate out of range");
  OPCodeTemplate opcode = is64bit ? 0xf2800000 : 0x72800000;
  assert(shift <= (is64bit ? 3U : 1U) && "Shift out of range");
  opcode |= (static_cast<OPCodeTemplate>(destReg)); // dest 0-4
  opcode |= (imm & 0xFFFFU) << 5U;                  // immediate value 5-20
  opcode |= (shift & 0x3U) << 21U;                  // shift value 21-22
  return opcode;
}
OPCodeTemplate mov_r_imm16(REG const destReg, uint16_t const imm, bool const is64bit) {
  // MOV (wide immediate)
  // sf 10(opc) 100101 hw2 imm16 Rd
  // 1101 0010 100 imm16 00000
  // 0xd280000
  // 1011 0010 0000
  // 1 01 100100
  assert(imm <= 0xFFFFU && "Immediate out of range");
  OPCodeTemplate opcode = is64bit ? 0xd2800000 : 0x52800000;
  opcode |= (static_cast<OPCodeTemplate>(destReg)); // dest 0-4
  opcode |= (imm & 0xFFFFU) << 5U;                  // immediate value 5-20
  return opcode;
}
// OPCodeTemplate str_sp_imm(REG const srcReg, uint32_t const spOffsetImm, bool const is64bit) {
//   OPCodeTemplate opcode = is64bit ? 0xF9000000 : 0xB9000000; // STR Xn, [SP, #imm] for 64-bit or 32-bit
//   opcode |= (static_cast<OPCodeTemplate>(srcReg) << 10U);    // source register 10-14
//   opcode |= (spOffsetImm & 0xFFFU) << 10U;                   // immediate offset 10-21
//   opcode |= (static_cast<OPCodeTemplate>(REG::SP) << 5U);    // SP register as base, 5-9
//   return opcode;
// }