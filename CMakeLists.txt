cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_COMPILER "/usr/bin/aarch64-linux-gnu-gcc" CACHE PATH "C compiler")
set(CMAKE_CXX_COMPILER "/usr/bin/aarch64-linux-gnu-g++" CACHE PATH "C++ compiler")
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

project(tiny-wasm-compiler)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

message(STATUS "CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
set(CMAKE_CXX_CLANG_TIDY clang-tidy --config-file ${CMAKE_CURRENT_LIST_DIR}/.clang-tidy "--header-filter=${CMAKE_CURRENT_LIST_DIR}/src/**/.*")

set(CAPSTONE_ARM64_SUPPORT ON CACHE BOOL "Enable ARM64 support")

set(CAPSTONE_ARM_SUPPORT OFF CACHE BOOL "Disable ARM support")
set(CAPSTONE_M68K_SUPPORT OFF CACHE BOOL "Disable M68K support")
set(CAPSTONE_MIPS_SUPPORT OFF CACHE BOOL "Disable MIPS support")
set(CAPSTONE_PPC_SUPPORT OFF CACHE BOOL "Disable PPC support")
set(CAPSTONE_SPARC_SUPPORT OFF CACHE BOOL "Disable SPARC support")
set(CAPSTONE_SYSZ_SUPPORT OFF CACHE BOOL "Disable SYSZ support")
set(CAPSTONE_XCORE_SUPPORT OFF CACHE BOOL "Disable XCORE support")
set(CAPSTONE_X86_SUPPORT OFF CACHE BOOL "Disable X86 support")
set(CAPSTONE_TMS320C64X_SUPPORT OFF CACHE BOOL "Disable TMS320C64X support")
set(CAPSTONE_M680X_SUPPORT OFF CACHE BOOL "Disable M680X support")
set(CAPSTONE_EVM_SUPPORT OFF CACHE BOOL "Disable EVM support")
set(CAPSTONE_MOS65XX_SUPPORT OFF CACHE BOOL "Disable MOS65XX support")
set(CAPSTONE_WASM_SUPPORT OFF CACHE BOOL "Disable WASM support")
set(CAPSTONE_BPF_SUPPORT OFF CACHE BOOL "Disable BPF support")
set(CAPSTONE_RISCV_SUPPORT OFF CACHE BOOL "Disable RISCV support")
set(CAPSTONE_SH_SUPPORT OFF CACHE BOOL "Disable SH support")
set(CAPSTONE_TRICORE_SUPPORT OFF CACHE BOOL "Disable TRICORE support")

add_subdirectory(third_party/capstone)
add_subdirectory(demo)
