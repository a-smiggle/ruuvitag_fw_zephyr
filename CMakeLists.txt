# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(ruuvitag)

target_sources(app PRIVATE src/main.c)
zephyr_include_directories(src)

add_subdirectory(src/led_handler)
add_subdirectory(src/button_handler)
add_subdirectory(src/bme280_handler)
add_subdirectory(src/lis2dh12_handler)
add_subdirectory(src/battery_handler)
add_subdirectory(src/board_info)
add_subdirectory(src/ruuvi_endpoint)
add_subdirectory(src/nfc_handler)
add_subdirectory(src/bt_handler)