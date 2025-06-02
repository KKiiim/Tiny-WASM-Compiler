#ifndef SRC_COMMON_EXECUTABLEMEMORY_H
#define SRC_COMMON_EXECUTABLEMEMORY_H

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <sys/mman.h>

class ExecutableMemory {
public:
  ExecutableMemory() : mem_(nullptr), alignedSize_(0U), rawSize_(0U){};
  ExecutableMemory(uint8_t *data, uint32_t size);

  ExecutableMemory(ExecutableMemory &&other) noexcept;
  ExecutableMemory &operator=(ExecutableMemory &&other) noexcept;

  ExecutableMemory(const ExecutableMemory &) = delete;
  ExecutableMemory &operator=(const ExecutableMemory &) = delete;

  template <typename T> T data() const {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<T>(mem_);
  }
  ~ExecutableMemory();
  void disassemble() const;

private:
  void *mem_;
  uint32_t alignedSize_;
  uint32_t rawSize_;
};

#endif