#include <cinttypes>
#include <cstdint>

#include "aarch64Assembler.hpp"
#include "aarch64_encoding.hpp"
#include "relpatch.hpp"

#include "src/common/constant.hpp"
#include "src/common/logger.hpp"

/////////////////////////////////////////////////////////////////
//< Basic instructions
/////////////////////////////////////////////////////////////////

void Assembler::ret() {
  constexpr const OPCodeTemplate insRET = 0xd65f03c0; // big endian
  append(insRET);
}
void Assembler::str_base_byteOff(REG const addrReg, REG const srcReg, uint32_t const byteOffset, bool const is64bit) {
  // Encoding for the 32-bit variant
  // Applies when (size == 10)
  // STR <Wt>, [<Xn|SP>{, #<pimm>}]
  // Encoding for the 64-bit variant
  // Applies when (size == 11)
  // STR <Xt>, [<Xn|SP>{, #<pimm>}]

  // 1x 11 1001 00 imm12 Rn Rt
  // F9000000
  OPCodeTemplate opcode = is64bit ? 0xF9000000 : 0xB9000000;
  opcode |= static_cast<OPCodeTemplate>(srcReg);
  opcode |= (static_cast<OPCodeTemplate>(addrReg) << 5U);
  // currently only support positive offset(bytes)
  uint32_t off = 0U;
  if (is64bit) {
    confirm(byteOffset % 8U == 0U, "Immediate offset must be multiple of 8 for 64-bit");
    off = byteOffset / 8U; // convert to number of 64-bit words
  } else {
    confirm(byteOffset % 4U == 0U, "Immediate offset must be multiple of 4 for 32-bit");
    off = byteOffset / 4U; // convert to number of 32-bit words
  }
  confirm(off <= 0xFFFU, "Immediate out of range");
  opcode |= (off & 0xFFFU) << 10U;
  append(opcode);
}
void Assembler::str_w_r(REG const addrReg, REG const srcReg) {
  OPCodeTemplate opcode = 0xB8206800;
  opcode |= static_cast<OPCodeTemplate>(srcReg);
  opcode |= (static_cast<OPCodeTemplate>(addrReg) << 5U);
  append(opcode);
}
void Assembler::str_h_w_r(REG const addrReg, REG const srcReg) {
  OPCodeTemplate opcode = 0x78206800;
  opcode |= static_cast<OPCodeTemplate>(srcReg);
  opcode |= (static_cast<OPCodeTemplate>(addrReg) << 5U);
  append(opcode);
}
void Assembler::ldr_base_byteOff(REG const destReg, REG const addrReg, uint32_t const byteOffset, bool const is64bit) {
  // Applies when (size == 10)
  // LDR <Wt>, [<Xn|SP>{, #<pimm>}]
  // Applies when (size == 11)
  // LDR <Xt>, [<Xn|SP>{, #<pimm>}]

  // Unsigned offset
  // 1x 111001 01 imm12 Rn Rt
  // F9400000 B9400000
  OPCodeTemplate opcode = is64bit ? 0xF9400000 : 0xB9400000;
  opcode |= (static_cast<OPCodeTemplate>(addrReg) << 5U); // addr 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);         // dest 0-4
  uint32_t off = 0U;
  if (is64bit) {
    confirm(byteOffset % 8U == 0U, "Immediate offset must be multiple of 8 for 64-bit");
    off = byteOffset / 8U; // convert to number of 64-bit words
  } else {
    confirm(byteOffset % 4U == 0U, "Immediate offset must be multiple of 4 for 32-bit");
    off = byteOffset / 4U; // convert to number of 32-bit words
  }
  confirm(off <= 0xFFFU, "Immediate out of range");
  opcode |= (off & 0xFFFU) << 10U;
  append(opcode);
}
void Assembler::ldr_offReg(REG const destReg, REG const addrReg, REG const offsetReg, bool const is64bit) {
  // size(1 x) 11 1000 011 Rm option(3) S(1) 10 Rn Rt
  // 1011 1000 011 Rm 010(UXTW) 1(S) 10 Rn Rt
  // B8605800

  OPCodeTemplate opcode = is64bit ? 0xF8605800 : 0xB8605800;
  opcode |= (static_cast<OPCodeTemplate>(offsetReg) << 16U);
  opcode |= (static_cast<OPCodeTemplate>(addrReg) << 5U);
  opcode |= static_cast<OPCodeTemplate>(destReg);
  append(opcode);
}
void Assembler::ldrb_uimm(REG const destReg, REG const addrReg, uint32_t const uimm) {
  // 0011 1001 01 imm12 Rn Rt
  // 39400000
  confirm(uimm <= 0xFFF, "Immediate out of range");
  OPCodeTemplate opcode = 0x39400000;
  opcode |= (static_cast<OPCodeTemplate>(addrReg) << 5U); // addr 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);         // dest 0-4
  opcode |= (uimm & 0xFFFU) << 10U;                       // imm12
  append(opcode);
}
void Assembler::ldrb_uReg(REG const destReg, REG const addrReg, REG const offsetReg) {
  // 0011 1000 011 Rm option(011 shift-mode) S(0) 10 Rn Rt
  // 38606800
  // no shift
  OPCodeTemplate opcode = 0x38606800;                        // LDRB (register offset)
  opcode |= (static_cast<OPCodeTemplate>(offsetReg) << 16U); // source 16-20
  opcode |= (static_cast<OPCodeTemplate>(addrReg) << 5U);    // addr 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);            // dest 0-4
  append(opcode);
}
void Assembler::add_r_r_imm(REG const destReg, REG const srcReg, uint32_t const uimm, bool const is64bit) {
  // sf 0 0 100010 sh imm12 Rn Rd
  // 0001 0001 00 imm12 00000 00000
  // 11000000
  // shift default set 0
  confirm(is64bit, "currently always use full register 64bits");
  OPCodeTemplate opcode = is64bit ? 0x91000000 : 0x11000000; // ADD Xd, Xn, #imm
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 5U);     // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);            // dest 0-4
  opcode |= (uimm & 0xFFFU) << 10U;                          // imm12
  append(opcode);
}
void Assembler::add_r_r_extendedR(REG const destReg, REG const firstSrcReg, REG const secondSrcReg, bool const is64bit) {
  confirm(false, "don't know how to use yet");
  // TODO(): can support shift and other extend features
  // sf 000 1011 001 Rm option(3) immShift(3) Rn Rd
  // 0b200000
  OPCodeTemplate opcode = is64bit ? 0x8b200000 : 0x0b200000;
  opcode |= (static_cast<OPCodeTemplate>(secondSrcReg) << 16U); // source 16-20
  opcode |= (static_cast<OPCodeTemplate>(firstSrcReg) << 5U);   // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);               // dest 0-4
  append(opcode);
}
void Assembler::add_r_r_shiftR(REG const destReg, REG const firstSrcReg, REG const secondSrcReg, bool const is64bit) {
  // sf 000 1011 shift(2) 0 Rm imm(6) Rn Rd
  // 0b000000
  // ADD (shifted register)
  // Add optionally-shifted register
  // adds a register value and an optionally-shifted register value, and writes the result to the destination register.

  // not support shift yet
  OPCodeTemplate opcode = is64bit ? 0x8b000000 : 0x0b000000;
  opcode |= (static_cast<OPCodeTemplate>(secondSrcReg) << 16U); // source 16-20
  opcode |= (static_cast<OPCodeTemplate>(firstSrcReg) << 5U);   // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);               // dest 0-4
  append(opcode);
}
void Assembler::adc_r_r_r(REG const destReg, REG const firstSrcReg, REG const secondSrcReg, bool const is64bit) {
  confirm(false, "don't know how to use yet");
  // sf 001101 0000 Rm 000000 Rn Rd
  // 0001 1010 000 Rm 000000 Rn Rd
  // 1a000000
  OPCodeTemplate opcode = is64bit ? 0x9a000000 : 0x1a000000;    // ADC Xd, Xn, Xm
  opcode |= (static_cast<OPCodeTemplate>(secondSrcReg) << 16U); // source 16-20
  opcode |= (static_cast<OPCodeTemplate>(firstSrcReg) << 5U);   // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);               // dest 0-4
  append(opcode);
}
void Assembler::sub_r_r_imm(REG const destReg, REG const srcReg, uint32_t const imm, bool const is64bit) {
  // sf 1 0 100010 sh imm12 Rn Rd
  // 0101 0001 0000
  // 61000000
  // 1101 0001 0000
  // d1000000
  confirm(is64bit, "currently always use full register 64bits");
  OPCodeTemplate opcode = 0xd1000000;                    // SUB Xd, Xn, #imm
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 5U); // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);        // dest 0-4
  opcode |= (imm & 0xFFFU) << 10U;
  append(opcode);
}
void Assembler::sub_r_r_immReg(REG const destReg, REG const srcReg, REG const immReg, bool const is64bit) {
  // sf 100 1011 001 Rm option(011) immShift(3) Rn Rd
  // 4B206000

  OPCodeTemplate opcode = is64bit ? 0xCB206000 : 0x4B206000; // SUB Xd, Xn, Xm
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 5U);
  opcode |= static_cast<OPCodeTemplate>(destReg);
  opcode |= (static_cast<OPCodeTemplate>(immReg) << 16U);
  append(opcode);
}
void Assembler::sub_r_r_shiftR(REG const destReg, REG const firstSrcReg, REG const secondSrcReg, bool const is64bit) {
  // Subtract optionally-shifted register
  // subtracts an optionally-shifted register value from a register value, and writes the result to the destination register.

  // sf 100 1011 shift(2) 0 Rm imm(6) Rn Rd
  // cb000000
  // shift is not supported yet
  OPCodeTemplate opcode = is64bit ? 0xcb000000 : 0x4b000000;
  opcode |= (static_cast<OPCodeTemplate>(secondSrcReg) << 16U); // source 16-20
  opcode |= (static_cast<OPCodeTemplate>(firstSrcReg) << 5U);   // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);               // dest 0-4
  append(opcode);
}
void Assembler::inc_sp(uint32_t const imm) {
  add_r_r_imm(REG::SP, REG::SP, imm, true);
}
void Assembler::dec_sp(uint32_t const imm) {
  sub_r_r_imm(REG::R9, REG::SP, imm, true);
  cmp_r_r(REG::R9, StackGuard, true);
  Relpatch const notStackOverflow = prepareJmp(CC::HI);
  setTrap(Trapcode::Stack_overflow);
  notStackOverflow.linkToHere();
  // if SP > StackGuard, continue, update SP
  sub_r_r_imm(REG::SP, REG::SP, imm, true);
}
void Assembler::mov_r_r(REG const destReg, REG const srcReg, bool const is64bit) {
  // sf 010 1010 000 Rm 000000 11111 Rd
  // 2A0003E0
  OPCodeTemplate opcode = is64bit ? 0xAA0003E0 : 0x2A0003E0;
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 16U);
  opcode |= static_cast<OPCodeTemplate>(destReg);
  append(opcode);
}
void Assembler::movk_r_imm16(REG const destReg, uint16_t const imm, uint8_t const shift, bool const is64bit) {
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
  confirm(imm <= 0xFFFFU, "Immediate out of range");
  OPCodeTemplate opcode = is64bit ? 0xf2800000 : 0x72800000;
  confirm(shift <= (is64bit ? 3U : 1U), "Shift out of range");
  opcode |= (static_cast<OPCodeTemplate>(destReg)); // dest 0-4
  opcode |= (imm & 0xFFFFU) << 5U;                  // immediate value 5-20
  opcode |= (shift & 0x3U) << 21U;                  // shift value 21-22
  append(opcode);
}
void Assembler::mov_r_imm16(REG const destReg, uint16_t const imm, bool const is64bit) {
  // MOV (wide immediate)
  // sf 10(opc) 100101 hw2 imm16 Rd
  // 1101 0010 100 imm16 00000
  // 0xd280000
  // 1011 0010 0000
  // 1 01 100100
  confirm(imm <= 0xFFFFU, "Immediate out of range");
  OPCodeTemplate opcode = is64bit ? 0xd2800000 : 0x52800000;
  opcode |= (static_cast<OPCodeTemplate>(destReg)); // dest 0-4
  opcode |= (imm & 0xFFFFU) << 5U;                  // immediate value 5-20
  append(opcode);
}

void Assembler::mul_r_r(REG const destReg, REG const firstSrcReg, REG const secondSrcReg, bool const is64bit) {
  // sf 001 1011 000 Rm 0111 11 Rn Rd
  // 1b007c00
  OPCodeTemplate opcode = is64bit ? 0x9b007c00 : 0x1b007c00;
  opcode |= (static_cast<OPCodeTemplate>(secondSrcReg) << 16U); // source 16-20
  opcode |= (static_cast<OPCodeTemplate>(firstSrcReg) << 5U);   // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);               // dest 0-4
  append(opcode);
}

void Assembler::cmp_r_r(REG const firstSrcReg, REG const secondSrcReg, bool const is64bit) {
  // sf 110 1011 shift(2) 0 Rm imm(6) Rn Rd(11111)
  // 6b00001f
  OPCodeTemplate opcode = is64bit ? 0xeb00001f : 0x6b00001f;
  opcode |= (static_cast<OPCodeTemplate>(secondSrcReg) << 16U); // source 16-20
  opcode |= (static_cast<OPCodeTemplate>(firstSrcReg) << 5U);   // source 5-9
  append(opcode);
}
void Assembler::cmp_r_imm(REG const firstSrcReg, uint32_t const imm, bool const is64bit) {
  // sf 111 0001 0 sh(1) imm12 Rn Rd(11111)
  // 7100001f
  confirm(imm <= 0xFFFU, "Immediate out of range 12");
  OPCodeTemplate opcode = is64bit ? 0xf100001f : 0x7100001f;
  opcode |= (static_cast<OPCodeTemplate>(firstSrcReg) << 5U); // source 5-9
  opcode |= (imm & 0xFFFU) << 10U;                            // immediate value 10-21
  append(opcode);
}

void Assembler::prepare_b_cond(CC const condition) {
  // 0101 0100 imm19 0 cond(4)
  // 54000000
  OPCodeTemplate opcode = 0x54000000;               // B.cc
  opcode |= static_cast<OPCodeTemplate>(condition); // condition 0-3
  append(opcode);
}
void Assembler::prepare_b() {
  // 0001 01 imm26
  // 14000000
  constexpr const OPCodeTemplate opcode = 0x14000000;
  append(opcode);
}
void Assembler::prepare_bl() {
  // 100101 imm26
  // 94000000
  constexpr const OPCodeTemplate opcode = 0x94000000;
  append(opcode);
}
void Assembler::brk() {
  // 1101 0100 001 imm16 00000
  // d4200000
  constexpr const OPCodeTemplate opcode = 0xd4200000;
  // Use regiter to store trap code before brk, so brk's imm is unused
  append(opcode);
}

void Assembler::sdiv_r_r(REG const destReg, REG const firstSrcReg, REG const secondSrcReg, bool const is64bit) {
  // sf 001 1010 110 Rm 000011 Rn Rd
  // 1ac00c00
  OPCodeTemplate opcode = is64bit ? 0x9ac00c00 : 0x1ac00c00;
  opcode |= (static_cast<OPCodeTemplate>(secondSrcReg) << 16U); // source 16-20
  opcode |= (static_cast<OPCodeTemplate>(firstSrcReg) << 5U);   // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);               // dest 0-4
  append(opcode);
}
void Assembler::udiv_r_r(REG const destReg, REG const firstSrcReg, REG const secondSrcReg, bool const is64bit) {
  // sf 001 1010 110 Rm 000010 Rn Rd
  // 1ac00800
  OPCodeTemplate opcode = is64bit ? 0x9ac00800 : 0x1ac00800;
  opcode |= (static_cast<OPCodeTemplate>(secondSrcReg) << 16U); // source 16-20
  opcode |= (static_cast<OPCodeTemplate>(firstSrcReg) << 5U);   // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);               // dest 0-4
  append(opcode);
}

void Assembler::rBits_r_r(REG const destReg, REG const srcReg, bool const is64bit) {
  // sf 101 1010 1100 ... Rn Rd
  // 5ac00000 dac00000
  OPCodeTemplate opcode = is64bit ? 0xdac00000 : 0x5ac00000;
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 5U); // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);        // dest 0-4
  append(opcode);
}
void Assembler::clz_r_r(REG const destReg, REG const srcReg, bool const is64bit) {
  // sf 101 1010 1100 0000 0001 00 Rn Rd
  // 5ac01000 dac01000
  OPCodeTemplate opcode = is64bit ? 0xdac01000 : 0x5ac01000;
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 5U); // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);        // dest 0-4
  append(opcode);
}

void Assembler::lsr_imm(REG const destReg, REG const srcReg, uint32_t const shift, bool const is64bit) {
  // sf 101 0011 0 N immr(6) x11111 Rn Rd
  // 1101 0011 01 immr(6) 111111 Rn Rd -> D340FC00 for 64bits
  // 0101 0011 00 immr(6) 011111 Rn Rd -> 53007C00 for 32bits
  confirm(shift <= (is64bit ? 63U : 31U), "Shift out of range");
  OPCodeTemplate opcode = is64bit ? 0xD340FC00 : 0x53007C00;
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 5U); // source 5-9
  opcode |= static_cast<OPCodeTemplate>(destReg);        // dest 0-4
  opcode |= (shift & 0x3FU) << 16U;
  append(opcode);
}

void Assembler::blr(REG const srcReg) {
  // 1101 0110 0011 1111 0000 00 Rn 00000
  // D63F0000
  OPCodeTemplate opcode = 0xD63F0000;                    // BLR Xn
  opcode |= (static_cast<OPCodeTemplate>(srcReg) << 5U); // source 5-9
  append(opcode);
}

/////////////////////////////////////////////////////////////////
//< Customized instructions
/////////////////////////////////////////////////////////////////

void Assembler::emit_mov_x_imm64(REG const destReg, uint64_t const imm) {
  mov_r_imm16(destReg, imm & 0xFFFFU, true);
  uint16_t const shift1 = (imm >> 16U) & 0xFFFFU;
  if (shift1 != 0) {
    movk_r_imm16(destReg, shift1, 1, true);
  }
  uint16_t const shift2 = (imm >> 32U) & 0xFFFFU;
  if (shift2 != 0) {
    movk_r_imm16(destReg, shift2, 2, true);
  }
  uint16_t const shift3 = (imm >> 48U) & 0xFFFFU;
  if (shift3 != 0) {
    movk_r_imm16(destReg, shift3, 3, true);
  }
}
void Assembler::emit_mov_w_imm32(REG const destReg, uint32_t const imm) {
  mov_r_imm16(destReg, imm & 0xFFFFU, false);
  uint16_t const shift1 = (imm >> 16U) & 0xFFFFU;
  if (shift1 != 0U) {
    // If the upper 16 bits are not zero, we need to move them as well
    movk_r_imm16(destReg, shift1, 1, false);
  }
}

void Assembler::decreaseSPWithClean(uint32_t const bytes) {
  confirm(bytes % 16 == 0, "[decreaseSPWithClean]size must be aligned to 16 bytes");
  // TODO(): support larger size
  confirm((bytes < static_cast<uint32_t>(INT32_MAX)), "size must be less than int32_t max value since str_base_byteOff offset max support 32bit");
  dec_sp(bytes);

  ///< Clean the stack memory. TODO(): how to simplify this?
  // R9 used as scratch register
  mov_r_imm16(REG::R9, 0U, true);
  uint32_t const bytesPerStr = 8U; // 64bits register, 8 bytes per str_base_byteOff
  for (uint32_t i = 0; i < bytes / bytesPerStr; ++i) {
    uint32_t const offset = static_cast<uint32_t>(i * 8U);
    str_base_byteOff(REG::SP, REG::R9, offset, true);
  }
}

void Assembler::set_b_cond_off(uint32_t const b_instructionPositionOffsetToOutputBinary, int32_t const condOffset) {
  confirm(outputBinary_.getSize() >= b_instructionPositionOffsetToOutputBinary + 4U, "must have the b instruction");

  OPCodeTemplate opcode = outputBinary_.get<OPCodeTemplate>(b_instructionPositionOffsetToOutputBinary);
  // TODO(): Currently, only support b.cond
  confirm((opcode & static_cast<OPCodeTemplate>(0x54000000)) == static_cast<OPCodeTemplate>(0x54000000), "");

  // 0101 0100 imm19 0 cond(4)
  // 54000000
  confirm(((condOffset >= static_cast<int32_t>(-(0x7ffff + 1))) && (condOffset <= static_cast<int32_t>(0x7ffff))),
          "Offset out of range signed 19 for branch instruction");
  opcode |= (static_cast<OPCodeTemplate>(condOffset) & 0x7FFFFU) << 5U; // offset 5-24

  outputBinary_.set(b_instructionPositionOffsetToOutputBinary, opcode);
}

void Assembler::set_b_off(uint32_t const b_instructionPositionOffsetToOutputBinary, int32_t const offset) {
  confirm(outputBinary_.getSize() >= b_instructionPositionOffsetToOutputBinary + 4U, "must have the b instruction");
  OPCodeTemplate opcode = outputBinary_.get<OPCodeTemplate>(b_instructionPositionOffsetToOutputBinary);
  // TODO(): Currently, only support b
  confirm((opcode & static_cast<OPCodeTemplate>(0x14000000)) == static_cast<OPCodeTemplate>(0x14000000), "");

  // 0001 01 imm26
  // 0x14000000
  confirm(((offset >= static_cast<int32_t>(-(0x3ffffff + 1))) && (offset <= static_cast<int32_t>(0x3ffffff))),
          "Offset out of range signed 26 for branch instruction");
  opcode |= static_cast<OPCodeTemplate>(offset) & 0x3FFFFFFU;

  outputBinary_.set(b_instructionPositionOffsetToOutputBinary, opcode);
}

void Assembler::set_bl_off(uint32_t const bl_instructionPositionOffsetToOutputBinary, int32_t const offset) {
  confirm(outputBinary_.getSize() >= bl_instructionPositionOffsetToOutputBinary + 4U, "must have the bl instruction");
  OPCodeTemplate opcode = outputBinary_.get<OPCodeTemplate>(bl_instructionPositionOffsetToOutputBinary);
  confirm((opcode & static_cast<OPCodeTemplate>(0x94000000)) == static_cast<OPCodeTemplate>(0x94000000), "");
  confirm(((offset >= static_cast<int32_t>(-(0x3ffffff + 1))) && (offset <= static_cast<int32_t>(0x3ffffff))),
          "Offset out of range signed 26 for branch instruction");
  opcode |= static_cast<OPCodeTemplate>(offset) & 0x3FFFFFFU;

  outputBinary_.set(bl_instructionPositionOffsetToOutputBinary, opcode);
}

void Assembler::setTrap(Trapcode const trapcode) {
  mov_r_imm16(REG::R0, static_cast<uint32_t>(trapcode), false);
  brk();
}

Relpatch Assembler::prepareJmp(CC const condition) {
  if (condition == CC::NONE) {
    // unconditional jump
    Relpatch patch{*this, false};
    prepare_b();
    return patch;
  } else {
    Relpatch patch{*this, true};
    prepare_b_cond(condition);
    return patch;
  }
}