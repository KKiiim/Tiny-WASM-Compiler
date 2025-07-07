#ifndef SRC_COMMON_CONSTANT_H
#define SRC_COMMON_CONSTANT_H

#include <cstdint>
#include <string>
#include <unordered_map>

constexpr const uint32_t DefaultPageSize = 64 * 1024; // 64 KB
constexpr const uint32_t MaxParamsForWasmFunction = 7U;
constexpr const uint32_t stackGuardSize = 64 * 1024;   // 64KB stack guard size
constexpr const uint32_t MaxLinearMemoryPages = 1024U; // 1024 pages
///< Max 64MB linear memory size
static_assert((static_cast<uint64_t>(DefaultPageSize) * MaxLinearMemoryPages) <= static_cast<uint64_t>(UINT32_MAX), "should within 32bits");

enum Trapcode : uint32_t {
  NONE = 0,
  DIV_0 = 1,
  Integer_overflow = 2,
  TableElement_out_of_range = 3,
  IndirectCall_signature_mismatch = 4,
  Stack_overflow = 5,
  Out_of_bounds_memory_access = 6,
};
const std::unordered_map<Trapcode, std::string> trapcodeString{
    {Trapcode::NONE, "NONE"},
    {Trapcode::DIV_0, "integer divide by zero"},
    {Trapcode::Integer_overflow, "integer overflow"},
    {Trapcode::TableElement_out_of_range, "undefined element"},
    {Trapcode::IndirectCall_signature_mismatch, "indirect call type mismatch"},
    {Trapcode::Stack_overflow, "call stack exhausted"},
    {Trapcode::Out_of_bounds_memory_access, "out of bounds memory access"},
};
#endif