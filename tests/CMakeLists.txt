project(Spectest)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} sourceFiles)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(${PROJECT_NAME} ${sourceFiles})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${WASM_COMPILER_INCLUDE_ROOT}/third_party/googletest/googletest/include
    ${WASM_COMPILER_INCLUDE_ROOT}/third_party/capstone/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    lib_src
    gtest  # libgtest.a
    gtest_main # libgtest_main.a
)

add_test(NAME ${PROJECT_NAME} COMMAND "qemu-aarch64" -L /usr/aarch64-linux-gnu ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME})