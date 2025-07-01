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

  inline uint64_t getStartAddr() const {
    return static_cast<uint64_t>(bit_cast<uintptr_t>(m_));
  }

  ///< set to specific offset
  ///< @note offset is in number of Data, not bytes
  template <typename Data> void set(uint32_t const offset, Data const data) {
    uint32_t const writePos = offset * sizeof(Data);
    if (writePos >= DefaultPageSize) {
      // TODO(): enlarge
      throw std::runtime_error("too large data size");
    }
    memcpy(&m_[writePos], &data, sizeof(data));
    // not related to pos, just write to specific offset
  }

  template <typename Data> Data get(uint32_t const offset) const {
    Data data{};
    uint32_t const readPos = offset * sizeof(Data);
    if (readPos >= DefaultPageSize) {
      throw std::runtime_error("read oom");
    }
    memcpy(&data, &m_[readPos], sizeof(data));

    return data;
  }

private:
  uint8_t *m_ = nullptr;
};

#endif