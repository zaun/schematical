cmake_minimum_required(VERSION 3.16)

project(logic LANGUAGES C)

# ---- The backend: event engine, part library, models, circuit API --------
# The app compiles the same files (backend/src) through an Xcode
# synchronized folder; the public headers are backend/include.
add_library(logic STATIC
    src/engine/sim.c
    src/engine/net.c
    src/circuit.c
    src/memory_image.c
    src/voltage.c
    src/library/library.c
    src/library/kinds.c
    src/library/kind_util.c
    src/library/footprints.c
    src/models/lights.c
    src/models/gate.c
    src/models/dff.c
    src/models/jkff.c
    src/models/switch.c
    src/models/sram.c
    src/models/display7.c
    src/models/supply.c
    src/models/led.c
    src/models/oscillator.c
    src/models/eeprom.c
    src/models/seq.c
    src/models/latch.c
    src/models/counter.c
    src/third_party/cjson/cJSON.c
)
target_include_directories(logic
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/engine
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/library
        ${CMAKE_CURRENT_SOURCE_DIR}/src/models
        ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/cjson
)
target_compile_features(logic PUBLIC c_std_11)
target_link_libraries(logic PUBLIC m)

enable_testing()

# ---- Tests ----------------------------------------------------------------
# tests/golden holds one listing line and one trace hash per part, recorded
# from the original C tables (plan-01-A); listing_check and trace_check keep
# the part files and models equal to them.
add_library(logic_test_support STATIC
    tests/support/read_file.c
    tests/support/listing.c
    tests/support/golden.c
    tests/support/trace_core.c
    tests/support/bench.c
)
target_include_directories(logic_test_support PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/support
    ${CMAKE_CURRENT_SOURCE_DIR}/src/library
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
    ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/cjson
)
target_link_libraries(logic_test_support PUBLIC logic)

function(logic_test NAME)
    add_executable(${NAME} tests/${NAME}.c)
    target_link_libraries(${NAME} PRIVATE logic_test_support)
    target_compile_definitions(${NAME} PRIVATE
        PARTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../schematical/schematical/Parts"
        GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/golden")
    add_test(NAME ${NAME} COMMAND ${NAME})
endfunction()

logic_test(library_check)
logic_test(listing_check)
logic_test(trace_check)
logic_test(switch_test)
logic_test(sram_test)
logic_test(display7_test)
logic_test(circuit_check)
logic_test(footprint_check)
logic_test(engine_test)
logic_test(eeprom_test)
logic_test(latch_test)
logic_test(counter_test)
logic_test(memory_image_test)
logic_test(voltage_test)

# Engine speed bench (plan-05-A). Not a ctest: a wall-clock check would be
# flaky. Build Release to get meaningful numbers.
add_executable(sim_bench tests/sim_bench.c)
target_link_libraries(sim_bench PRIVATE logic_test_support)
target_compile_definitions(sim_bench PRIVATE
    PARTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../schematical/schematical/Parts")
