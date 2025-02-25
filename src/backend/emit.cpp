#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/mman.h>

#include "emit.hpp"

Emit::Emit() : data_(nullptr), size_(0U) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  data_ = reinterpret_cast<uint8_t *>(realloc(data_, DefaultPageSize));
}
Emit::~Emit() {
  free(data_);
}
void Emit::append(uint32_t const ins) {
  if (size_ + 4 >= DefaultPageSize) {
    // TODO(): re-large
    throw std::runtime_error("too large code size");
  }

  memcpy(&data_[size_], &ins, sizeof(ins));
  size_ += sizeof(ins);
}

ExecutableMemory Emit::getExecutableMemory() {
  return ExecutableMemory{data_, size_};
}