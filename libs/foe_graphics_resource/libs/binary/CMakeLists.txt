# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)
project(foe_graphics_resource_binary)

# Declaration
add_library(foe_graphics_resource_binary SHARED)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_graphics_resource_binary EXPORT_MACRO_NAME FOE_GFX_RES_BINARY_EXPORT
  EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/graphics/resource/binary/export.h)

target_include_directories(foe_graphics_resource_binary
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe_graphics_resource_binary
                      PUBLIC foe_ecs foe_graphics_resource foe_imex_binary)

target_code_coverage(foe_graphics_resource_binary)

# Auxiliary Targets
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
