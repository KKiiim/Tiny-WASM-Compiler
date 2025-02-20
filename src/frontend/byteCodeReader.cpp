#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "byteCodeReader.hpp"

void BytecodeReader::readWasmBinary(std::string const &wasmPath) {
  std::ifstream file(wasmPath, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    throw std::runtime_error(wasmPath + " open failed");
  }

  std::streamsize const size = file.tellg();
  file.seekg(0, std::ios::beg);
  bytecode_.resize(size);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  if (!file.read(reinterpret_cast<char *>(bytecode_.data()), size)) {
    throw std::runtime_error(wasmPath + " read failed");
  }
  file.close();
}

uint32_t BytecodeReader::readLEU32() {
  uint8_t const *const oldPtr{&bytecode_[offset_]};
  uint32_t const value{(static_cast<uint32_t>(*oldPtr)) | (static_cast<uint32_t>(*pAddI(oldPtr, 1)) << 8U) |
                       (static_cast<uint32_t>(*pAddI(oldPtr, 2)) << 16U) | (static_cast<uint32_t>(*pAddI(oldPtr, 3)) << 24U)};
  offset_ += 4;
  return value;
}

uint64_t BytecodeReader::readLEU64() {
  uint8_t const *const oldPtr{&bytecode_[offset_]};
  uint64_t const value{(static_cast<uint64_t>(*oldPtr)) | (static_cast<uint64_t>(*pAddI(oldPtr, 1)) << 8U) |
                       (static_cast<uint64_t>(*pAddI(oldPtr, 2)) << 16U) | (static_cast<uint64_t>(*pAddI(oldPtr, 3)) << 24U) |
                       (static_cast<uint64_t>(*pAddI(oldPtr, 4)) << 32U) | (static_cast<uint64_t>(*pAddI(oldPtr, 5)) << 40U) |
                       (static_cast<uint64_t>(*pAddI(oldPtr, 6)) << 48U) | (static_cast<uint64_t>(*pAddI(oldPtr, 7)) << 56U)};
  offset_ += 8;
  return value;
}

uint64_t BytecodeReader::readLEB128(bool const signedInt, uint32_t const maxBits) {
  assert(maxBits <= 64U && "maxBits longer than 64 bits");
  uint64_t result{0U};
  uint32_t bitsWritten{0U};
  uint8_t byte{0xFFU};
  while ((static_cast<uint32_t>(byte) & 0x80U) != 0U) {
    byte = readByte<uint8_t>();
    if (bitsWritten >= maxBits) {
      // One full byte too many -> malformed
      throw std::runtime_error("Malformed_LEB128_integer__Out_of_bounds_");
    }
    uint32_t const lowByte{static_cast<uint32_t>(byte) & 0x7F_U32};
    result |= static_cast<uint64_t>(lowByte) << static_cast<uint64_t>(bitsWritten);
    bitsWritten = bitsWritten + 7U;
    if (bitsWritten > maxBits) {
      // More bits written than allowed
      if (signedInt && ((static_cast<uint32_t>(byte) & (1_U32 << (6_U32 - (bitsWritten - maxBits)))) != 0_U32)) {
        // If it is signed and negative (sign bit set) "1" padding allowed
        uint32_t const bitMask{(0xFF_U32 << ((6_U32 - (bitsWritten - maxBits)) + 1_U32)) & 0b01111111_U32};
        if ((static_cast<uint32_t>(byte) & bitMask) != bitMask) {
          throw std::runtime_error("Malformed_LEB128_signed_integer__Wrong_padding_");
        }
      } else {
        // Zero padding allowed if unsigned or positive signed integer
        uint32_t const bitMask{(0xFF_U32 << ((6_U32 - (bitsWritten - maxBits)) + 1_U32)) & 0b01111111_U32};
        if ((static_cast<uint32_t>(byte) & bitMask) != 0U) {
          throw std::runtime_error("Malformed_LEB128_unsigned_integer__Wrong_padding_");
        }
      }
    }
  }

  // Can also be used if bitsWritten > maxBits because valid padding has already been established and is guaranteed to
  // correspond to the sign bit
  if ((signedInt && ((static_cast<uint32_t>(byte) & 0x40U) != 0U)) && (bitsWritten < 64_U32)) {
    // Sign extend
    uint64_t const signExtensionMask{0xFF'FF'FF'FF'FF'FF'FF'FFLLU << bitsWritten};
    result |= signExtensionMask;
  }
  return result;
}