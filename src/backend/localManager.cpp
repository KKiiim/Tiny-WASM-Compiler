#include <cstdlib>
#include <stdexcept>

#include "localManager.hpp"

LM::LM() : data_(nullptr), size_(0U) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  data_ = reinterpret_cast<uint8_t *>(realloc(data_, DefaultPageSize));
}
LM::~LM() {
  free(data_);
}
uint32_t LM::add() {
  if (size_ + 8 > DefaultPageSize) {
    // TODO(): re-large
    throw std::runtime_error("too more locals");
  }
  uint32_t const startOffset = size_;
  size_ += 8U;
  return startOffset;
}

ExecutableMemory LM::getExecutableMemory() {
  return ExecutableMemory{data_, size_};
}
