#ifndef SRC_COMMON_EXECUTABLEMEMORY_H
#define SRC_COMMON_EXECUTABLEMEMORY_H

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <sys/mman.h>

class ExecutableMemory {
public:
  ExecutableMemory(uint8_t *data, uint32_t size);
  template <typename T> T data() const {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<T>(mem_);
  }
  ~ExecutableMemory();

private:
  void *mem_;
  uint32_t alignedSize_;
};

#endif