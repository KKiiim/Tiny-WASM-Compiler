#ifndef SRC_COMMON_EXECUTABLEMEMORY_H
#define SRC_COMMON_EXECUTABLEMEMORY_H

#include <cstdint>
#include <cstring>
#include <stdexcept>

#include "src/common/util.hpp"

class ExecutableMemory {
public:
  explicit ExecutableMemory(uint32_t size);

  ExecutableMemory(ExecutableMemory &&other) = delete;
  ExecutableMemory &operator=(ExecutableMemory &&other) = delete;
  ExecutableMemory(const ExecutableMemory &) = delete;
  ExecutableMemory &operator=(const ExecutableMemory &) = delete;

  template <typename T> T data() const {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<T>(mem_);
  }
  ~ExecutableMemory();
  void disassemble() const;

  ///< set to specific offset
  template <typename Data> void set(uint32_t const offset, Data const data) {
    if (offset + sizeof(data) >= alignedSize_) {
      throw std::runtime_error("ExecutableMemory set data oom");
    }
    memcpy(&static_cast<uint8_t *>(mem_)[offset], &data, sizeof(data));
    // not related to pos, just write to specific offset
  }
  template <typename Data> Data get(uint32_t const offset) const {
    Data data{};
    if (offset + sizeof(data) >= alignedSize_) {
      throw std::runtime_error("ExecutableMemory read oom");
    }
    memcpy(&data, &static_cast<uint8_t *>(mem_)[offset], sizeof(data));
    return data;
  }
  template <typename Data> void append(Data const data) {
    set(pos_, data);
    pos_ += sizeof(data);
  }
  inline uint32_t getSize() const {
    return pos_;
  }
  inline uintptr_t getCurrentAbsAddress() const {
    return bit_cast<uintptr_t>(&static_cast<uint8_t *>(mem_)[pos_]);
  }
  uint8_t *getRawPtr() {
    return static_cast<uint8_t *>(mem_);
  }

private:
  void *mem_;
  uint32_t alignedSize_;
  uint32_t pos_{0U};
};

#endif