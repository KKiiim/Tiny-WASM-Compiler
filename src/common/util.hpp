#ifndef SRC_COMMON_UTIL_H
#define SRC_COMMON_UTIL_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <utility>

///
/// @brief add a pointer with offset
///
/// @tparam T Pointer type
/// @tparam TNUM Offset type
/// @param ptr
/// @param offset
/// @return calculate result
///
template <typename T, typename TNUM> inline T pAddI(T const ptr, TNUM const offset) {
  static_assert(std::is_pointer<T>::value, "T must be a pointer");
  static_assert(std::is_integral<TNUM>::value, "TNUM must be an integer");
  T const res{ptr + offset};
  return res;
}

///
/// @brief Literal suffix for explicit initialization of a uint8_t
///
// NOLINTNEXTLINE(google-runtime-int)
inline constexpr uint8_t operator"" _U8(const unsigned long long arg) {
  return static_cast<uint8_t>(arg);
}

///
/// @brief Literal suffix for explicit initialization of a uint32_t
///
// NOLINTNEXTLINE(google-runtime-int)
inline constexpr uint32_t operator"" _U32(const unsigned long long arg) {
  return static_cast<uint32_t>(arg);
}

///
/// @brief Literal suffix for explicit initialization of a uint64_t
///
// NOLINTNEXTLINE(google-runtime-int)
inline constexpr uint64_t operator"" _U64(const unsigned long long arg) {
  return static_cast<uint64_t>(arg);
}

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
///
/// @brief Reinterprets a data type so that the underlying bit representation is unchanged. Unconditionally fulfills the
/// strict aliasing rule.
///
/// @tparam Dest Destination type
/// @tparam Source Source type
/// @param source Data to reinterpret
/// @return constexpr Dest Reinterpreted data
template <class Dest, class Source> constexpr Dest bit_cast(Source const &source) {
  static_assert(std::is_trivially_copyable<Source>::value, "bit_cast requires the source type to be copyable");
  static_assert(std::is_trivially_copyable<Dest>::value, "bit_cast requires the destination type to be copyable");
  static_assert(sizeof(Dest) == sizeof(Source), "bit_cast requires source and destination to be the same size");
  Dest dest;
  static_cast<void>(std::memcpy(&dest, &source, static_cast<size_t>(sizeof(dest))));
  return dest;
}

union ConstUnion {
  uint32_t u32; ///< 32-bit integer
  uint64_t u64; ///< 64-bit integer
  float f32;    ///< 32-bit float
  double f64;   ///< 64-bit float

  ///
  /// @brief Get the raw, reinterpreted value of the float as an integer
  ///
  /// @return uint32_t Raw, reinterpreted value of the float
  inline uint32_t rawF32() const {
    return bit_cast<uint32_t>(f32);
  }

  ///
  /// @brief Get the raw, reinterpreted value of the float as an integer
  ///
  /// @return uint64_t Raw, reinterpreted value of the float
  inline uint64_t rawF64() const {
    return bit_cast<uint64_t>(f64);
  }
};

///
/// @brief Creates an std::array from a list of elements as function arguments
///
/// @tparam V The explicit type of the array
/// @tparam T Type of the elements
/// @param t Parameter pack of the elements
/// @return std::array<V, sizeof...(T)> Array of the given type with the elements in place
template <typename... T> constexpr auto make_array(T &&...t) noexcept -> std::array<typename std::common_type<T...>::type const, sizeof...(T)> {
  return {{std::forward<typename std::common_type<T...>::type const>(t)...}};
}

#endif