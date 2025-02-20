#ifndef SRC_BACKEND_LOCALMANAGER_H
#define SRC_BACKEND_LOCALMANAGER_H

#include <cstdint>

#include "../common/ExecutableMemory.hpp"

class LM {
public:
  LM();
  ~LM();
  /// @brief always alloc 64bits for local
  /// @return offset in local manager
  uint32_t add();

  ExecutableMemory getExecutableMemory();

private:
  uint32_t const DefaultPageSize = static_cast<uint32_t>(1000U) * 2U;

  uint8_t *data_;
  uint32_t size_;
};

#endif