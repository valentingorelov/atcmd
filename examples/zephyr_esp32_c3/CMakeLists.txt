cmake_minimum_required(VERSION 3.28.0)

message(STATUS "ENV_PATH=$ENV{PATH}")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(atcmd_zephyr_demo LANGUAGES CXX)

target_sources(app PRIVATE src/main.cpp)

# A workaround to bypass Zephyr`s compiler check
# See: https://github.com/zephyrproject-rtos/zephyr/issues/36558#issuecomment-914038682
set(CMAKE_CXX_COMPILE_FEATURES cxx_std_23)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../lib atcmd_lib)
target_link_libraries(app PRIVATE atcmd)

target_compile_options(atcmd PUBLIC -std=c++23)
