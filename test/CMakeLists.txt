find_package(GTest REQUIRED)

file(GLOB SRC_FILES
    ${CMAKE_CURRENT_LIST_DIR}/../src/backend/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/common/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/frontend/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/*.hpp
)

add_executable(Spectest ${SRC_FILES})

target_include_directories(Spectest PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}/../third_party/capstone/include
    ${CMAKE_CURRENT_LIST_DIR}/../third_party/googletest/googletest/include
)

target_link_libraries(Spectest 
    capstone
    GTest::gtest GTest::gtest_main
    pthread
)