#include <type_traits>

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