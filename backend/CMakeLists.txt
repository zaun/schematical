cmake_minimum_required(VERSION 3.16)

project(logic LANGUAGES C)

# ---- The backend: event engine, part library, models, circuit API --------
# The app compiles the same files (backend/src) through an Xcode
# synchronized folder; the public headers are backend/include.
add_library(logic STATIC
    src/engine/sim.c
    src/engine/sim_contacts.c
    src/engine/net.c
    src/circuit.c
    src/logic_alloc.c
    src/timer_rc.c
    src/memory_image.c
    src/voltage.c
    src/erc_pins.c
    src/erc_check.c
    src/library/library.c
    src/library/library_load.c
    src/library/library_load_device.c
    src/library/kinds.c
    src/library/kind_util.c
    src/library/footprints.c
    src/models/lights.c
    src/models/gate.c
    src/models/dff.c
    src/models/jkff.c
    src/models/switch.c
    src/models/diode.c
    src/models/capacitor.c
    src/models/mcu.c
    src/models/mcu_avr.c
    src/models/mcu_avr_serial.c
    src/models/avr_cpu.c
    src/models/avr_instruction.c
    src/models/avr_dasm.c
    src/models/avr_gpio.c
    src/models/avr_clock.c
    src/models/avr_timer.c
    src/models/avr_timer_tiny.c
    src/models/avr_usi.c
    src/models/avr_usart.c
    src/models/avr_spi.c
    src/models/avr_twi.c
    src/models/avr_adc.c
    src/models/avr_eeprom.c
    src/models/avr_watchdog.c
    src/models/avr_chip.c
    src/models/pld.c
    src/models/resistor.c
    src/models/transistor.c
    src/models/generic.c
    src/models/serialmem.c
    src/models/serialmem_cmd.c
    src/models/serialmem_load.c
    src/models/sram.c
    src/models/display7.c
    src/models/supply.c
    src/models/led.c
    src/models/nixie.c
    src/models/dekatron.c
    src/models/lcd.c
    src/models/regulator.c
    src/models/crystal.c
    src/models/powerswitch.c
    src/models/terminals.c
    src/models/inductor.c
    src/models/fuse.c
    src/models/relay.c
    src/models/buzzer.c
    src/models/timer.c
    src/models/opamp.c
    src/models/model_pins.c
    src/models/oscillator.c
    src/models/eeprom.c
    src/models/eeprom_kind.c
    src/models/seq.c
    src/models/latch.c
    src/models/register.c
    src/models/decoder.c
    src/models/mux.c
    src/models/mux_stored.c
    src/models/arith.c
    src/models/arith_alu.c
    src/models/counter.c
    src/models/counter_ripple.c
    src/models/counter_sync.c
    src/models/counter_sync_load.c
    src/models/counter_div.c
    src/models/counter_seg7.c
    src/models/shift.c
    src/models/shift_load.c
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
    src/models/charlcd_load.c
    src/models/charlcd_font.c
    src/models/serial.c
    src/models/serial_line.c
    src/models/serial_bus.c
    src/models/ns16550.c
    src/models/ns16550_io.c
    src/models/ns16550_kind.c
    src/models/ns16550_load.c
    src/models/ns16550_ext.c
    src/models/ns16550_lpt.c
    src/models/ns16550_exar.c
    src/models/ns16550_exar_load.c
    src/models/ns16550_exar_flow.c
    src/models/ns16550_exar_pins.c
    src/models/serial_target.c
    src/models/converter.c
    src/models/converter_model.c
    src/models/converter_proto.c
    src/models/converter_load.c
    src/models/converter_load_conv.c
    src/models/converter_load_display.c
    src/models/converter_load_lcd.c
    src/models/converter_load_proto.c
    src/models/converter_load_rtc.c
    src/models/converter_par.c
    src/models/rtc.c
    src/models/display_out.c
    src/models/display_clock.c
    src/models/display_ccsi.c
    src/models/leddriver.c
    src/models/lcddriver.c
    src/models/ns16550_bridge.c
    src/models/ns16550_bridge_kind.c
    src/models/ns16550_xr1280.c
    src/models/serial_max3100.c
    src/models/serial_max310x.c
    src/models/serial_max310x_data.c
    src/models/serial_max310x_regs.c
    src/models/serial_ay1013.c
    src/models/serial_i8251.c
    src/models/serial_i82510.c
    src/models/serial_i82510_regs.c
    src/models/serial_acia6551.c
    src/models/serial_usbuart.c
    src/models/serial_hs82c52.c
    src/models/serial_com78c80x.c
    src/models/serial_sc18is604.c
    src/models/connector.c
    src/models/header.c
    src/models/socket.c
    src/models/pio.c
    src/models/bus65.c
    src/models/pio_pia6521.c
    src/models/pio_via6522.c
    src/models/pio_via6522_regs.c
    src/models/buffer.c
    src/models/buffer_reg.c
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/support
    ${CMAKE_CURRENT_SOURCE_DIR}/src/library
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
    ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/cjson
)
target_link_libraries(logic_test_support PUBLIC logic)

function(logic_test NAME)
    # alloc_check.c: the program fails if it leaves backend memory (plan-68-K).
    add_executable(${NAME} tests/${NAME}.c tests/support/alloc_check.c)
    target_link_libraries(${NAME} PRIVATE logic_test_support)
    target_compile_definitions(${NAME} PRIVATE
        PARTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../schematical/schematical/Parts"
        GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/golden")
    add_test(NAME ${NAME} COMMAND ${NAME})
endfunction()

# plan-68-H: a test program split from a bigger one, with the shared
# helpers of its family compiled in (tests/support/<bench>.c).
function(logic_bench_test NAME BENCH)
    logic_test(${NAME})
    target_sources(${NAME} PRIVATE tests/support/${BENCH}.c)
endfunction()

logic_bench_test(library_check library_check_bench)
logic_bench_test(library_logic_check library_check_bench)
logic_bench_test(library_parts_check library_check_bench)
logic_bench_test(library_pins_check library_check_bench)
logic_bench_test(library_footprint_check library_check_bench)
logic_bench_test(library_voltage_check library_check_bench)
logic_bench_test(library_reject_check library_check_bench)
logic_bench_test(library_footprint_more_check library_check_bench)
logic_test(listing_check)
logic_test(part_unit_test)
logic_test(trace_check)
logic_test(switch_test)
logic_test(connector_test)
logic_test(sram_test)
logic_test(display7_test)
logic_bench_test(circuit_check circuit_bench)
logic_test(circuit_misuse_test)
logic_bench_test(circuit_adc_check circuit_bench)
logic_test(footprint_check)
logic_test(part_file_test)
logic_test(engine_test)
logic_test(engine_misuse_test)
logic_test(eeprom_test)
logic_bench_test(latch_test latch_bench)
logic_test(register_test)
logic_test(decoder_test)
logic_test(mux_test)
logic_test(arith_test)
logic_bench_test(counter_test counter_bench)
logic_bench_test(counter_div_test counter_bench)
logic_test(counter_sn74143_test)
logic_bench_test(shift_test shift_bench)
logic_bench_test(shift_special_test shift_bench)
logic_test(memory_image_test)
# plan-68-K: the wrapper itself (without alloc_check.c: it frees foreign memory on purpose).
add_executable(logic_alloc_test tests/logic_alloc_test.c)
target_link_libraries(logic_alloc_test PRIVATE logic_test_support)
add_test(NAME logic_alloc_test COMMAND logic_alloc_test)
logic_test(oom_test)
logic_test(voltage_test)
logic_test(erc_pins_test)
logic_test(erc_pins_check)
logic_test(erc_test)
logic_test(erc_more_test)
logic_test(buffer_test)
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
    # plan-68-A: no in-scope source file over 1000 lines of code (docs/testing.md).
    add_test(NAME source_size_check COMMAND ${LOGIC_NODE}
        ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/source-size.js --check --root ${CMAKE_CURRENT_SOURCE_DIR}/..)
endif()
logic_bench_test(cpu_test cpu_test_bench)
logic_bench_test(cpu_z80_test cpu_test_bench)
logic_bench_test(cpu_65816_test cpu_test_bench)
logic_bench_test(cpu_io_board_test cpu_test_bench)
# plan-62: the AVR core (a C port of avr8js) against avr8js itself. With node,
# the tests run tools/avr8js-oracle/oracle.js and compare line by line;
# without it they compare with the hashes in tests/avr/golden.txt.
logic_test(avr_core_test)
logic_test(avr_periph_test)
logic_bench_test(charlcd_test charlcd_bench)
logic_bench_test(charlcd_4row_test charlcd_bench)
logic_bench_test(charlcd_board_test charlcd_bench)
# plan-68-G: the serial tests, one program per family, on shared benches
# compiled into the serial programs only (support/serial_bench*.c).
function(logic_serial_test NAME)
    logic_test(${NAME})
    set(sources tests/support/serial_bench.c)
    foreach(extra ${ARGN})
        list(APPEND sources tests/support/${extra}.c)
    endforeach()
    target_sources(${NAME} PRIVATE ${sources})
endfunction()
logic_serial_test(serial_ns16550_test)
logic_serial_test(serial_d_test serial_bench_d)
logic_serial_test(serial_d_more_test serial_bench_d)
logic_serial_test(serial_maxim_test serial_bench_maxim)
logic_serial_test(serial_max310x_test serial_bench_maxim)
logic_serial_test(serial_ay1013_test serial_bench_classic)
logic_serial_test(serial_intel_test serial_bench_classic)
logic_serial_test(serial_acia_test serial_bench_classic)
logic_serial_test(serial_hs_com_test serial_bench_classic)
logic_serial_test(serial_bridge_test serial_bench_maxim serial_bench_bridge)
logic_serial_test(serial_sc18is604_test serial_bench_maxim serial_bench_bridge)
logic_serial_test(serial_exar_test serial_bench_d serial_bench_exar)
logic_serial_test(serial_exar_v_test serial_bench_d serial_bench_exar)
logic_test(usbuart_test)
logic_bench_test(pio_test pio_bench)
logic_bench_test(pio_via_test pio_bench)
logic_test(converter_test)
logic_test(rtc_test)
logic_test(display_out_test)
logic_test(pcf1179c_test)
logic_test(tlc6983_test)
logic_test(icm7218_test)
logic_test(icm7228_test)
logic_test(icm7211_test)
logic_test(as1115_test)
logic_test(max6950_test)
logic_test(max6957_test)
logic_test(max6954_max6955_test)
logic_test(pcf8576_family_test)
logic_test(lcd_spi_i2c_test)
logic_test(icm7231_max7231_test)
logic_test(cd22105_icm7224_test)
logic_test(tactile_test)
logic_test(slide_test)
logic_test(diode_test)
logic_test(resistor_test)
logic_test(transistor_test)
logic_test(capacitor_test)
logic_test(mcu_test)
logic_test(avr_serial_test)
logic_test(avr_board_test)
logic_test(avr_tiny_test)
foreach(AVR_TEST avr_core_test avr_periph_test mcu_test avr_serial_test avr_board_test avr_tiny_test)
    target_compile_definitions(${AVR_TEST} PRIVATE
        AVR_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/avr"
        AVR_TMP="${CMAKE_CURRENT_BINARY_DIR}")
    if(LOGIC_NODE)
        target_compile_definitions(${AVR_TEST} PRIVATE
            AVR_NODE="${LOGIC_NODE}"
            AVR_ORACLE="${CMAKE_CURRENT_SOURCE_DIR}/tools/avr8js-oracle/oracle.js")
    endif()
endforeach()
logic_test(generic_test)
logic_test(pld_test)
logic_test(socket_test)
logic_bench_test(serialmem_test serialmem_bench)
logic_bench_test(serialmem_flash_test serialmem_bench)
logic_test(nixie_test)
logic_test(dekatron_test)
logic_test(lcd_test)
logic_test(regulator_test)
logic_test(voltage_more_test)
logic_test(crystal_test)
logic_test(powerswitch_test)
logic_test(inductor_test)
logic_test(fuse_test)
logic_test(relay_test)
logic_test(buzzer_test)
logic_test(timer_test)
logic_test(opamp_test)
# plan-68-O
logic_serial_test(serial_bridge_more_test serial_bench_maxim serial_bench_bridge serial_bench_cov)
logic_serial_test(serial_xr1280_more_test serial_bench_maxim serial_bench_bridge serial_bench_cov)
logic_serial_test(serial_sc18is604_more_test serial_bench_maxim serial_bench_bridge serial_bench_cov)
logic_serial_test(serial_target_more_test serial_bench_maxim serial_bench_bridge serial_bench_cov)
# plan-68-P
logic_serial_test(serial_max310x_cov_test serial_bench_maxim)
logic_serial_test(serial_max310x_cov2_test serial_bench_maxim)
logic_serial_test(serial_max3100_cov_test serial_bench_maxim)
logic_serial_test(serial_i82510_cov_test serial_bench_classic)
logic_serial_test(serial_i8251_cov_test serial_bench_classic)
logic_serial_test(serial_acia_cov_test serial_bench_classic)
logic_serial_test(serial_ay1013_cov_test serial_bench_classic)
logic_serial_test(serial_hs_com_cov_test serial_bench_classic)
logic_test(usbuart_cov_test)
logic_serial_test(serial_core_cov_test serial_bench_classic serial_bench_maxim serial_bench_d)
# plan-68-M
logic_bench_test(converter_load_cov_test library_check_bench)
logic_bench_test(converter_disp_load_cov_test library_check_bench)
logic_bench_test(converter_more_test converter_bench)
logic_bench_test(converter_more2_test converter_bench)
logic_bench_test(rtc_more_test converter_bench)
# plan-68-N
logic_serial_test(serial_n68_load_test serial_bench_n68)
logic_serial_test(serial_n68_core_test serial_bench_d serial_bench_exar serial_bench_n68)
logic_serial_test(serial_n68_ext_test serial_bench_d serial_bench_exar serial_bench_n68)
logic_serial_test(serial_n68_exar_test serial_bench_d serial_bench_exar serial_bench_n68)
logic_serial_test(serial_n68_5g_test serial_bench_d serial_bench_exar serial_bench_n68)
logic_serial_test(serial_n68_bridge_test serial_bench_maxim serial_bench_bridge serial_bench_n68)
# plan-68-Q
logic_bench_test(logic_ff_cov_test q_cov_bench)
logic_bench_test(counter_sync_cov_test q_cov_bench)
logic_bench_test(counter_div_cov_test q_cov_bench)
logic_bench_test(shift_cov_test q_cov_bench)
logic_bench_test(shift_special_cov_test q_cov_bench)
logic_bench_test(latch_cov_test q_cov_bench)
logic_bench_test(buffer_cov_test q_cov_bench)
logic_bench_test(logic_misc_cov_test q_cov_bench)

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
