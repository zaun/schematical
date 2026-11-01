cmake_minimum_required(VERSION 3.16)

project(basicsim LANGUAGES C)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../backend backend)

add_executable(basicsim main.c)
target_link_libraries(basicsim PRIVATE logic)
# kind.h (kind_model) is internal to the library; basicsim builds engine-level instances.
target_include_directories(basicsim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../backend/src/library
    ${CMAKE_CURRENT_SOURCE_DIR}/../backend/src/third_party/cjson
)
target_compile_definitions(basicsim PRIVATE
    PARTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../schematical/schematical/Parts")
