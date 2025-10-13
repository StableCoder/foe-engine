# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_graphics_yaml)

# Declaration
add_library(foe_graphics_yaml SHARED)
add_library(foe::graphics::yaml ALIAS foe_graphics_yaml)

# Definition
generate_export_header(
  foe_graphics_yaml EXPORT_MACRO_NAME FOE_GFX_YAML_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/yaml/export.h)

target_link_libraries(foe_graphics_yaml PUBLIC foe_graphics foe_yaml)

target_code_coverage(foe_graphics_yaml)

target_sources(
  foe_graphics_yaml
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/yaml/export.h
         include/foe/graphics/yaml/enums.hpp)

add_subdirectory(src)

# install
set_property(TARGET foe_graphics_yaml PROPERTY EXPORT_NAME graphics::yaml)

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
