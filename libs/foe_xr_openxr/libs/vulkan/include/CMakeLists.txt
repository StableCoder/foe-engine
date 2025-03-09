# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(
  foe_xr_openxr_vk EXPORT_MACRO_NAME FOE_OPENXR_VK_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/xr/openxr/vk/export.h)

target_sources(
  foe_xr_openxr_vk
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/xr/openxr/vk/export.h
         foe/xr/openxr/vk/render_graph_jobs_swapchain.hpp
         foe/xr/openxr/vk/result.h
         foe/xr/openxr/vk/vulkan.h)
