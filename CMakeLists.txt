# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# Add external Drivers
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/drivers/lis2dh12)
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/drivers/tmp117)

# Add the board
set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ruuvitag)

target_sources(app PRIVATE src/main.c)

zephyr_include_directories(src)

add_subdirectory(src/led_handler)
add_subdirectory(src/button_handler)
add_subdirectory(src/board_info)
add_subdirectory(src/ruuvi_endpoint)
add_subdirectory(src/nfc_handler)
add_subdirectory(src/bt_handler)

###############Sensors###############
add_subdirectory(src/sensor_handler)
add_subdirectory(src/tmp117_handler)
add_subdirectory(src/bme280_handler)
add_subdirectory(src/lis2dh12_handler)
add_subdirectory(src/battery_handler)
add_subdirectory(src/shtcx_handler)
add_subdirectory(src/dps310_handler)