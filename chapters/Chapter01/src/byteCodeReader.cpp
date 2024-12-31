#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "byteCodeReader.hpp"
#include "util.hpp"

void ByteCodeReader::readWasmBinary(std::string const &wasmPath) {
  std::ifstream file(wasmPath, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    throw std::runtime_error(wasmPath + " open failed");
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);
  bytecode_.resize(size);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  if (!file.read(reinterpret_cast<char *>(bytecode_.data()), size)) {
    throw std::runtime_error(wasmPath + " read failed");
  }
  file.close();
}

uint32_t ByteCodeReader::readLEU32() {
  uint8_t const *const oldPtr{&bytecode_[offset_]};
  uint32_t const value{(static_cast<uint32_t>(*oldPtr)) | (static_cast<uint32_t>(*pAddI(oldPtr, 1)) << 8U) |
                       (static_cast<uint32_t>(*pAddI(oldPtr, 2)) << 16U) | (static_cast<uint32_t>(*pAddI(oldPtr, 3)) << 24U)};
  offset_ += 4;
  return value;
}