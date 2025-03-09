# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(
  foe_graphics_resource_imgui EXPORT_MACRO_NAME FOE_GFX_RES_IMGUI_EXPORT
  EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/resource/imgui/export.h)

target_sources(
  foe_graphics_resource_imgui
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/resource/imgui/export.h
         foe/graphics/resource/imgui/registration.hpp)
