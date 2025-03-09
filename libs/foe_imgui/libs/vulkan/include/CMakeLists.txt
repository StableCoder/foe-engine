# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(foe_imgui_vk EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/imgui/vk/export.h)

target_sources(
  foe_imgui_vk
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/imgui/vk/export.h
         foe/imgui/vk/renderer.hpp
         foe/imgui/vk/render_graph_job_imgui.hpp
         foe/imgui/vk/result.h)
