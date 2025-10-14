# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_graphics_vk_yaml)

# Declaration
add_library(foe_graphics_vk_yaml SHARED)
add_library(foe::graphics::vk::yaml ALIAS foe_graphics_vk_yaml)

# Definition
generate_export_header(
  foe_graphics_vk_yaml EXPORT_MACRO_NAME FOE_GFX_VK_YAML_EXPORT
  EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/vk/yaml/export.h)

target_link_libraries(
  foe_graphics_vk_yaml
  PUBLIC foe_ecs foe_graphics_yaml foe_graphics_vk foe_yaml
  PRIVATE vk_struct_cleanup vk_struct_compare vk_value_serialization)

target_code_coverage(foe_graphics_vk_yaml)

target_sources(
  foe_graphics_vk_yaml
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/vk/yaml/export.h
         include/foe/graphics/vk/yaml/structs.hpp
         include/foe/graphics/vk/yaml/vk_enums.hpp
         include/foe/graphics/vk/yaml/vk_pod.hpp
         include/foe/graphics/vk/yaml/vk_structs.hpp)

add_subdirectory(src)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_graphics_vk_yaml PROPERTY EXPORT_NAME
                                                  foe::graphics::vk::yaml)

install(
  TARGETS foe_graphics_vk_yaml
  EXPORT foe_graphics_vk_yaml
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_graphics_vk_yaml DESTINATION share/cmake/foe)

install(FILES foe_graphics_vk_yaml-config.cmake DESTINATION share/cmake/foe)
