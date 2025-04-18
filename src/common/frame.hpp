#ifndef SRC_COMMON_FRAME_H
#define SRC_COMMON_FRAME_H

#include <cstdint>

// Frame Layout


class Frame final {
public:
  Frame() = default;

private:
  // Max number of local variables in a function
  static constexpr const uint32_t MAX_LOCAL_VARS = 8U;

private:
  void *mem_;
  uint32_t offset_;
};

#endif