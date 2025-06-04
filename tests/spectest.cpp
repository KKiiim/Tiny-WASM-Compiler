#include <cstdint>
#include <gtest/gtest.h>
#include <string>

#include "src/compiler.hpp"

TEST(Chapter02, Local0wasm) {
  Compiler compiler;
  ExecutableMemory const &execMemory = compiler.compile("chapters/Chapter02/local.0.wasm");

  uint32_t ret32 = compiler.singleCallByName<uint32_t>("type-local-i32", "i()");
  EXPECT_EQ(ret32, 0U);
  uint64_t ret64 = compiler.singleCallByName<uint64_t>("type-local-i64", "I()");
  EXPECT_EQ(ret64, 0U);
  ret32 = compiler.singleCallByName<uint32_t>("type-param-i32", "i(i)", 2U);
  EXPECT_EQ(ret32, 2U);
  ret64 = compiler.singleCallByName<uint64_t>("type-param-i64", "I(I)", 3U);
  EXPECT_EQ(ret64, 3U);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}