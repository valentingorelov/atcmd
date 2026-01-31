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
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Turn off optimizations for nicer coverage reports
    target_compile_options(atcmd_tests PRIVATE -O0 -g --coverage)
    target_link_options(atcmd_tests PRIVATE --coverage)
  else()
    message(FATAL_ERROR "Coverage is only supported with GCC/Clang in this setup.")
  endif()
endif()
