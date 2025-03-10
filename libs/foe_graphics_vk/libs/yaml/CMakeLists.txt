# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_graphics_vk_yaml)

# Declaration
add_library(foe_graphics_vk_yaml SHARED)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(
  foe_graphics_vk_yaml
  PUBLIC foe_ecs foe_graphics_yaml foe_graphics_vk foe_yaml
         $<BUILD_INTERFACE:vk_struct_cleanup>
         $<BUILD_INTERFACE:vk_value_serialization>)

target_code_coverage(foe_graphics_vk_yaml)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_graphics_vk_yaml PROPERTY EXPORT_NAME graphics_vk_yaml)

install(
  TARGETS foe_graphics_vk_yaml
  EXPORT foe_graphics_vk_yaml
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_graphics_vk_yaml
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_graphics_vk_yaml-config.cmake DESTINATION share/cmake/foe)
