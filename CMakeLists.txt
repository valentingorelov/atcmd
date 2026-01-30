cmake_minimum_required(VERSION 3.28)

# option() has lower priority than the user-supplied arguments
cmake_policy(SET CMP0077 NEW)

# Main project
project(atcmd
    VERSION 0.0.1
    LANGUAGES CXX)

# Add subdirectories
add_subdirectory(lib)

# Option to build examples
option(ATCMD_BUILD_EXAMPLES "Build example applications" OFF)

if(ATCMD_BUILD_EXAMPLES)
    add_subdirectory(examples/at_terminal)
    message(STATUS "Building examples")
else()
    message(STATUS "Examples disabled - not building at_terminal")
endif()

# Option to build tests
option(ATCMD_BUILD_TESTS "Build unit tests" OFF)

if(ATCMD_BUILD_TESTS)
    add_subdirectory(tests)
    message(STATUS "Building tests")
else()
    message(STATUS "Tests disabled - not building tests")
endif()

# Option to produce test coverage info
option(ATCMD_COVERAGE "Enable coverage flags (GCC/Clang only)" OFF)

if(ATCMD_BUILD_TESTS AND ATCMD_COVERAGE)
    find_program(LCOV_EXEC lcov)
    find_program(GENHTML_EXEC genhtml)
    if (LCOV_EXEC AND GENHTML_EXEC)
        set(LCOV_EXEC ${LCOV_EXEC} --gcov-tool /usr/bin/gcov-14 --ignore-errors mismatch)
        add_custom_target(atcmd_coverage ALL
            COMMAND ${CMAKE_COMMAND} -E rm -rf coverage coverage.info coverage.filtered.info
            COMMAND ${LCOV_EXEC} --zerocounters --directory .
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${LCOV_EXEC} --capture --directory . --output-file coverage.info --branch-coverage
            COMMAND ${LCOV_EXEC} --remove coverage.info
                    /usr/*
                    */_deps/*
                    --output-file coverage.filtered.info
            COMMAND ${GENHTML_EXEC} coverage.filtered.info
                    --output-directory coverage
                    --branch-coverage
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
            COMMENT "Running tests + generating lcov HTML report in build/coverage/"
            VERBATIM
        )
        add_dependencies(atcmd_coverage atcmd::atcmd atcmd::atcmd_tests)
    else()
        error("Tests coverage report can not be generated because lcov and genhtml are not available")
    endif()
endif()
