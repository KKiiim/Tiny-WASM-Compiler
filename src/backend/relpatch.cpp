#include "relpatch.hpp"

void Relpatch::linkedHere() const {
  // The jump instruction is a placeholder, we need to patch it with the correct address
  int32_t const currentOffset = static_cast<int32_t>((as_.getCurrentOffset() - jumpInsPos_) / 4);
  if (isConditional_) {
    as_.set_b_cond_off(jumpInsPos_, currentOffset);
  } else {
    as_.set_b_off(jumpInsPos_, currentOffset);
  }
}