# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(
  foe_graphics_resource_yaml EXPORT_MACRO_NAME FOE_GFX_RES_YAML_EXPORT
  EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/resource/yaml/export.h)

target_sources(
  foe_graphics_resource_yaml
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/resource/yaml/export.h
         foe/graphics/resource/yaml/export_registration.h
         foe/graphics/resource/yaml/import_registration.h
         foe/graphics/resource/yaml/result.h
         foe/graphics/resource/yaml/structs.hpp)
