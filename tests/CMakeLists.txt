# Test configuration for AT Command Server Library

cmake_minimum_required(VERSION 3.28)

# Disable gtest install
set(INSTALL_GTEST OFF CACHE BOOL "Disable installation of googletest" FORCE)

# Fetch Google Test
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add test executable
add_executable(atcmd_tests
    trie.cpp
)
add_executable(atcmd::atcmd_tests ALIAS atcmd_tests)

# Link with our library and Google Test
target_link_libraries(atcmd_tests
    PRIVATE
    atcmd::atcmd
    GTest::gtest_main
)

target_compile_features(atcmd_tests PUBLIC cxx_std_23)
set_target_properties(atcmd_tests PROPERTIES CXX_EXTENSIONS OFF)

# Enable testing
enable_testing()

# Add test discovery
include(GoogleTest)
gtest_discover_tests(atcmd_tests)

if(ATCMD_BUILD_TESTS)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(atcmd_tests PRIVATE $<$<CXX_COMPILER_ID:Clang>:-fconstexpr-steps=20000000>)
    target_compile_options(atcmd_tests PRIVATE -fconstexpr-depth=20480)
  endif()
endif()

if(ATCMD_COVERAGE)
  find_program(LCOV_EXEC lcov)
  find_program(GENHTML_EXEC genhtml)
  if (LCOV_EXEC AND GENHTML_EXEC)
      if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # Turn off optimizations for nicer coverage reports
        target_compile_options(atcmd_tests PRIVATE -O0 -g --coverage)
        target_link_options(atcmd_tests PRIVATE --coverage)
      else()
        message(FATAL_ERROR "Coverage is only supported with GCC/Clang in this setup.")
      endif()

      # Automatically discover the GCC version
      execute_process(
        COMMAND ${CMAKE_C_COMPILER} -dumpfullversion -dumpversion
        OUTPUT_VARIABLE GCC_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
      # Take the major version (e.g. "14" from "14.2.0")
      string(REGEX MATCH "^[0-9]+" GCC_MAJOR "${GCC_VERSION}")

      set(LCOV_EXEC ${LCOV_EXEC} --gcov-tool /usr/bin/gcov-${GCC_MAJOR} --ignore-errors mismatch)
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
      message(FATAL_ERROR "Tests coverage report can not be generated because lcov and genhtml are not available")
  endif()
endif()
