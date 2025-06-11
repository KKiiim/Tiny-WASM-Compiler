#ifndef SRC_BACKEND_AARCH64ASSEMBLER_H
#define SRC_BACKEND_AARCH64ASSEMBLER_H

#include <cstdint>
#include <cstdlib>

#include "aarch64_encoding.hpp"

#include "src/common/ExecutableMemory.hpp"
#include "src/common/wasm_type.hpp"

///
/// @brief Basic template for AArch64 OPCodes
///
using OPCodeTemplate = uint32_t;

/// @brief 9 bits signed immediate for LDR/STR instructions, max value is 255
uint8_t constexpr const MaxPositiveImmForLdrStr = 0xFFU;

class Assembler {
public:
  Assembler();
  ~Assembler();

  Assembler(Assembler const &) = delete;
  Assembler &operator=(Assembler const &) = delete;
  Assembler(Assembler &&) = delete;
  Assembler &operator=(Assembler &&) = delete;

  ExecutableMemory getExecutableMemory();
  uint32_t getCurrentOffset() const {
    return size_;
  }

  /////////////////////////////////////////////////////////////////
  //< Basic instructions
  /////////////////////////////////////////////////////////////////

  void ret();
  /// @brief LDR <Wt>, [<Xn|SP>{, #<pimm>}]. Unsigned offset
  void ldr_base_off(REG const destReg, REG const addrReg, uint32_t const offset, bool const is64bit);
  /// @brief STR <Wt>, [<Xn|SP>{, #<pimm>}]. Unsigned offset
  void str_base_off(REG const addrReg, REG const srcReg, uint32_t const offset, bool const is64bit);
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
  void mov_r_r(REG const destReg, REG const srcReg);
  /// @brief MOV R[d], imm. Can shift and not keep, but not supported yet
  void mov_r_imm16(REG const destReg, uint16_t const imm, bool const is64bit);
  ///@brief MOVK R[d], imm, shift. K means keeping other bits unchanged
  void movk_r_imm16(REG const destReg, uint16_t const imm, uint8_t const shift, bool const is64bit);

  void mul_r_r(REG const destReg, REG const firstSrcReg, REG const secondSrcReg, bool const is64bit);

  void cmp_r_r(REG const firstSrcReg, REG const secondSrcReg, bool const is64bit);
  void cmp_r_imm(REG const firstSrcReg, uint32_t const imm, bool const is64bit);

  /////////////////////////////////////////////////////////////////
  //< Customized instructions
  /////////////////////////////////////////////////////////////////

  void emit_mov_x_imm64(REG const destReg, uint64_t const imm);
  void emit_mov_w_imm32(REG const destReg, uint32_t const imm);
  void decreaseSPWithClean(uint32_t const bytes);

  void prepare_b_cond(CC const condition);
  void prepare_b();

  void set_b_cond_off(uint32_t const b_instructionPositionOffsetToOutputBinary, int32_t const condOffset);
  void set_b_off(uint32_t const b_instructionPositionOffsetToOutputBinary, int32_t const offset);

private:
  void append(OPCodeTemplate const ins);

private:
  uint8_t *data_;
  uint32_t size_;
};

#endif // SRC_BACKEND_AARCH64ASSEMBLER_H