cmake_minimum_required(VERSION 3.28.0)

project(atcmd_zephyr_demo LANGUAGES CXX)

message(STATUS "ENV_PATH=$ENV{PATH}")
message(STATUS "ZEPHYR_BASE=$ENV{ZEPHYR_BASE}")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

target_sources(app PRIVATE src/main.cpp
    src/commands/extended/rgb.h src/commands/extended/rgb.cpp
    src/hwserial.h src/hwserial.cpp
    src/atserver.h src/atserver.cpp
    src/bleserial.h src/bleserial.cpp
    src/serial.h src/serial.cpp)

# A workaround to bypass Zephyr`s compiler check
# See: https://github.com/zephyrproject-rtos/zephyr/issues/36558#issuecomment-914038682
set(CMAKE_CXX_COMPILE_FEATURES cxx_std_23)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../lib atcmd_lib)
target_link_libraries(app PRIVATE atcmd)

target_compile_options(atcmd PUBLIC -std=c++23)
target_link_libraries(atcmd PUBLIC $<BUILD_INTERFACE:zephyr_interface>)


# Flashing the esp32 chip using the west utility
set(ESP_PORT "" CACHE STRING "Serial port for flashing (e.g., /dev/cu.wchusbserial110)")
set(FLASH_ARGS "")
if(NOT "${ESP_PORT}" STREQUAL "")
    set(FLASH_ARGS "--esp-device=${ESP_PORT}")
endif()
install(CODE "
    message(STATUS \"Flashing firmware to device...\")
    execute_process(
        COMMAND west flash --no-rebuild ${FLASH_ARGS}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
" COMPONENT flash)