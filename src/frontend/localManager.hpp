#ifndef SRC_BACKEND_LOCALMANAGER_H
#define SRC_BACKEND_LOCALMANAGER_H

#include <cstdint>

#include "../common/wasm_type.hpp"

class LM {
public:
  /// @brief store i32 than store i64
  /// @return record offset in from SP
  uint32_t add(WasmType const localType);
  inline uint32_t getSize() const {
    return size_;
  }

private:
  uint32_t size_ = 0U;

  // Max number of local variables in a function
  static constexpr const uint32_t MAX_LOCAL_VARS = 8U;
};

#endif