cmake_minimum_required(VERSION 3.28)

# option() has lower priority than the user-supplied arguments
cmake_policy(SET CMP0077 NEW)

# Main project
project(atcmd
    VERSION 0.0.1
    LANGUAGES CXX)

# Configuration options
include(CMakeDependentOption)

option(ATCMD_BUILD_EXAMPLES "Build example applications" OFF)
cmake_dependent_option(ATCMD_BUILD_EXAMPLES_AT_TERMINAL
    "Build AT-terminal example"
    ON
    ATCMD_BUILD_EXAMPLES
    OFF)
cmake_dependent_option(ATCMD_BUILD_EXAMPLES_ZEPHYR_ESP32_C3
    "Build Zephyr ESP32-C3 example"
    OFF
    ATCMD_BUILD_EXAMPLES
    OFF)

option(ATCMD_BUILD_TESTS "Build unit tests" OFF)
cmake_dependent_option(ATCMD_COVERAGE
    "Enable coverage flags (GCC/Clang only)"
    OFF
    "(NOT ATCMD_BUILD_EXAMPLES) AND ATCMD_BUILD_TESTS"
    OFF)

option(ATCMD_BUILD_DOCUMENTATION "Build Doxygen documentation" OFF)

# Add subdirectories
add_subdirectory(lib)

if(ATCMD_BUILD_EXAMPLES)
    if(ATCMD_BUILD_EXAMPLES_AT_TERMINAL)
        add_subdirectory(examples/at_terminal)
        message(STATUS "Building AT-terminal example")
    endif()
else()
    message(STATUS "Examples disabled")
endif()

if(ATCMD_BUILD_TESTS)
    add_subdirectory(tests)
    message(STATUS "Building tests")
else()
    message(STATUS "Tests disabled - not building tests")
endif()
