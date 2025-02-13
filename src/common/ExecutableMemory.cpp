#include "ExecutableMemory.hpp"

ExecutableMemory::ExecutableMemory(uint8_t *data, uint32_t size) : data_(data), size_(size) {
  if (size_ == 0U) {
    throw std::runtime_error("empty ExecutableMemory");
  }
  void *const execMemory = mmap(nullptr, size_, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (execMemory == MAP_FAILED) {
    throw std::runtime_error("mmap failed");
  }
  std::memcpy(execMemory, data_, size_);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  __builtin___clear_cache(reinterpret_cast<char *>(execMemory), reinterpret_cast<char *>(execMemory) + size_);
}

ExecutableMemory::~ExecutableMemory() {
  munmap(data_, size_);
}