#ifndef SRC_BACKEND_LOCALMANAGER_H
#define SRC_BACKEND_LOCALMANAGER_H

#include <cstdint>

#include "../common/wasm_type.hpp"
#include "../backend/arm64Backend.hpp"

class LM {
public:
  explicit LM(Arm64Backend& backend) : size_(0U),backend_(backend) {}
  /// @brief store i32 and i64
  /// @return record offset in from SP
  uint32_t add(WasmType const localType);
  inline uint32_t getAlignedSize() const {
    constexpr uint32_t alignment = 16U;
    return (size_ + alignment - 1) & ~(alignment - 1);
  }
  void pushStack(REG const reg);
  void popStack(REG const reg);
private:
  uint32_t size_;
  Arm64Backend& backend_;

  // Max number of local variables in a function
  static constexpr const uint32_t MAX_LOCAL_VARS = 8U;
};

#endif