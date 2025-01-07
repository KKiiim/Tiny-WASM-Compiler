file(GLOB SRC_FILES
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
)

add_executable(Compiler ${SRC_FILES})