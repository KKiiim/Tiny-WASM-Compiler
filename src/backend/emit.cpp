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
void Emit::append(OPCode const opcode) {
  if (size_ + 4 >= DefaultPageSize) {
    // TODO(): re-large
    throw std::runtime_error("too large code size");
  }

  switch (opcode) {
  case OPCode::RETURN: {
    // d6 5f 03 c0
    uint32_t const insRET = 0xd65f03c0; // little endian for aarch64
    memcpy(&data_[size_], &insRET, sizeof(insRET));
    size_ += sizeof(insRET);
    break;
  }
  case OPCode::LOCAL_GET: {
    break;
  }
  case OPCode::UNREACHABLE:
  case OPCode::NOP:
  case OPCode::BLOCK:
  case OPCode::LOOP:
  case OPCode::IF:
  case OPCode::ELSE:
  case OPCode::END:
  case OPCode::BR:
  case OPCode::BR_IF:
  case OPCode::BR_TABLE:
  case OPCode::CALL:
  case OPCode::CALL_INDIRECT:
  case OPCode::REF_NULL:
  case OPCode::REF_IS_NULL:
  case OPCode::REF_FUNC:
  case OPCode::DROP:
  case OPCode::SELECT:
  case OPCode::SELECT_T:
  case OPCode::LOCAL_SET:
  case OPCode::LOCAL_TEE:
  case OPCode::GLOBAL_GET:
  case OPCode::GLOBAL_SET:
  case OPCode::TABLE_GET:
  case OPCode::TABLE_SET:
  case OPCode::I32_LOAD:
  case OPCode::I64_LOAD:
  case OPCode::F32_LOAD:
  case OPCode::F64_LOAD:
  case OPCode::I32_LOAD8_S:
  case OPCode::I32_LOAD8_U:
  case OPCode::I32_LOAD16_S:
  case OPCode::I32_LOAD16_U:
  case OPCode::I64_LOAD8_S:
  case OPCode::I64_LOAD8_U:
  case OPCode::I64_LOAD16_S:
  case OPCode::I64_LOAD16_U:
  case OPCode::I64_LOAD32_S:
  case OPCode::I64_LOAD32_U:
  case OPCode::I32_STORE:
  case OPCode::I64_STORE:
  case OPCode::F32_STORE:
  case OPCode::F64_STORE:
  case OPCode::I32_STORE8:
  case OPCode::I32_STORE16:
  case OPCode::I64_STORE8:
  case OPCode::I64_STORE16:
  case OPCode::I64_STORE32:
  case OPCode::MEMORY_SIZE:
  case OPCode::MEMORY_GROW:
  case OPCode::I32_CONST:
  case OPCode::I64_CONST:
  case OPCode::F32_CONST:
  case OPCode::F64_CONST:
  case OPCode::I32_EQZ:
  case OPCode::I32_EQ:
  case OPCode::I32_NE:
  case OPCode::I32_LT_S:
  case OPCode::I32_LT_U:
  case OPCode::I32_GT_S:
  case OPCode::I32_GT_U:
  case OPCode::I32_LE_S:
  case OPCode::I32_LE_U:
  case OPCode::I32_GE_S:
  case OPCode::I32_GE_U:
  case OPCode::I64_EQZ:
  case OPCode::I64_EQ:
  case OPCode::I64_NE:
  case OPCode::I64_LT_S:
  case OPCode::I64_LT_U:
  case OPCode::I64_GT_S:
  case OPCode::I64_GT_U:
  case OPCode::I64_LE_S:
  case OPCode::I64_LE_U:
  case OPCode::I64_GE_S:
  case OPCode::I64_GE_U:
  case OPCode::I32_CLZ:
  case OPCode::I32_CTZ:
  case OPCode::I32_POPCNT:
  case OPCode::I32_ADD:
  case OPCode::I32_SUB:
  case OPCode::I32_MUL:
  case OPCode::I32_DIV_S:
  case OPCode::I32_DIV_U:
  case OPCode::I32_REM_S:
  case OPCode::I32_REM_U:
  case OPCode::I32_AND:
  case OPCode::I32_OR:
  case OPCode::I32_XOR:
  case OPCode::I32_SHL:
  case OPCode::I32_SHR_S:
  case OPCode::I32_SHR_U:
  case OPCode::I32_ROTL:
  case OPCode::I32_ROTR:
  case OPCode::I64_CLZ:
  case OPCode::I64_CTZ:
  case OPCode::I64_POPCNT:
  case OPCode::I64_ADD:
  case OPCode::I64_SUB:
  case OPCode::I64_MUL:
  case OPCode::I64_DIV_S:
  case OPCode::I64_DIV_U:
  case OPCode::I64_REM_S:
  case OPCode::I64_REM_U:
  case OPCode::I64_AND:
  case OPCode::I64_OR:
  case OPCode::I64_XOR:
  case OPCode::I64_SHL:
  case OPCode::I64_SHR_S:
  case OPCode::I64_SHR_U:
  case OPCode::I64_ROTL:
  case OPCode::I64_ROTR:
    throw std::runtime_error("unsupported OPCode " + std::to_string(static_cast<uint32_t>(opcode)));
    break;
  }
}

ExecutableMemory Emit::getExecutableMemory() {
  return ExecutableMemory{data_, size_};
}