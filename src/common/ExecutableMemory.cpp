#include "ExecutableMemory.hpp"

static constexpr size_t PAGE_SIZE = 4096;

ExecutableMemory::ExecutableMemory(uint8_t *data, uint32_t size) {
  if (size == 0U) {
    throw std::runtime_error("empty ExecutableMemory");
  }

  alignedSize_ = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

  mem_ = mmap(nullptr, alignedSize_, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mem_ == MAP_FAILED) {
    throw std::runtime_error("mmap failed");
  }

  std::memcpy(mem_, data, size);
  uint8_t *const fillNOPStart = static_cast<uint8_t *>(mem_) + size;
  std::memset(fillNOPStart, 0x90, alignedSize_ - size); // 0x90: NOP
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  __builtin___clear_cache(reinterpret_cast<char *>(mem_), reinterpret_cast<char *>(mem_) + alignedSize_);
}

ExecutableMemory::~ExecutableMemory() {
  if (mem_ != MAP_FAILED) {
    munmap(mem_, alignedSize_);
  }
}