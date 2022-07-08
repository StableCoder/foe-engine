# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)
project(foe_position_imgui)

# Declaration
add_library(foe_position_imgui)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_position_imgui EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/position/imgui/export.h)

target_include_directories(foe_position_imgui
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe_position_imgui PUBLIC foe_imgui foe_position
                                                foe_simulation_imgui)

target_code_coverage(foe_position_imgui)

if(MSVC)
  target_compile_options(foe_position_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_position_imgui PRIVATE -fno-exceptions)
endif()
