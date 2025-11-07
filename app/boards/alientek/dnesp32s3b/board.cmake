# Copyright (c) 2025 XiNGRZ <hi@xingrz.me>
# SPDX-License-Identifier: Apache-2.0

board_runner_args(esp32 "--esp-baud-rate=3000000")

if(NOT "${OPENOCD}" MATCHES "^${ESPRESSIF_TOOLCHAIN_PATH}/.*")
  set(OPENOCD OPENOCD-NOTFOUND)
endif()
find_program(OPENOCD openocd PATHS ${ESPRESSIF_TOOLCHAIN_PATH}/openocd-esp32/bin NO_DEFAULT_PATH)

include(${ZEPHYR_BASE}/boards/common/esp32.board.cmake)
include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
