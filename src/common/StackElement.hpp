#ifndef SRC_COMMON_STACKELEMENT_H
#define SRC_COMMON_STACKELEMENT_H

#include <cstdint>
#include <type_traits>

#include "util.hpp"
#include "wasm_type.hpp"

class StackType final {
public:
  constexpr inline StackType() : StackType(0U) {
  }

  ///
  /// @param raw The raw uint32_t value of this stack type.
  constexpr inline explicit StackType(uint32_t const raw) : raw_(raw) {
  }

  template <typename RHS> inline constexpr StackType &operator=(RHS const rhs) {
    static_assert(std::is_same<RHS, StackType>::value || std::is_same<RHS, uint32_t>::value, "RHS must be either StackType or uint32_t");
    raw_ = static_cast<uint32_t>(rhs);
    return *this;
  }

  static constexpr uint32_t INVALID{0U};     ///< Invalid StackElement, not representing any actual operand
  static constexpr uint32_t SANULL{INVALID}; ///< StackElement with undefined Type

  static constexpr uint32_t SCRATCHREGISTER{1U}; ///< StackElement representing a variable in a scratch register
  static constexpr uint32_t TEMPSTACK{2U};       ///< StackElement representing a variable on the stack (not a param or global)
  static constexpr uint32_t CONSTANT{3U};        ///< StackElement representing a constant

  static constexpr uint32_t LOCAL{4U};  ///< StackElement representing a local variable (Can be on stack or in a register, actual location defined in
                                        ///< the corresponding LocalDef)
  static constexpr uint32_t GLOBAL{5U}; ///< StackElement representing a global variable (Actual location defined in the corresponding GlobalDef)

  static constexpr uint32_t DEFERREDACTION{6U}; ///< StackElement representing a deferred action, i.e. an arithmetic instruction, conversion etc. that
                                                ///< has not been emitted yet

  static constexpr uint32_t BLOCK{7U};   ///< StackElement representing the opening of a structural block
  static constexpr uint32_t LOOP{8U};    ///< StackElement representing the opening of a structural loop
  static constexpr uint32_t IFBLOCK{9U}; ///< StackElement representing a synthetic block that is inserted to properly realize branches for IF
                                         ///< statements

  static constexpr uint32_t SKIP{10U}; ///< StackElements that will be skipped when traversing; inserted when iteratively condensing valent blocks

  // Flags for scratchRegister, tempStack and constant

  static constexpr uint32_t TVOID{0b0000'0000U};                        ///< void
  static constexpr uint32_t I32{0b0001'0000U};                          ///< int32
  static constexpr uint32_t SCRATCHREGISTER_I32{SCRATCHREGISTER | I32}; ///< int32 in scratch register
  static constexpr uint32_t TEMPSTACK_I32{TEMPSTACK | I32};             ///< int32 in tmp stack
  static constexpr uint32_t CONSTANT_I32{CONSTANT | I32};               ///< int32 const

  static constexpr uint32_t I64{0b0010'0000U};                          ///< int64
  static constexpr uint32_t SCRATCHREGISTER_I64{SCRATCHREGISTER | I64}; ///< int64 in scratch register
  static constexpr uint32_t TEMPSTACK_I64{TEMPSTACK | I64};             ///< int64 in tmp stack
  static constexpr uint32_t CONSTANT_I64{CONSTANT | I64};               ///< int64 const

  static constexpr uint32_t F32{0b0100'0000U};                          ///< float32
  static constexpr uint32_t SCRATCHREGISTER_F32{SCRATCHREGISTER | F32}; ///< float32 in scratch register
  static constexpr uint32_t TEMPSTACK_F32{TEMPSTACK | F32};             ///< float32 in tmp stack
  static constexpr uint32_t CONSTANT_F32{CONSTANT | F32};               ///< float32 const

  static constexpr uint32_t F64{0b1000'0000U};                          ///< float64
  static constexpr uint32_t SCRATCHREGISTER_F64{SCRATCHREGISTER | F64}; ///< float64 in scratch register
  static constexpr uint32_t TEMPSTACK_F64{TEMPSTACK | F64};             ///< float64 in tmp stack
  static constexpr uint32_t CONSTANT_F64{CONSTANT | F64};               ///< float64 const
  static constexpr uint32_t UNKNOWN{0b1111'0000U};                      ///< unknown
  static constexpr uint32_t BASEMASK{0b0000'1111U};                     ///< mask of base type
  static constexpr uint32_t TYPEMASK{0b1111'0000U};                     ///< mask of type(i32/i64/f32/f64)

  template <typename RHS> inline constexpr StackType operator&(RHS const rhs) const {
    return StackType{this->raw_ & static_cast<uint32_t>(rhs)};
  }

  explicit inline constexpr operator uint32_t() const {
    return raw_;
  }

private:
  uint32_t raw_;
};

class StackElement final {
public:
  StackType type;

  union Data {
    OPCode opcode;
    ConstUnion constUnion{};
  };

  Data data = {};

  static inline constexpr StackElement i32Const(uint32_t const value) {
    StackElement res{};
    res.type = StackType::CONSTANT_I32;
    res.data.constUnion.u32 = value;
    return res;
  }

  static inline constexpr StackElement i64Const(uint64_t const value) {
    StackElement res{};
    res.type = StackType::CONSTANT_I64;
    res.data.constUnion.u64 = value;
    return res;
  }

  static inline constexpr StackElement f32Const(float const value) {
    StackElement res{};
    res.type = StackType::CONSTANT_F32;
    res.data.constUnion.f32 = value;
    return res;
  }

  static inline constexpr StackElement f64Const(double const value) {
    StackElement res{};
    res.type = StackType::CONSTANT_F64;
    res.data.constUnion.f64 = value;
    return res;
  }
};

#endif
