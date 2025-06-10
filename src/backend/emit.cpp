#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <sys/mman.h>

#include "aarch64_encoding.hpp"
#include "emit.hpp"

#include "src/common/constant.hpp"
#include "src/common/logger.hpp"

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
  // Use it very carefully, ExecutableMemory is return as a temporary object.
  // Use like: emit.getExecutableMemory().data<void (*)()>() and call it may failed because
  // munmap will be called in its destructor
  return ExecutableMemory{data_, size_};
}

void Emit::emit_mov_x_imm64(REG const destReg, uint64_t const imm) {
  append(mov_r_imm16(destReg, imm & 0xFFFFU, true));
  append(movk_r_imm16(destReg, (imm >> 16U) & 0xFFFFU, 1, true));
  append(movk_r_imm16(destReg, (imm >> 32U) & 0xFFFFU, 2, true));
  append(movk_r_imm16(destReg, (imm >> 48U) & 0xFFFFU, 3, true));
}
void Emit::emit_mov_w_imm32(REG const destReg, uint32_t const imm) {
  append(mov_r_imm16(destReg, imm & 0xFFFFU, false));
  append(movk_r_imm16(destReg, (imm >> 16U) & 0xFFFFU, 1, false));
}

void Emit::decreaseSPWithClean(uint32_t const bytes) {
  assert(bytes % 16 == 0 && "[decreaseSPWithClean]size must be aligned to 16 bytes");
  // TODO(): support larger size
  assert((bytes < static_cast<uint32_t>(INT32_MAX)) && "size must be less than int32_t max value since str_base_off offset max support 32bit");
  append(dec_sp(bytes));

  ///< Clean the stack memory. TODO(): how to simplify this?
  // R9 used as scratch register
  append(mov_r_imm16(REG::R9, 0U, true));
  uint32_t const bytesPerStr = 8U; // 64bits register, 8 bytes per str_base_off
  for (uint32_t i = 0; i < bytes / 8; ++i) {
    int32_t const offset = static_cast<int32_t>(i * 8U);
    append(str_base_off(REG::SP, REG::R9, offset, true));
  }
}

void Emit::set_b_cond_off(uint32_t const b_instructionPositionOffsetToOutputBinary, int32_t const condOffset) {
  assert(size_ >= b_instructionPositionOffsetToOutputBinary + 4U && "must have the b instruction");

  OPCodeTemplate opcode;
  memcpy(&opcode, &data_[b_instructionPositionOffsetToOutputBinary], sizeof(OPCodeTemplate));
  // TODO(): Currently, only support b.cond
  assert((opcode & static_cast<OPCodeTemplate>(0x54000000)) == static_cast<OPCodeTemplate>(0x54000000));
  LOG_YELLOW << "opcode before" << std::hex << static_cast<uint32_t>(opcode) << std::endl;

  // 0101 0100 imm19 0 cond(4)
  // 54000000
  assert(((condOffset >= static_cast<int32_t>(-524288)) && (condOffset <= static_cast<int32_t>(0x7ffff))) &&
         "Offset out of range signed 19 for branch instruction");
  opcode |= (static_cast<OPCodeTemplate>(condOffset) & 0x7FFFFU) << 5U; // offset 5-24

  memcpy(&data_[b_instructionPositionOffsetToOutputBinary], &opcode, sizeof(OPCodeTemplate));
}