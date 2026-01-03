cmake_minimum_required(VERSION 3.5)

# Main project
project(atcmd LANGUAGES CXX)

# Compiler flags function
include(CheckCXXCompilerFlag)

function(enable_cxx_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_cxx_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
        unset(flag_supported CACHE)
    endif()
endfunction()

# Enable compiler warnings
enable_cxx_compiler_flag_if_supported("-Wall")
enable_cxx_compiler_flag_if_supported("-Wextra")
enable_cxx_compiler_flag_if_supported("-pedantic")

# Option to build examples
option(BUILD_EXAMPLES "Build example applications" ON)

# Add subdirectories
add_subdirectory(lib)

if(BUILD_EXAMPLES)
    add_subdirectory(examples/at_terminal)
    message(STATUS "Building examples")
else()
    message(STATUS "Examples disabled - not building at_terminal")
endif()
