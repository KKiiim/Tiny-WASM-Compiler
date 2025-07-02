#ifndef SRC_FRONTEND_RUNTIMEBLOCK_HPP
#define SRC_FRONTEND_RUNTIMEBLOCK_HPP

#include <cstdint>
#include <cstdlib>

#include "src/common/constant.hpp"
#include "src/common/logger.hpp"
#include "src/common/util.hpp"

template <typename Data> class RuntimeBlock {
  static constexpr const uint32_t defaultBlockSize = DefaultPageSize;

public:
  RuntimeBlock() : m_(static_cast<uint8_t *>(malloc(defaultBlockSize))) {
    confirm(m_ != nullptr, "malloc RuntimeBlock failed");
  }
  ~RuntimeBlock() {
    free(m_);
  }
  RuntimeBlock(const RuntimeBlock &) = delete;
  RuntimeBlock &operator=(const RuntimeBlock &) = delete;
  RuntimeBlock(RuntimeBlock &&) = delete;
  RuntimeBlock &operator=(RuntimeBlock &&) = delete;

  inline uintptr_t getStartAddr() const {
    return bit_cast<uintptr_t>(m_);
  }

  /// set to specific offset
  /// @note offset is in number of Data, not bytes
  void set(uint32_t const offset, Data const data) {
    uint32_t const writePos = offset * sizeof(Data);
    if (writePos >= defaultBlockSize) {
      // TODO(): enlarge
      throw std::runtime_error("too large data size");
    }
    memcpy(&m_[writePos], &data, sizeof(data));
  }

  /// @note offset is in number of Data, not bytes
  Data get(uint32_t const offset) const {
    uint32_t const readPos = offset * sizeof(Data);
    if (readPos >= defaultBlockSize) {
      throw std::runtime_error("read oom");
    }
    Data result;
    memcpy(&result, &m_[readPos], sizeof(Data));
    return result;
  }

private:
  uint8_t *m_ = nullptr;
};

#endif