cmake_minimum_required(VERSION 3.16)

project(basicsim LANGUAGES C)

# Registers the backend's tests here too, so ctest on this build runs them all
# (scripts/coverage.js builds this project for the backend coverage tier).
enable_testing()

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

# plan-68-A: the demo runs to its last checkpoint.
add_test(NAME basicsim COMMAND basicsim)
set_tests_properties(basicsim PROPERTIES PASS_REGULAR_EXPRESSION "55 ns: AND = 0, NAND = 1")
