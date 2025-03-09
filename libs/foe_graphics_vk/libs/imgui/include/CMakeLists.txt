# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(
  foe_graphics_vk_imgui EXPORT_MACRO_NAME FOE_GFX_VK_IMGUI_EXPORT
  EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/vk/imgui/export.h)

target_sources(
  foe_graphics_vk_imgui
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/vk/imgui/export.h
         foe/graphics/vk/imgui/fragment_descriptor.hpp
         foe/graphics/vk/imgui/shader.hpp
         foe/graphics/vk/imgui/vertex_descriptor.hpp
         foe/graphics/vk/imgui/vk_struct.hpp
         foe/graphics/vk/imgui/vk_type.hpp)
