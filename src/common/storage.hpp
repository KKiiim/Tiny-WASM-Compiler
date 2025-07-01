#ifndef SRC_COMMON_STORAGE_H
#define SRC_COMMON_STORAGE_H

#include <cstdint>

#include "src/backend/aarch64_encoding.hpp"

///
/// @brief Type of the location of this variable
///
enum class StorageType : uint8_t { REGISTER, CONSTANT };

class Storage {
public:
  explicit Storage(REG const reg) : type_(StorageType::REGISTER) {
    location_.reg = reg;
  }
  explicit Storage(ConstUnion const v) : type_(StorageType::CONSTANT) {
    location_.constUnion = v;
  }

  /// @brief Description of the location of this variable (active member is chosen by type)
  union Location {
    REG reg;               ///< CPU register defined by the backend
    ConstUnion constUnion; ///< Store a const immediate value
  };

  StorageType type_;
  Location location_{};
};

#endif