
#include <cstdint>
#include <string>
#include <vector>

class ByteCodeReader {
public:
  ByteCodeReader() = default;
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

  uint8_t readByte();
  uint32_t readLEU32();

private:
  std::vector<uint8_t> bytecode_{}; ///> wasm binary code read from .wasm file
  uint64_t offset_{};               ///> read offset of bytecode_
};