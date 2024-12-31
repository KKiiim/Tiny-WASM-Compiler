cmake_minimum_required(VERSION 3.22)

project(tiny-wasm-compiler)
set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_COMPILER /usr/bin/cc)
set(CMAKE_CXX_COMPILER /usr/bin/c++)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
set(CMAKE_CXX_CLANG_TIDY clang-tidy --config-file ${CMAKE_CURRENT_LIST_DIR}/.clang-tidy "--header-filter=${CMAKE_CURRENT_LIST_DIR}/chapters/**/.*")

add_subdirectory(chapters/Chapter01)