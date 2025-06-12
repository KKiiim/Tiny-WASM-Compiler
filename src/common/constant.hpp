#ifndef SRC_COMMON_CONSTANT_H
#define SRC_COMMON_CONSTANT_H

#include <cstdint>
#include <string>
#include <unordered_map>

constexpr const uint32_t DefaultPageSize = static_cast<uint32_t>(1000U) * 2U;
constexpr const uint32_t MaxParamsForWasmFunction = 8U;

enum Trapcode : uint32_t {
  NONE = 0,
  DIV_0 = 1,
};
const std::unordered_map<Trapcode, std::string> trapcodeString{
    {Trapcode::NONE, "NONE"},
    {Trapcode::DIV_0, "DIV_0"},
};
#endif