#ifndef SRC_COMMON_CONSTANT_H
#define SRC_COMMON_CONSTANT_H

#include <cstdint>
#include <string>
#include <unordered_map>

constexpr const uint32_t DefaultPageSize = 4 * 1024; // 4 KB
constexpr const uint32_t MaxParamsForWasmFunction = 7U;
constexpr const uint32_t stackGuardSize = 64 * 1024; // 64KB stack guard size

enum Trapcode : uint32_t {
  NONE = 0,
  DIV_0 = 1,
  Integer_overflow = 2,
  TableElement_out_of_range = 3,
  IndirectCall_signature_mismatch = 4,
  Stack_overflow = 5,
};
const std::unordered_map<Trapcode, std::string> trapcodeString{
    {Trapcode::NONE, "NONE"},
    {Trapcode::DIV_0, "integer divide by zero"},
    {Trapcode::Integer_overflow, "integer overflow"},
    {Trapcode::TableElement_out_of_range, "undefined element"},
    {Trapcode::IndirectCall_signature_mismatch, "indirect call type mismatch"},
    {Trapcode::Stack_overflow, "call stack exhausted"},
};
#endif