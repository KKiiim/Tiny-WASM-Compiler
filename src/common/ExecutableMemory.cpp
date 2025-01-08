#include "ExecutableMemory.hpp"

ExecutableMemory::ExecutableMemory(uint8_t *data, uint32_t size) : data_(data), size_(size) {
  void *execMemory = mmap(nullptr, size_, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (execMemory == MAP_FAILED) {
    throw std::runtime_error("mmap failed");
  }
  std::memcpy(execMemory, data_, size_);
}

ExecutableMemory::~ExecutableMemory() {
  munmap(data_, size_);
}