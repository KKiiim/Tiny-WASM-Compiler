#ifndef SRC_FRONTEND_RUNTIMEBLOCK_HPP
#define SRC_FRONTEND_RUNTIMEBLOCK_HPP

#include <cstdint>
#include <cstdlib>

#include "src/common/constant.hpp"
#include "src/common/logger.hpp"
#include "src/common/util.hpp"

class RuntimeBlock {
public:
  RuntimeBlock() : m_(static_cast<uint8_t *>(malloc(DefaultPageSize))) {
    confirm(m_ != nullptr, "malloc RuntimeBlock failed");
  }
  ~RuntimeBlock() {
    free(m_);
  }
  RuntimeBlock(const RuntimeBlock &) = delete;
  RuntimeBlock &operator=(const RuntimeBlock &) = delete;
  RuntimeBlock(RuntimeBlock &&) = delete;
  RuntimeBlock &operator=(RuntimeBlock &&) = delete;

  inline uint64_t getStartAddr() {
    return static_cast<uint64_t>(bit_cast<uintptr_t>(m_));
  }

  template <typename Data> void set(Data const data) {
    if (size_ + sizeof(Data) >= DefaultPageSize) {
      // TODO(): enlarge
      throw std::runtime_error("too large data size");
    }
    memcpy(&m_[size_], &data, sizeof(data));
    size_ += sizeof(data);
  }

private:
  uint8_t *m_ = nullptr;
  uint32_t size_{0U};
};

#endif