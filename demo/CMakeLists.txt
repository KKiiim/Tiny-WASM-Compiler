file(GLOB SRC_FILES
    ${CMAKE_CURRENT_LIST_DIR}/../src/backend/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/common/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/frontend/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/demo.cpp
)

add_executable(Compiler ${SRC_FILES})
target_link_libraries(Compiler capstone)
target_include_directories(Compiler PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../third_party/capstone/include)