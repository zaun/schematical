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

# plan-68-L: each way the demo stops (fixtures: basicsim/tests/make_fixtures.py).
set(BASICSIM_TESTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
add_test(NAME basicsim_no_file COMMAND basicsim ${BASICSIM_TESTS}/no-such-file.json)
set_tests_properties(basicsim_no_file PROPERTIES
    PASS_REGULAR_EXPRESSION "Could not read .*no-such-file.json\nSimulation error: unknown")
add_test(NAME basicsim_bad_file COMMAND basicsim ${BASICSIM_TESTS}/make_fixtures.py)
set_tests_properties(basicsim_bad_file PROPERTIES
    PASS_REGULAR_EXPRESSION "make_fixtures.py.*\nSimulation error: unknown")
add_test(NAME basicsim_missing_part COMMAND basicsim ${BASICSIM_TESTS}/one_gate.json)
set_tests_properties(basicsim_missing_part PROPERTIES PASS_REGULAR_EXPRESSION "^Simulation error: unknown\n$")
add_test(NAME basicsim_slow_nand COMMAND basicsim ${BASICSIM_TESTS}/slow_gates.json)
set_tests_properties(basicsim_slow_nand PROPERTIES
    PASS_REGULAR_EXPRESSION "FAIL at 28 ns: expected AND = 1, NAND = 0\nSimulation error: unknown")
