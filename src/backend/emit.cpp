#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/mman.h>

#include "../common/constant.hpp"
#include "aarch64_encoding.hpp"
#include "emit.hpp"

Emit::Emit() : data_(nullptr), size_(0U) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  data_ = reinterpret_cast<uint8_t *>(realloc(data_, DefaultPageSize));
}
Emit::~Emit() {
  free(data_);
}
void Emit::append(OPCodeTemplate const ins) {
  if (size_ + sizeof(OPCodeTemplate) >= DefaultPageSize) {
    // TODO(): enlarge
    throw std::runtime_error("too large data_ size_");
  }

  // convert ins to little endian
  uint8_t const *const insPtr = bit_cast<uint8_t const *>(&ins);
  OPCodeTemplate littleEndianIns = 0;
  for (size_t i = 0; i < sizeof(OPCodeTemplate); i++) {
    littleEndianIns |= static_cast<OPCodeTemplate>(insPtr[i]) << (i * 8U);
  }
  memcpy(&data_[size_], &ins, sizeof(ins));
  size_ += sizeof(ins);
}

ExecutableMemory Emit::getExecutableMemory() {
  return ExecutableMemory{data_, size_};
}