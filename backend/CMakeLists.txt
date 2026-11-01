cmake_minimum_required(VERSION 3.16)

project(logic_backend LANGUAGES C)

add_library(logic_backend STATIC
    net.c
    sim.c
    catalog.c
    74_00.c
    74_08.c
    74_32.c
    74_02.c
    74_86.c
    74_266.c
    ff-d.c
    ff-jk.c
    led.c
    supply.c
    catalog_validate.c
    osc.c
    netlist_sim.c
    cd4000.c
    gates.c
    sram.c
    switch.c
    led7.c
)

target_include_directories(logic_backend
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(logic_backend PUBLIC c_std_11)

enable_testing()

add_executable(catalog_check catalog_check.c)
target_link_libraries(catalog_check PRIVATE logic_backend)
add_test(NAME catalog_check COMMAND catalog_check)

add_executable(netlist_check netlist_check.c)
target_link_libraries(netlist_check PRIVATE logic_backend)
add_test(NAME netlist_check COMMAND netlist_check)

add_executable(sram_check sram_check.c)
target_link_libraries(sram_check PRIVATE logic_backend)
add_test(NAME sram_check COMMAND sram_check)

add_executable(switch_check switch_check.c)
target_link_libraries(switch_check PRIVATE logic_backend)
add_test(NAME switch_check COMMAND switch_check)

add_executable(led7_check led7_check.c)
target_link_libraries(led7_check PRIVATE logic_backend)
add_test(NAME led7_check COMMAND led7_check)
