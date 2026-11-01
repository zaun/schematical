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

# ---- plan-01-A baseline: goldens recorded from the old backend ------------
# tools/baseline dies with the old backend (plan-01-G); tests/support stays.
add_library(old_baseline STATIC
    tools/baseline/old_parts.c
    tests/support/trace_core.c
)
target_include_directories(old_baseline PUBLIC tools/baseline tests/support)
target_link_libraries(old_baseline PUBLIC logic_backend)

add_executable(old_trace tools/baseline/old_trace.c)
target_link_libraries(old_trace PRIVATE old_baseline)

add_executable(old_listing tools/baseline/old_listing.c)
target_link_libraries(old_listing PRIVATE old_baseline)

# The old backend must keep reproducing the goldens until plan-01-G deletes it.
add_test(NAME baseline_listing
    COMMAND sh -c "\"$1\" | diff - \"$2\"" sh $<TARGET_FILE:old_listing>
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/golden/parts.txt)
add_test(NAME baseline_traces
    COMMAND sh -c "\"$1\" | diff - \"$2\"" sh $<TARGET_FILE:old_trace>
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/golden/traces.txt)

# ---- plan-01: the part library (new backend) -------------------------------
# Builds next to logic_backend until plan-01-G removes the old files.
add_library(logic STATIC
    sim.c
    net.c
    src/library/library.c
    src/library/kinds.c
    src/library/kind_util.c
    src/third_party/cjson/cJSON.c
)
target_include_directories(logic
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/library
        ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/cjson
)
target_compile_features(logic PUBLIC c_std_11)
target_link_libraries(logic PUBLIC m)
