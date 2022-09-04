# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)
project(foe_position_binary)

# Declaration
add_library(foe_position_binary SHARED)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_position_binary EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/position/binary/export.h)

target_include_directories(foe_position_binary
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe_position_binary PUBLIC foe_ecs foe_position
                                                 foe_imex_binary)

target_code_coverage(foe_position_binary)

# Auxiliary Targets
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
