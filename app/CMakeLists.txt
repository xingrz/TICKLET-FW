# Copyright (c) 2025 XiNGRZ <hi@xingrz.me>
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(APP_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND BOARD_ROOT ${APP_ROOT})
list(APPEND DTS_ROOT ${APP_ROOT})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ticklet)

zephyr_include_directories(include)

target_sources(app PRIVATE
  main.c
)
