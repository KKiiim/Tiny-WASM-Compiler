///
/// Definition of calling convention for AArch64
///
#ifndef SRC_BACKEND_AARCH64_CC_HPP
#define SRC_BACKEND_AARCH64_CC_HPP

#include <cstdint>

#include "aarch64_encoding.hpp"

#define COMMA ,

#if defined(__linux__) && !defined(__ANDROID__)
#define JUST_LINUX(X) X
#else
#define JUST_LINUX(X)
#endif

namespace aarch64 {

///
/// @brief Size of the automatic return address in bytes on the stack the CALL instruction pushes
///
constexpr uint32_t returnAddrWidth{0U};

namespace WasmABI {

/// @brief Number of register for GPR that will be reserved as "scratch" registers and can be used for various
/// calculations on the fly and to hold variables Those registers will be taken from the end of the gpr array
constexpr uint32_t resScratchRegsGPR{5U};

/// @brief Number of register for FPR that will be reserved as "scratch" registers and can be used for various
/// calculations on the fly and to hold variables Those registers will be taken from the end of the fpr array
constexpr uint32_t resScratchRegsFPR{5U};

/// @brief At most, regsForParams (N) registers will be allocated in registers, the other parameters will be passed on
/// the stack
///
/// This also implicitly defines the calling convention that is used by the Wasm functions on the machine code level.
/// Any registers after that will also be used as scratch registers.
constexpr uint32_t regsForParams{8U};

/// @brief At most, gpRegsForReturnValues (N) return values will be allocated in general purpose registers, the other return values will be passed on
constexpr uint32_t gpRegsForReturnValues{2U};

/// @brief At most, fpRegsForReturnValues (N) return values will be allocated in floating point registers, the other return values will be passed on
constexpr uint32_t fpRegsForReturnValues{2U};

namespace REGS {

#if LINEAR_MEMORY_BOUNDS_CHECKS
/// @brief Cache of linear memory size in bytes (minus 8) for increased performance for memory bounds checks, can also
/// be used by emitMove
constexpr REG memSize{REG::R27};
#else
/// @brief Register that the landing pad can use as a scratch register, can also be used by emitMove
constexpr REG landingPadHelper{REG::R27};
#endif
// R28-> start | operand stack start address
constexpr REG jobMem{REG::R28}; ///< Pointer to base of job memory
constexpr REG linMem{REG::FP};  ///< Pointer to base of linear memory
constexpr REG trapReg{REG::R0}; ///< Register for internal usage where the trap indicator will be passed

constexpr REG trapPosReg{REG::R1};      ///< Register indicating where trap happened as position in bytecode (set only in debug mode)
constexpr REG indirectCallReg{REG::R0}; ///< Register for internal usage where the indirect call index will be passed

constexpr auto gpRetRegs = make_array(REG::R0, REG::R26); ///< General purpose return registers of Wasm functions

constexpr auto fpRetRegs = make_array(REG::F0, REG::F26); ///< Floating point return registers of Wasm functions
constexpr REG moveHelper{REG::F31};                       ///< Helper register for memory->memory emitMove

} // namespace REGS

// The order is defined by the following rules:
// 1. Return value register (that should be the same as in the native ABI) should be among the reserved scratch
// registers
// 2. The parameter registers should be as congruent as possible with the native ABI parameter registers (here: R0-R7,
// F0-F7)
//    Since R0 and F0 should be allocated as reserved scratch regs according to rule 1, we replace it with some other
//    volatile register (according to native ABI)
// 3. The rest will simply be allocated in order (or otherwise arbitrarily)

// NOTE: in linux x18 is used as temporary register, see gcc\config\aarch64\aarch64.h, on other platforms it is reserved
// and should not be used

///
/// @brief Array of usable general purpose registers with no dedicated content (unlike SP, LR, memSize etc.)
///
constexpr auto gpr = make_array(
    REG::R19, REG::R8, REG::R1, REG::R2, REG::R3, REG::R4, REG::R5, REG::R6, REG::R7, REG::R12, REG::R13, REG::R14, REG::R15, REG::R16, REG::R17,
    /* APPLE reserve register x18.
       https://developer.apple.com/documentation/xcode/writing-arm64-code-for-apple-platforms#Respect-the-purpose-of-specific-CPU-registers
     */
    JUST_LINUX(REG::R18 COMMA) REG::R20, REG::R21, REG::R22, REG::R23, REG::R24, REG::R25,
    /* REG::R27 for landingPadHelper or memSize */
    REG::R0, REG::R26, REG::R9, REG::R10, REG::R11); // <-- Last 5 reserved as scratch registers

///
/// @brief Array of usable floating point registers with no dedicated content
///
constexpr auto fpr =
    make_array(REG::F8, REG::F1, REG::F2, REG::F3, REG::F4, REG::F5, REG::F6, REG::F7, REG::F12, REG::F13, REG::F14, REG::F15, REG::F16, REG::F17,
               REG::F18, REG::F19, REG::F20, REG::F21, REG::F22, REG::F23, REG::F24, REG::F25, REG::F27, REG::F28, REG::F29, REG::F30, //

               REG::F0, REG::F26, REG::F9, REG::F10, REG::F11); // <-- Last 5 reserved as scratch registers

static_assert((fpr.size() <= UINT8_MAX) && (gpr.size() <= UINT8_MAX), "Array too long");
static_assert(resScratchRegsGPR >= 5U, "Need a minimum of 4 scratch registers");
static_assert(resScratchRegsFPR >= 5U, "Need a minimum of 4 scratch registers");

constexpr uint32_t numGPR{static_cast<uint32_t>(gpr.size())}; ///< Total number of GPRs available for allocation
constexpr uint32_t numFPR{static_cast<uint32_t>(fpr.size())}; ///< Total number of FPRs available for allocation

///
/// @brief Get the position in the gpr or fpr array for a register
///
/// @param reg Register to look up
/// @return Position of this register in the gpr or fpr array
uint32_t getRegPos(REG const reg);

///
/// @brief Check whether a register is a reserved scratch register
///
/// @param reg Register to check
/// @return true if register is a reserved scratch register, false otherwise
bool isResScratchReg(REG const reg);

} // namespace WasmABI

// Definition of the calling convention the native C++ code is using and corresponds with the AArch64 ABI
// This is necessary because we are going to call imported (native) C++ functions from Wasm code
// gpParams and fpParams define (in order) in which registers GP and FP parameters for function calls are passed
namespace NativeABI {

///
/// @brief General purpose registers for passing params in the native ABI
///
constexpr auto gpParams = make_array(REG::R0, REG::R1, REG::R2, REG::R3, REG::R4, REG::R5, REG::R6, REG::R7);
///
/// @brief Floating point registers for passing params in the native ABI
///
constexpr auto fpParams = make_array(REG::F0, REG::F1, REG::F2, REG::F3, REG::F4, REG::F5, REG::F6, REG::F7);
///
/// @brief Nonvolatile registers in the native ABI. Callee save
///
constexpr auto nonvolRegs = make_array(REG::R19, REG::R20, REG::R21, REG::R22, REG::R23, REG::R24, REG::R25, REG::R26, REG::R27, REG::R28, REG::F8,
                                       REG::F9, REG::F10, REG::F11, REG::F12, REG::F13, REG::F14, REG::F15, REG::LR, REG::FP);
///
/// @brief Volatile registers in the native ABI. Callee don't need to save
///
constexpr auto volRegs = make_array(REG::R0, REG::R1, REG::R2, REG::R3, REG::R4, REG::R5, REG::R6, REG::R7, REG::R8, REG::R9, REG::R10, REG::R11,
                                    REG::R12, REG::R13, REG::R14, REG::R15, REG::R16, REG::R17, JUST_LINUX(REG::R18 COMMA) REG::F0, REG::F1, REG::F2,
                                    REG::F3, REG::F4, REG::F5, REG::F6, REG::F7, REG::F16, REG::F17, REG::F18, REG::F19, REG::F20, REG::F21, REG::F22,
                                    REG::F23, REG::F24, REG::F25, REG::F26, REG::F27, REG::F28, REG::F29, REG::F30, REG::F31);

/// @brief General purpose return register in the native ABI
constexpr REG gpRetReg{REG::R0};
/// @brief Floating point return register in the native ABI
constexpr REG fpRetReg{REG::F0};

///
/// @brief Check whether a register is a volatile register in the native ABI
///
/// @param reg Register to check
/// @return True if the register is volatile
bool isVolatileReg(REG const reg);

///
/// @brief Check whether a register can be a parameter in the native ABI
///
/// @param reg Register to check
/// @return True if the register can be a parameter
bool canBeParam(REG const reg);

} // namespace NativeABI

///
/// @brief List of registers that can be used as scratch registers during a function call
///
/// Can be used during indirect calls, imported calls and Wasm calls
/// These registers are never params (neither in WasmABI nor in NativeABI), return registers or indirect call index
/// registers and are thus never used during function calls
///
constexpr auto callScrRegs = make_array(REG::R9, REG::R10, REG::R11);
static_assert(callScrRegs.size() >= 3, "Minimum 3 scratch registers needed for calls");

} // namespace aarch64

#endif