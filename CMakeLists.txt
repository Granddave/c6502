cmake_minimum_required(VERSION 3.10.2)

project(cpu-6502
    LANGUAGES CXX)

set(COMPILER_WARNINGS -Werror -Wall -Wextra)

add_subdirectory(external/Catch2)

# Library
add_library(c6502
    ${CMAKE_CURRENT_SOURCE_DIR}/include/c6502/c6502.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/c6502.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/c6502AddrModes.cpp
)
target_include_directories(c6502 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(c6502 PRIVATE ${COMPILER_WARNINGS})
set_target_properties(c6502 PROPERTIES CXX_STANDARD 17)


# Test
add_executable(c6502-test
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_c6502.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_c6502.h
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_insLoad.cpp
)
target_link_libraries(c6502-test PRIVATE
    c6502
    Catch2::Catch2
)
target_include_directories(c6502-test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/test)
target_compile_options(c6502-test PRIVATE ${COMPILER_WARNINGS})
set_target_properties(c6502-test PROPERTIES CXX_STANDARD 17)
