set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_COMPILER /usr/bin/cc)
set(CMAKE_CXX_COMPILER /usr/bin/c++)

file(GLOB SRC_FILES
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
)

add_executable(parser ${SRC_FILES})