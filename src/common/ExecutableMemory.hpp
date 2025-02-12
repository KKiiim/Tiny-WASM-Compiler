#ifndef SRC_COMMON_EXECUTABLEMEMORY_H
#define SRC_COMMON_EXECUTABLEMEMORY_H

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <sys/mman.h>

class ExecutableMemory {
public:
  ExecutableMemory(uint8_t *data, uint32_t size);
  inline uint8_t *data() const {
    return data_;
  }
  ~ExecutableMemory();

private:
  uint8_t *data_;
  uint32_t size_;
};

#endif