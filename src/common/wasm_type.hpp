#ifndef SRC_COMMON_WASM_TYPE_H
#define SRC_COMMON_WASM_TYPE_H

#include <cstdint>

///
/// @brief Type representing the different types available in WebAssembly as their binary encoding plus a
/// WasmType::INVALID encoding used by the compiler for internal purposes
///
enum class WasmType : uint8_t {
  EXTERN_REF = 0x6F,
  FUNC_REF = 0x70,
  VEC_TYPE = 0x7B,
  F64 = 0x7C,
  F32 = 0x7D,
  I64 = 0x7E,
  I32 = 0x7F,
  TVOID = 0x40,
  INVALID = 0x00
};

/// @brief WebAssembly section IDs
enum class SectionType : uint8_t {
  CUSTOM = 0,
  TYPE,
  IMPORT,
  FUNCTION,
  TABLE,
  MEMORY,
  GLOBAL,
  EXPORT,
  START,
  ELEMENT,
  CODE,
  DATA,
  DATA_COUNT,
  PLACEHOLDER ///< module end
};

enum class WasmImportExportType : uint8_t { FUNC = 0x00, TABLE = 0x01, MEM = 0x02, GLOBAL = 0x03 };

enum class OPCode : uint8_t {
  // CONTROL FLOW OPERATORS
  UNREACHABLE = 0x00,
  NOP = 0x01,
  BLOCK = 0x02,
  LOOP = 0x03,
  IF = 0x04,
  ELSE = 0x05,

  END = 0x0B,
  BR = 0x0C,
  BR_IF = 0x0D,
  BR_TABLE = 0x0E,
  RETURN = 0x0F,

  // CALL OPERATORS
  CALL = 0x10,
  CALL_INDIRECT = 0x11,

  // REFERENCE OPERATORS
  REF_NULL = 0xD0,
  REF_IS_NULL = 0xD1,
  REF_FUNC = 0xD2,

  // PARAMETRIC OPERATORS
  DROP = 0x1A,
  SELECT = 0x1B,
  SELECT_T = 0x1C,

  // VARIABLE ACCESS
  LOCAL_GET = 0x20,
  LOCAL_SET = 0x21,
  LOCAL_TEE = 0x22,
  GLOBAL_GET = 0x23,
  GLOBAL_SET = 0x24,

  // TABLE OPERATORS
  TABLE_GET = 0x25,
  TABLE_SET = 0x26,

  // MEMORY-RELATED OPERATOR
  I32_LOAD = 0x28,
  I64_LOAD = 0x29,
  F32_LOAD = 0x2A,
  F64_LOAD = 0x2B,
  I32_LOAD8_S = 0x2C,
  I32_LOAD8_U = 0x2D,
  I32_LOAD16_S = 0x2E,
  I32_LOAD16_U = 0x2F,
  I64_LOAD8_S = 0x30,
  I64_LOAD8_U = 0x31,
  I64_LOAD16_S = 0x32,
  I64_LOAD16_U = 0x33,
  I64_LOAD32_S = 0x34,
  I64_LOAD32_U = 0x35,
  I32_STORE = 0x36,
  I64_STORE = 0x37,
  F32_STORE = 0x38,
  F64_STORE = 0x39,
  I32_STORE8 = 0x3A,
  I32_STORE16 = 0x3B,
  I64_STORE8 = 0x3C,
  I64_STORE16 = 0x3D,
  I64_STORE32 = 0x3E,
  MEMORY_SIZE = 0x3F,
  MEMORY_GROW = 0x40,

  // CONSTANTS
  I32_CONST = 0x41,
  I64_CONST = 0x42,
  F32_CONST = 0x43,
  F64_CONST = 0x44,

  // COMPARISON OPERATORS + INVERTED CMP OPCODE
  I32_EQZ = 0x45,  // UNREACHABLE
  I32_EQ = 0x46,   // I32_NE
  I32_NE = 0x47,   // I32_EQ
  I32_LT_S = 0x48, // I32_GE_S
  I32_LT_U = 0x49, // I32_GE_U
  I32_GT_S = 0x4A, // I32_LE_S
  I32_GT_U = 0x4B, // I32_LE_U
  I32_LE_S = 0x4C, // I32_GT_S
  I32_LE_U = 0x4D, // I32_GT_U
  I32_GE_S = 0x4E, // I32_LT_S
  I32_GE_U = 0x4F, // I32_LT_U

  I64_EQZ = 0x50,  // UNREACHABLE
  I64_EQ = 0x51,   // I64_NE
  I64_NE = 0x52,   // I64_EQ
  I64_LT_S = 0x53, // I64_GE_S
  I64_LT_U = 0x54, // I64_GE_U
  I64_GT_S = 0x55, // I64_LE_S
  I64_GT_U = 0x56, // I64_LE_U
  I64_LE_S = 0x57, // I64_GT_S
  I64_LE_U = 0x58, // I64_GT_U
  I64_GE_S = 0x59, // I64_LT_S
  I64_GE_U = 0x5A, // I64_LT_U

  // NUMERIC OPERATORS
  I32_CLZ = 0x67,
  I32_CTZ = 0x68,
  I32_POPCNT = 0x69,
  I32_ADD = 0x6A,
  I32_SUB = 0x6B,
  I32_MUL = 0x6C,
  I32_DIV_S = 0x6D,
  I32_DIV_U = 0x6E,
  I32_REM_S = 0x6F,
  I32_REM_U = 0x70,
  I32_AND = 0x71,
  I32_OR = 0x72,
  I32_XOR = 0x73,
  I32_SHL = 0x74,
  I32_SHR_S = 0x75,
  I32_SHR_U = 0x76,
  I32_ROTL = 0x77,
  I32_ROTR = 0x78,

  I64_CLZ = 0x79,
  I64_CTZ = 0x7A,
  I64_POPCNT = 0x7B,
  I64_ADD = 0x7C,
  I64_SUB = 0x7D,
  I64_MUL = 0x7E,
  I64_DIV_S = 0x7F,
  I64_DIV_U = 0x80,
  I64_REM_S = 0x81,
  I64_REM_U = 0x82,
  I64_AND = 0x83,
  I64_OR = 0x84,
  I64_XOR = 0x85,
  I64_SHL = 0x86,
  I64_SHR_S = 0x87,
  I64_SHR_U = 0x88,
  I64_ROTL = 0x89,
  I64_ROTR = 0x8A,
};

#endif