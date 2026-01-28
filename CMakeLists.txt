cmake_minimum_required(VERSION 3.28)

# option() has lower priority than the user-supplied arguments
cmake_policy(SET CMP0077 NEW)

# Main project
project(atcmd
    VERSION 0.0.1
    LANGUAGES CXX)

# Option to build examples
option(BUILD_EXAMPLES "Build example applications" OFF)

# Add subdirectories
add_subdirectory(lib)

if(BUILD_EXAMPLES)
    add_subdirectory(examples/at_terminal)
    message(STATUS "Building examples")
else()
    message(STATUS "Examples disabled - not building at_terminal")
endif()
