# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_graphics_resource_binary)

# Declaration
add_library(foe_graphics_resource_binary SHARED)
add_library(foe::graphics::resource::binary ALIAS foe_graphics_resource_binary)

# Definition
generate_export_header(
  foe_graphics_resource_binary EXPORT_MACRO_NAME FOE_GFX_RES_BINARY_EXPORT
  EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/resource/binary/export.h)

target_link_libraries(foe_graphics_resource_binary
                      PUBLIC foe_ecs foe_graphics_resource foe_imex_binary)

target_code_coverage(foe_graphics_resource_binary)

target_sources(
  foe_graphics_resource_binary
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/resource/binary/export.h
         include/foe/graphics/resource/binary/export_registration.h
         include/foe/graphics/resource/binary/import_registration.h
         include/foe/graphics/resource/binary/result.h)

add_subdirectory(src)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_graphics_resource_binary
             PROPERTY EXPORT_NAME foe::graphics::resource::binary)

install(
  TARGETS foe_graphics_resource_binary
  EXPORT foe_graphics_resource_binary
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_graphics_resource_binary DESTINATION share/cmake/foe)

install(FILES foe_graphics_resource_binary-config.cmake
        DESTINATION share/cmake/foe)
