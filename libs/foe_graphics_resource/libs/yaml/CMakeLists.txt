# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_graphics_resource_yaml)

# Declaration
add_library(foe_graphics_resource_yaml SHARED)
add_library(foe::graphics::resource::yaml ALIAS foe_graphics_resource_yaml)

# Definition
set_target_properties(foe_graphics_resource_yaml PROPERTIES DEBUG_POSTFIX "_d")

generate_export_header(
  foe_graphics_resource_yaml EXPORT_MACRO_NAME FOE_GFX_RES_YAML_EXPORT
  EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/resource/yaml/export.h)

target_link_libraries(
  foe_graphics_resource_yaml
  PUBLIC foe_ecs_yaml foe_graphics_resource foe_graphics_vk_yaml foe_imex_yaml
  PRIVATE vk_struct_compare)

target_code_coverage(foe_graphics_resource_yaml)

target_sources(
  foe_graphics_resource_yaml
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/resource/yaml/export.h
         include/foe/graphics/resource/yaml/export_registration.h
         include/foe/graphics/resource/yaml/import_registration.h
         include/foe/graphics/resource/yaml/result.h
         include/foe/graphics/resource/yaml/structs.hpp)

add_subdirectory(src)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_graphics_resource_yaml
             PROPERTY EXPORT_NAME foe::graphics::resource::yaml)

install(
  TARGETS foe_graphics_resource_yaml
  EXPORT foe_graphics_resource_yaml
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_graphics_resource_yaml DESTINATION share/cmake/foe)

install(FILES foe_graphics_resource_yaml-config.cmake
        DESTINATION share/cmake/foe)
