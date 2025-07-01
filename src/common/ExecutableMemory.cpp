#include <capstone/capstone.h>
#include <cstdint>
#include <iomanip>
#include <sys/mman.h>

#include "ExecutableMemory.hpp"
#include "util.hpp"

#include "src/common/logger.hpp"

static constexpr size_t PAGE_SIZE = 4096;

ExecutableMemory::ExecutableMemory(uint32_t size) {
  if (size == 0U) {
    throw std::runtime_error("empty ExecutableMemory");
  }

  alignedSize_ = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

  mem_ = mmap(nullptr, alignedSize_, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mem_ == MAP_FAILED) {
    throw std::runtime_error("mmap failed");
  }
  if (bit_cast<uintptr_t>(mem_) % 4 != 0) {
    throw std::runtime_error("Memory is not 4-byte aligned");
  }
  LOG_DEBUG << "mmap ExecutableMemory from: " << std::hex << bit_cast<uintptr_t>(mem_) << " to "
            << bit_cast<uintptr_t>(&static_cast<uint8_t *>(mem_)[alignedSize_]) << std::dec << LOG_END;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  __builtin___clear_cache(reinterpret_cast<char *>(mem_), reinterpret_cast<char *>(mem_) + alignedSize_);
}

ExecutableMemory::~ExecutableMemory() {
  LOG_DEBUG << "unmap ExecutableMemory from: " << std::hex << bit_cast<uintptr_t>(mem_) << " to "
            << bit_cast<uintptr_t>(&static_cast<uint8_t *>(mem_)[alignedSize_]) << std::dec << LOG_END;
  if (mem_ != MAP_FAILED) {
    munmap(mem_, alignedSize_);
  }
}

void ExecutableMemory::disassemble() const {
  csh handle;
  cs_insn *insn;
  cs_err err;

  if (cs_open(CS_ARCH_ARM64, CS_MODE_ARM, &handle) != CS_ERR_OK) {
    LOG_ERROR << "Capstone init failed" << LOG_END;
    return;
  }
  cs_option(handle, CS_OPT_SKIPDATA, CS_OPT_OFF);
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_OFF);

  size_t const count = cs_disasm(handle, bit_cast<const uint8_t *>(mem_), pos_, 0, 0, &insn);

  if (count <= 0) {
    LOG_ERROR << "disassemble failed" << LOG_END;
    cs_close(&handle);
    return;
  }

  LOG_INFO << "Address     | Code      | Instruction\n";
  LOG_INFO << "----------------------------------------\n";
  for (size_t i = 0; i < count; i++) {
    LOG_INFO << "0x" << std::hex << std::setw(8) << std::setfill('0') << insn[i].address << ": ";
    for (int j = 0; j < 4; j++) {
      LOG_INFO << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(insn[i].bytes[j]) << " ";
    }
    LOG_INFO << " " << std::left << std::setw(8) << std::setfill(' ') << insn[i].mnemonic << insn[i].op_str << LOG_END;
  }

  cs_free(insn, count);
  cs_close(&handle);
}