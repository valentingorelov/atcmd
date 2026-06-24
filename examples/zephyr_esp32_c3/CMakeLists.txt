cmake_minimum_required(VERSION 3.28.0)

project(atcmd_zephyr_demo LANGUAGES CXX)

message(STATUS "ENV_PATH=$ENV{PATH}")
message(STATUS "ZEPHYR_BASE=$ENV{ZEPHYR_BASE}")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

target_sources(app PRIVATE src/main.cpp)

# A workaround to bypass Zephyr`s compiler check
# See: https://github.com/zephyrproject-rtos/zephyr/issues/36558#issuecomment-914038682
set(CMAKE_CXX_COMPILE_FEATURES cxx_std_23)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../lib atcmd_lib)
target_link_libraries(app PRIVATE atcmd)

target_compile_options(atcmd PUBLIC -std=c++23)
target_link_libraries(atcmd PUBLIC $<BUILD_INTERFACE:zephyr_interface>)
