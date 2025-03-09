# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(
  foe_graphics_vk_yaml EXPORT_MACRO_NAME FOE_GFX_VK_YAML_EXPORT
  EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/vk/yaml/export.h)

target_sources(
  foe_graphics_vk_yaml
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/vk/yaml/export.h
         foe/graphics/vk/yaml/structs.hpp
         foe/graphics/vk/yaml/vk_enums.hpp
         foe/graphics/vk/yaml/vk_pod.hpp
         foe/graphics/vk/yaml/vk_structs.hpp)
