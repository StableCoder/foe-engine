# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)
project(foe_graphics_vk_yaml)

# Declaration
add_library(foe_graphics_vk_yaml SHARED)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_graphics_vk_yaml EXPORT_MACRO_NAME FOE_GFX_VK_YAML_EXPORT
  EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/graphics/vk/yaml/export.h)

target_include_directories(
  foe_graphics_vk_yaml PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public
                              ../../src)

target_link_libraries(
  foe_graphics_vk_yaml PUBLIC foe_ecs foe_graphics_yaml foe_graphics_vk
                              foe_yaml vk_struct_cleanup vk_value_serialization)

target_code_coverage(foe_graphics_vk_yaml)

# Auxiliary Targets
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
