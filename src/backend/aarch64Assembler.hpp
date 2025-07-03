#ifndef SRC_BACKEND_AARCH64ASSEMBLER_H
#define SRC_BACKEND_AARCH64ASSEMBLER_H

#include <cstdint>
#include <cstdlib>

#include "aarch64_encoding.hpp"

#include "src/common/ExecutableMemory.hpp"
#include "src/common/constant.hpp"

///
/// @brief Basic template for AArch64 OPCodes
///
using OPCodeTemplate = uint32_t;

/// @brief 9 bits signed immediate for LDR/STR instructions, max value is 255
uint8_t constexpr const MaxPositiveImmForLdrStr = 0xFFU;

class Relpatch;
class Assembler {
public:
  Assembler() : outputBinary_(DefaultPageSize) {
  }
  inline ExecutableMemory &getExecutableMemory() {
    return outputBinary_;
  }
  uint32_t getCurrentOffset() const {
    return outputBinary_.getSize();
  }
  uintptr_t getCurrentAbsAddress() const {
    return outputBinary_.getCurrentAbsAddress();
  }

  /////////////////////////////////////////////////////////////////
  //< Basic instructions
  /////////////////////////////////////////////////////////////////

  void ret();
  /// @brief LDR <Wt>, [<Xn|SP>{, #<pimm>}]. Unsigned offset
  void ldr_base_byteOff(REG const destReg, REG const addrReg, uint32_t const byteOffset, bool const is64bit);
  /// default shift 2 for 32bits(offset times 4), shift 3 for 64bits(offset times 8)
  void ldr_offReg(REG const destReg, REG const addrReg, REG const offsetReg, bool const is64bit);
  /// @brief STR <Wt>, [<Xn|SP>{, #<pimm>}]. Unsigned offset
  void str_base_byteOff(REG const addrReg, REG const srcReg, uint32_t const byteOffset, bool const is64bit);
  /// @brief ADD R[d], R[s], imm
  void add_r_r_imm(REG const destReg, REG const srcReg, uint32_t const uimm, bool const is64bit);
  /// @brief ADD R[d], R[n], R[m]
  void add_r_r_extendedR(REG const destReg, REG const firstSrcReg, REG const secondSrcReg, bool const is64bit);
  /// @brief ADD R[d], R[n], R[m]
  void add_r_r_shiftR(REG const destReg, REG const firstSrcReg, REG const secondSrcReg, bool const is64bit);
  /// @brief ADD R[d], R[n], R[m]. adds two register values and the Carry flag value, and writes the result to the destination register.
  void adc_r_r_r(REG const destReg, REG const firstSrcReg, REG const secondSrcReg, bool const is64bit);
  /// @brief SUB R[d], R[s], imm
  void sub_r_r_imm(REG const destReg, REG const srcReg, uint32_t const imm, bool const is64bit);
  /// @brief SUB R[d], R[s], R[imm]
  void sub_r_r_immReg(REG const destReg, REG const srcReg, REG const immReg);
  void sub_r_r_shiftR(REG const destReg, REG const firstSrcReg, REG const secondSrcReg, bool const is64bit);
  /// @brief Increment SP by imm
  void inc_sp(uint32_t const imm);
  /// @brief Decrement SP by imm
  void dec_sp(uint32_t const imm);
  /// @brief MOV R[d], R[s]
  void mov_r_r(REG const destReg, REG const srcReg, bool const is64bit);
  /// @brief MOV R[d], imm. Can shift and not keep, but not supported yet
  void mov_r_imm16(REG const destReg, uint16_t const imm, bool const is64bit);
  ///@brief MOVK R[d], imm, shift. K means keeping other bits unchanged
  void movk_r_imm16(REG const destReg, uint16_t const imm, uint8_t const shift, bool const is64bit);

  void mul_r_r(REG const destReg, REG const firstSrcReg, REG const secondSrcReg, bool const is64bit);

  void cmp_r_r(REG const firstSrcReg, REG const secondSrcReg, bool const is64bit);
  void cmp_r_imm(REG const firstSrcReg, uint32_t const imm, bool const is64bit);

  void brk();

  void sdiv_r_r(REG const destReg, REG const firstSrcReg, REG const secondSrcReg, bool const is64bit);
  void udiv_r_r(REG const destReg, REG const firstSrcReg, REG const secondSrcReg, bool const is64bit);

  /// @brief reverse bytes in a register
  void rBits_r_r(REG const destReg, REG const srcReg, bool const is64bit);
  /// @brief Count leading zeros in a register
  void clz_r_r(REG const destReg, REG const srcReg, bool const is64bit);

  void lsr_imm(REG const destReg, REG const srcReg, uint32_t const shift, bool const is64bit);

  void blr(REG const srcReg);

  /////////////////////////////////////////////////////////////////
  //< Customized instructions
  /////////////////////////////////////////////////////////////////

  void emit_mov_x_imm64(REG const destReg, uint64_t const imm);
  void emit_mov_w_imm32(REG const destReg, uint32_t const imm);
  void decreaseSPWithClean(uint32_t const bytes);

  void setTrap(uint32_t const trapcode);

  void prepare_b_cond(CC const condition);
  void prepare_b();
  /// @brief condOffset: offset from the address of this instruction, in the range +/-128MB, is encoded as "imm26" times 4.
  /// Which means the offset is instruction position offset to output binary.
  void set_b_cond_off(uint32_t const b_instructionPositionOffsetToOutputBinary, int32_t const condOffset);
  /// @brief offset: offset from the address of this instruction, in the range +/-1MB, is encoded as "imm19" times 4.
  /// Which means the offset is instruction position offset to output binary.
  void set_b_off(uint32_t const b_instructionPositionOffsetToOutputBinary, int32_t const offset);

  void prepare_bl();
  /// @brief offset from the address of this instruction, in the range +/-128MB, is encoded as "imm26" times 4.
  void set_bl_off(uint32_t const bl_instructionPositionOffsetToOutputBinary, int32_t const offset);

  Relpatch prepareJmp(CC const condition);

private:
  void append(OPCodeTemplate const ins);

private:
  ExecutableMemory outputBinary_;
};

#endif // SRC_BACKEND_AARCH64ASSEMBLER_H