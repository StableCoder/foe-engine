# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_graphics_yaml)

# Declaration
add_library(foe_graphics_yaml SHARED)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_graphics_yaml PUBLIC foe_graphics foe_yaml)

target_code_coverage(foe_graphics_yaml)

# install
set_property(TARGET foe_graphics_yaml PROPERTY EXPORT_NAME graphics_yaml)

install(
  TARGETS foe_graphics_yaml
  EXPORT foe_graphics_yaml
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_graphics_yaml
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_graphics_yaml-config.cmake DESTINATION share/cmake/foe)
