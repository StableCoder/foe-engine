# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(
  foe_graphics_resource_binary EXPORT_MACRO_NAME FOE_GFX_RES_BINARY_EXPORT
  EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/resource/binary/export.h)

target_sources(
  foe_graphics_resource_binary
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/resource/binary/export.h
         foe/graphics/resource/binary/export_registration.h
         foe/graphics/resource/binary/import_registration.h
         foe/graphics/resource/binary/result.h)
