set(CMAKE_CXX_STANDARD 17)
project(Spectest)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} sourceFiles)

set(GTEST_LIB_DIR ${WASM_COMPILER_INCLUDE_ROOT}/libs) 
set(GTEST_INCLUDE_DIR ${WASM_COMPILER_INCLUDE_ROOT}/third_party/googletest/googletest/include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(${PROJECT_NAME} ${sourceFiles} ${COMPILER_SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${GTEST_INCLUDE_DIR}
    ${WASM_COMPILER_INCLUDE_ROOT}/third_party/capstone/include
)

target_link_directories(${PROJECT_NAME} PRIVATE ${GTEST_LIB_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE
    capstone
    gtest  # libgtest.a
    gtest_main # libgtest_main.a
)

add_test(NAME ${PROJECT_NAME} COMMAND "qemu-aarch64" -L /usr/aarch64-linux-gnu ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME})