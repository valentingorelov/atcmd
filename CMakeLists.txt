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
    if(ATCMD_BUILD_EXAMPLES_ZEPHYR_ESP32_C3)
        if(NOT DEFINED CMAKE_C_COMPILER_RISCV64 OR CMAKE_C_COMPILER_RISCV64 STREQUAL "")
            message(FATAL_ERROR
                "CMAKE_C_COMPILER_RISCV64 is not set. "
                "Please pass it on the command line, e.g.\n"
                "  -DCMAKE_C_COMPILER_RISCV64=/path/to/your/compiler"
            )
        endif()
        if(NOT DEFINED CMAKE_CXX_COMPILER_RISCV64 OR CMAKE_CXX_COMPILER_RISCV64 STREQUAL "")
            message(FATAL_ERROR
                "CMAKE_CXX_COMPILER_RISCV64 is not set. "
                "Please pass it on the command line, e.g.\n"
                "  -DCMAKE_CXX_COMPILER_RISCV64=/path/to/your/compiler"
            )
        endif()
        if(NOT DEFINED ATCMD_ZEPHYR_VENV OR ATCMD_ZEPHYR_VENV STREQUAL "")
            message(FATAL_ERROR
                "ATCMD_ZEPHYR_VENV is not set. "
                "Please pass it on the command line, e.g.\n"
                "  -ATCMD_ZEPHYR_VENV=/path/to/your/zephyr/python/virtual/environment/bin"
            )
        endif()


        include(ExternalProject)
        ExternalProject_Add(zephyr_example
          DOWNLOAD_COMMAND  "" # nothing to fetch
          SOURCE_DIR        "${CMAKE_SOURCE_DIR}/examples/zephyr_esp32_c3"
          BINARY_DIR        "${CMAKE_BINARY_DIR}/examples/zephyr_esp32_c3"

          CONFIGURE_COMMAND
            ${CMAKE_COMMAND} -E env
              "PATH=${ATCMD_ZEPHYR_VENV}:$ENV{PATH}"
            ${CMAKE_COMMAND}
              -S <SOURCE_DIR> -B <BINARY_DIR>
              -DBOARD=esp32c3_devkitm
              -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER_RISCV64}
              -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER_RISCV64}

          BUILD_COMMAND
            ${CMAKE_COMMAND} -E env
              "PATH=${ATCMD_ZEPHYR_VENV}:$ENV{PATH}"
            ${CMAKE_COMMAND} --build <BINARY_DIR>

          INSTALL_COMMAND ""
        )
        message(STATUS "Building Zephyr ESP32 C3 example")
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
