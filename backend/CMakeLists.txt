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
    src/models/counter_div.c
    src/models/shift.c
    src/models/shift_special.c
    src/models/cpu.c
    src/models/cpu6502.c
    src/models/cpuz80.c
    src/models/cpu65816.c
    src/models/chips_m6502.c
    src/models/chips_w65c02.c
    src/models/chips_z80.c
    src/models/chips_dasm.c
    src/models/charlcd.c
    src/models/charlcd_font.c
    src/models/serial.c
    src/models/serial_line.c
    src/models/serial_bus.c
    src/models/ns16550.c
    src/models/ns16550_ext.c
    src/models/ns16550_lpt.c
    src/models/ns16550_exar.c
    src/models/serial_target.c
    src/models/ns16550_bridge.c
    src/models/ns16550_xr1280.c
    src/models/serial_max3100.c
    src/models/serial_max310x.c
    src/models/serial_ay1013.c
    src/models/serial_i8251.c
    src/models/serial_i82510.c
    src/models/serial_acia6551.c
    src/models/serial_hs82c52.c
    src/models/serial_com78c80x.c
    src/models/serial_sc18is604.c
    src/models/connector.c
    src/models/pio.c
    src/models/bus65.c
    src/models/pio_pia6521.c
    src/models/pio_via6522.c
    src/third_party/cjson/cJSON.c
    src/third_party/jsmooch/wdc65816.c
    src/third_party/jsmooch/wdc65816_generated_opcodes.c
    src/third_party/jsmooch/wdc65816_disassembler.c
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
    tests/support/cpu_board.c
    tests/support/io_board.c
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
logic_test(connector_test)
logic_test(sram_test)
logic_test(display7_test)
logic_test(circuit_check)
logic_test(footprint_check)
logic_test(engine_test)
logic_test(eeprom_test)
logic_test(latch_test)
logic_test(counter_test)
logic_test(shift_test)
logic_test(memory_image_test)
logic_test(voltage_test)
logic_test(cpu_core_test)
target_compile_definitions(cpu_core_test PRIVATE
    CPU_VECTORS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/cpu")
# plan-14-G: with node, cpu_core_test checks the 65816 generator reproduces
# the committed opcode table.
find_program(LOGIC_NODE node)
if(LOGIC_NODE)
    target_compile_definitions(cpu_core_test PRIVATE
        JSMOOCH_NODE="${LOGIC_NODE}"
        JSMOOCH_GEN="${CMAKE_CURRENT_SOURCE_DIR}/tools/jsmooch-gen/run.js"
        JSMOOCH_C="${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/jsmooch/wdc65816_generated_opcodes.c"
        JSMOOCH_TMP="${CMAKE_CURRENT_BINARY_DIR}")
endif()
logic_test(cpu_test)
logic_test(charlcd_test)
logic_test(serial_test)
logic_test(pio_test)

# Engine speed bench (plan-05-A). Not a ctest: a wall-clock check would be
# flaky. Build Release to get meaningful numbers.
add_executable(sim_bench tests/sim_bench.c)
target_link_libraries(sim_bench PRIVATE logic_test_support)
target_compile_definitions(sim_bench PRIVATE
    PARTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../schematical/schematical/Parts")

# CPU board speed (plan-09-D). Not a ctest, like sim_bench.
add_executable(cpu_bench tests/cpu_bench.c)
target_link_libraries(cpu_bench PRIVATE logic_test_support)
target_compile_definitions(cpu_bench PRIVATE
    PARTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../schematical/schematical/Parts")
