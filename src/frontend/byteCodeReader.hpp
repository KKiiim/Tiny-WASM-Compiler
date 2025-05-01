#ifndef SRC_FRONTEND_BYTECODEREADER_H
#define SRC_FRONTEND_BYTECODEREADER_H

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include "../common/util.hpp"

class BytecodeReader {
public:
  BytecodeReader() = default;
  void readWasmBinary(std::string const &wasmPath);
  inline uint64_t getOffset() const {
    return offset_;
  }
  ///
  /// @brief Get the number of bytes still left in the binary
  ///
  /// @return size_t Number of bytes left in the binary
  inline size_t getBytesLeft() const {
    return bytecode_.size() - getOffset();
  }

  ///
  /// @brief Whether there is at least one byte left in the binary, i.e. getBytesLeft is greater than 1
  ///
  /// @return bool Whether there is at least one byte left in the binary
  inline bool hasNextByte() const {
    return getBytesLeft() > 0U;
  }

  inline void jump(uint32_t const offset) {
    offset_ += offset;
  }

  /// @brief Read a byte as given datatype from the binary
  template <class Dest> Dest readByte() {
    if (offset_ == bytecode_.size()) {
      throw std::runtime_error("offset " + std::to_string(offset_) + " readByte failed");
    }

    static_assert(std::is_trivially_copyable<Dest>::value, "readByte requires the destination type to be copyable");
    static_assert(sizeof(Dest) == 1, "Size of type of readByte needs to be 1");
    Dest dest;
    uint8_t const *const oldPtr = &bytecode_[offset_];
    offset_++;
    static_cast<void>(std::memcpy(&dest, oldPtr, static_cast<size_t>(sizeof(Dest))));
    return dest;
  }

  ///
  /// @brief Read fixed 4 bytes from the binary (host-endian-independently from little endian) into a uint32_t
  ///
  /// @return uint32_t Data that has been read
  /// @throws ValidationException if the new pointer is out of bounds
  uint32_t readLEU32();

  ///
  /// @brief Read fixed 8 bytes from the binary (host-endian-independently from little endian) into a uint64_t
  ///
  /// @return uint64_t Data that has been read
  /// @throws ValidationException if the new pointer is out of bounds
  uint64_t readLEU64();

  ///
  /// @brief Read the next LEB128 encoded variable length integer from the current cursor
  ///
  /// @tparam Type Integer type to read (can be signed, unsigned and of an arbitrary standardized length)
  /// @return Type Data that has been read
  /// @throws ValidationException if the new pointer is out of bounds or the LEB128-encoded integer is malformed
  template <class Type> Type readLEB128() {
    static_assert(std::is_integral<Type>::value, "readLEB128 can only read variable length integers");
    using IntermediateType = typename std::conditional<std::is_signed<Type>::value, int64_t, uint64_t>::type;
    return static_cast<Type>(bit_cast<IntermediateType>(readLEB128(std::is_signed<Type>::value, sizeof(Type) * 8U)));
  }

private:
  std::vector<uint8_t> bytecode_; ///> wasm binary code read from .wasm file
  uint64_t offset_{};             ///> read offset of bytecode_

  ///
  /// @brief Read a LEB128 integer from the current cursor of the BytecodeReader
  ///
  /// @param signedInt Whether the integer that should be read is signed
  /// @param maxBits How many bits the integer that should be read has
  /// @return uint64_t Sign-extended data that has been read, maximally the maxBits least significant bits are populated
  /// with actual data, rest is sign-extended
  /// @throws ValidationException if the new pointer is out of bounds or the LEB128-encoded integer is malformed
  uint64_t readLEB128(bool const signedInt, uint32_t const maxBits);
};

#endif