cmake_minimum_required(VERSION 3.16)

project(basicsim LANGUAGES C)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../backend backend)

add_executable(basicsim main.c)
target_link_libraries(basicsim PRIVATE logic_backend)
