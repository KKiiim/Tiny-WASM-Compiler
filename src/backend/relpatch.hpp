#ifndef SRC_BACKEND_RELPATCH_HPP
#define SRC_BACKEND_RELPATCH_HPP

#include <cstdint>

#include "src/backend/aarch64Assembler.hpp"
class Relpatch {
  friend Assembler;

public:
  explicit Relpatch(Assembler &assembler, bool const isConditional)
      : as_(assembler), jumpInsPos_(assembler.getCurrentOffset()), isConditional_(isConditional) {
  }

  void linkedHere() const;

private:
  Assembler &as_;
  uint32_t jumpInsPos_;
  bool isConditional_;
};

#endif // SRC_BACKEND_RELPATCH_HPP
