# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_graphics_vk_imgui)

# Declaration
add_library(foe_graphics_vk_imgui)
add_library(foe::grpahics::vk::imgui ALIAS foe_graphics_vk_imgui)

# Definition
generate_export_header(
  foe_graphics_vk_imgui EXPORT_MACRO_NAME FOE_GFX_VK_IMGUI_EXPORT
  EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/vk/imgui/export.h)

target_link_libraries(
  foe_graphics_vk_imgui
  PUBLIC foe_graphics_imgui foe_graphics_vk foe_imgui
  PRIVATE vk_struct_compare vk_value_serialization)

target_code_coverage(foe_graphics_vk_imgui)

if(MSVC)
  target_compile_options(foe_graphics_vk_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_graphics_vk_imgui PRIVATE -fno-exceptions)
endif()

target_sources(
  foe_graphics_vk_imgui
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/vk/imgui/export.h
         include/foe/graphics/vk/imgui/fragment_descriptor.hpp
         include/foe/graphics/vk/imgui/shader.hpp
         include/foe/graphics/vk/imgui/vertex_descriptor.hpp
         include/foe/graphics/vk/imgui/vk_struct.hpp
         include/foe/graphics/vk/imgui/vk_type.hpp)

add_subdirectory(src)

# install
set_property(TARGET foe_graphics_vk_imgui PROPERTY EXPORT_NAME
                                                   foe::graphics::vk::imgui)

install(
  TARGETS foe_graphics_vk_imgui
  EXPORT foe_graphics_vk_imgui
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_graphics_vk_imgui DESTINATION share/cmake/foe)

install(FILES foe_graphics_vk_imgui-config.cmake DESTINATION share/cmake/foe)
