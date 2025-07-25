///
/// Instruction encoding for AArch64
///

#ifndef AARCH64_ENCODING_HPP
#define AARCH64_ENCODING_HPP

#include <cstdint>

#include "src/common/util.hpp"

///
/// @brief Native registers and their encoding that can be placed into the respective fields in an instruction
/// NOTE: REG::NONE will be used to represent an invalid register (or no register at all)
///
enum class REG : uint32_t { // clang-format off
  R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15, R16, R17, R18, R19, R20, R21, R22, R23, R24, 
  R25, R26, R27, R28, FP, LR, ZR, SP = ZR, F0 = 0b0010'0000, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, 
  F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25, F26, F27, F28, F29, F30, F31, NUMREGS, NONE = 0b1000'0000
}; // clang-format on

constexpr uint32_t totalNumRegs{static_cast<uint32_t>(REG::NUMREGS)}; ///< Total number of registers in the enum

///////////////////////////////////////////////////////////////////
// R28 always point the next free space in the stack
static constexpr const REG ROP = REG::R28;
// R27 points to the start of global memory
static constexpr const REG GLOBAL = REG::R27;
// R26 points to base of linear memory
static constexpr const REG LinMem = REG::R26;
// R25 stores the bytes size of linear memory
static constexpr const REG SizeLinMem = REG::R25;
// R24 stores the stack guard address
static constexpr const REG StackGuard = REG::R24;
///////////////////////////////////////////////////////////////////

namespace RegUtil {
///
/// @brief Checks whether a register is a general purpose register (as opposed to a floating point register)
///
/// @param reg Register to check
/// @return bool Whether the register is a general purpose register
inline bool isGPR(REG const reg) {
  return (static_cast<uint32_t>(reg) & 0b10'0000U) == 0U;
}

} // namespace RegUtil

///
/// @brief AArch64 CPU condition codes
///
/// @note
///    Condition   |  Meaning               |  Notes
///       EQ       |  equal                 |  Equal
///       NE       |  not equal             |  Not equal
///       CS       |  carry set             |  Carry set
///       HS       |  high or same          |  Unsigned higher or same
///       CC       |  carry clear           |  Carry clear
///       LO       |  low                   |  Unsigned lower
///       MI       |  minus                 |  Negative
///       PL       |  plus                  |  Positive or zero
///       VS       |  overflow set          |  Signed overflow
///       VC       |  overflow clear        |  No signed overflow
///       HI       |  high                  |  Unsigned higher
///       LS       |  low or same           |  Unsigned lower or same
///       GE       |  greater than or equal |  Signed greater than or equal
///       LT       |  less than             |  Signed less than
///       GT       |  greater than          |  Signed greater than
///       LE       |  less than or equal    |  Signed less than or equal
///       AL       |  always                |  Always executed (unconditional)
///
enum class CC : uint8_t { EQ, NE, CS, HS = CS, CC, LO = CC, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE, AL, NV, NONE = 0xFF };

#endif // AARCH64_ENCODING_HPP