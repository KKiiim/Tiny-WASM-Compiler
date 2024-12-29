cmake_minimum_required(VERSION 3.15)
project(arm64_sum_fibo)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Og")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Og")

set(ASM_SOURCE ${HOME}/arm64asm.s)
set(C_SOURCE ${HOME}/src/qsort.cpp)

enable_language(ASM)
add_executable(arm64asm_test
  arm64asm.s
  googleTest.cc
)

enable_testing()
target_link_libraries(arm64asm_test
    gtest
    gtest_main
    pthread
)

add_test(
    NAME arm64asm_test
    COMMAND "qemu-aarch64" -L /usr/aarch64-linux-gnu ./output/arm64asm_test
)