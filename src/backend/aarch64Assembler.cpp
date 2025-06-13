#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <sys/mman.h>

#include "aarch64Assembler.hpp"

#include "src/common/constant.hpp"
#include "src/common/logger.hpp"

Assembler::Assembler() : data_(nullptr), size_(0U) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  data_ = reinterpret_cast<uint8_t *>(realloc(data_, DefaultPageSize));
}
Assembler::~Assembler() {
  free(data_);
}
void Assembler::append(OPCodeTemplate const ins) {
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

ExecutableMemory Assembler::getExecutableMemory() {
  // Use it very carefully, ExecutableMemory is return as a temporary object.
  // Use like: emit.getExecutableMemory().data<void (*)()>() and call it may failed because
  // munmap will be called in its destructor
  return ExecutableMemory{data_, size_};
}