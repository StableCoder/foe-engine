# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_graphics_resource_imgui)

# Declaration
add_library(foe_graphics_resource_imgui)
add_library(foe::graphics::resource::imgui ALIAS foe_graphics_resource_imgui)

# Definition
generate_export_header(
  foe_graphics_resource_imgui EXPORT_MACRO_NAME FOE_GFX_RES_IMGUI_EXPORT
  EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/resource/imgui/export.h)

target_link_libraries(
  foe_graphics_resource_imgui
  PUBLIC foe_graphics_resource foe_graphics_vk_imgui foe_imgui
         foe_resource_imgui foe_simulation_imgui)

target_code_coverage(foe_graphics_resource_imgui)

if(MSVC)
  target_compile_options(foe_graphics_resource_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_graphics_resource_imgui PRIVATE -fno-exceptions)
endif()

target_sources(
  foe_graphics_resource_imgui
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/resource/imgui/export.h
         include/foe/graphics/resource/imgui/registration.hpp)

add_subdirectory(src)

# install
set_property(TARGET foe_graphics_resource_imgui
             PROPERTY EXPORT_NAME graphics::resource::imgui)

install(
  TARGETS foe_graphics_resource_imgui
  EXPORT foe_graphics_resource_imgui
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_graphics_resource_imgui
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_graphics_resource_imgui-config.cmake
        DESTINATION share/cmake/foe)
