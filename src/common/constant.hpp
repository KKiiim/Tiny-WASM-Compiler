#ifndef SRC_COMMON_CONSTANT_H
#define SRC_COMMON_CONSTANT_H

#include <cstdint>

constexpr const uint32_t DefaultPageSize = static_cast<uint32_t>(1000U) * 2U;
constexpr const uint32_t MaxParamsForWasmFunction = 8U;

#endif