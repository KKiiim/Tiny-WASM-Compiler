#include <cstdint>
#include <gtest/gtest.h>
#include <string>

#include "src/common/util.hpp"
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
TEST(Chapter02, Local1wasm) {
  Compiler compiler;
  ExecutableMemory const &execMemory = compiler.compile("chapters/Chapter02/local.1.wasm");

  compiler.singleCallByName<void>("type-local-i32", "()");
  compiler.singleCallByName<void>("type-local-i64", "()");
  compiler.singleCallByName<void>("type-param-i32", "(i)", 2U);
  compiler.singleCallByName<void>("type-param-i64", "(I)", 3U);
  compiler.singleCallByName<void>("type-mixed", "(Iii)", 0U, 0U, 0U);
}
TEST(Chapter02, Local2wasm) {
  Compiler compiler;
  ExecutableMemory const &execMemory = compiler.compile("chapters/Chapter02/local.2.wasm");

  uint32_t ret32 = compiler.singleCallByName<uint32_t>("type-local-i32", "i()");
  EXPECT_EQ(ret32, 1U);
  uint64_t ret64 = compiler.singleCallByName<uint64_t>("type-local-i64", "I()");
  EXPECT_EQ(ret64, 1U);
  ret32 = compiler.singleCallByName<uint32_t>("type-param-i32", "i(i)", 2U);
  EXPECT_EQ(ret32, 10U);
  ret64 = compiler.singleCallByName<uint64_t>("type-param-i64", "I(I)", 3U);
  EXPECT_EQ(ret64, 11U);

  compiler.singleCallByName<void>("as-local.set-value", "()");

  ret32 = compiler.singleCallByName<uint32_t>("as-local.tee-value", "i(i)", 0U);
  EXPECT_EQ(ret32, 1U);
}

TEST(Chapter03, arithmetic0) {
  Compiler compiler;
  ExecutableMemory const &execMemory = compiler.compile("chapters/Chapter03/arithmetic.0.wasm");

  // i32.add
  EXPECT_EQ(2U, static_cast<uint32_t>(compiler.singleCallByName<uint32_t>("add", "i(ii)", 1U, 1U)));
  EXPECT_EQ(1U, static_cast<uint32_t>(compiler.singleCallByName<uint32_t>("add", "i(ii)", 1U, 0U)));
  EXPECT_EQ(-2, static_cast<uint32_t>(compiler.singleCallByName<uint32_t>("add", "i(ii)", -1, -1)));
  EXPECT_EQ(0, static_cast<uint32_t>(compiler.singleCallByName<uint32_t>("add", "i(ii)", -1, 1U)));
  EXPECT_EQ(0x80000000, static_cast<uint32_t>(compiler.singleCallByName<uint32_t>("add", "i(ii)", 0x7fffffff, 1U)));
  EXPECT_EQ(0x7fffffff, static_cast<uint32_t>(compiler.singleCallByName<uint32_t>("add", "i(ii)", 0x80000000, -1)));
  EXPECT_EQ(0, static_cast<uint32_t>(compiler.singleCallByName<uint32_t>("add", "i(ii)", 0x80000000, 0x80000000)));
  EXPECT_EQ(0x40000000, static_cast<uint32_t>(compiler.singleCallByName<uint32_t>("add", "i(ii)", 0x3fffffff, 1U)));
}

TEST(Chapter03, arithmetic1) {
  Compiler compiler;
  ExecutableMemory const &execMemory = compiler.compile("chapters/Chapter03/arithmetic.1.wasm");

  // i64.add
  EXPECT_EQ(2U, static_cast<uint64_t>(compiler.singleCallByName<uint64_t>("add", "I(II)", 1U, 1U)));
  EXPECT_EQ(1U, static_cast<uint64_t>(compiler.singleCallByName<uint64_t>("add", "I(II)", 1U, 0U)));
  EXPECT_EQ(-2, static_cast<uint64_t>(compiler.singleCallByName<uint64_t>("add", "I(II)", static_cast<uint64_t>(-1), static_cast<uint64_t>(-1))));
  EXPECT_EQ(0, static_cast<uint64_t>(compiler.singleCallByName<uint64_t>("add", "I(II)", static_cast<uint64_t>(-1), 1U)));
  EXPECT_EQ(0x8000000000000000,
            static_cast<uint64_t>(compiler.singleCallByName<uint64_t>("add", "I(II)", static_cast<uint64_t>(0x7fffffffffffffff), 1U)));
  EXPECT_EQ(0x7fffffffffffffff,
            static_cast<uint64_t>(compiler.singleCallByName<uint64_t>("add", "I(II)", 0x8000000000000000, static_cast<uint64_t>(-1))));
  EXPECT_EQ(0, static_cast<uint64_t>(compiler.singleCallByName<uint64_t>("add", "I(II)", 0x8000000000000000, 0x8000000000000000)));
  EXPECT_EQ(0x4000000000000000, static_cast<uint64_t>(compiler.singleCallByName<uint64_t>("add", "I(II)", 0x3fffffffffffffff, 1U)));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}