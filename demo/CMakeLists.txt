file(GLOB DEMO_FILES
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)

add_executable(demo ${COMPILER_SRC_FILES} ${DEMO_FILES})
target_link_libraries(demo lib_src)
target_include_directories(demo PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../third_party/capstone/include)