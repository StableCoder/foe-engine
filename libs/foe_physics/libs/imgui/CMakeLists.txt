# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)
project(foe_physics_imgui)

# Declaration
add_library(foe_physics_imgui)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_physics_imgui EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/physics/imgui/export.h)

target_include_directories(foe_physics_imgui
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(
  foe_physics_imgui PUBLIC foe_imgui foe_physics foe_resource_imgui
                           foe_simulation_imgui)

target_code_coverage(foe_physics_imgui)

if(MSVC)
  target_compile_options(foe_physics_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_physics_imgui PRIVATE -fno-exceptions)
endif()
