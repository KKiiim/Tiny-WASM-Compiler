set(LibName lib_src)

file(GLOB SRC_FILES 
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/backend/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/frontend/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/common/*.cpp
)

add_library(${LibName} ${SRC_FILES})
target_include_directories(${LibName} PUBLIC "$<BUILD_INTERFACE:${WASM_COMPILER_INCLUDE_ROOT}>")

target_link_libraries(${LibName} PUBLIC capstone_static)