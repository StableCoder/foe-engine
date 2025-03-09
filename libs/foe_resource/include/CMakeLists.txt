# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(
  foe_resource EXPORT_MACRO_NAME FOE_RES_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/resource/export.h)

target_sources(
  foe_resource
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/resource/export.h
         foe/resource/create_info.h
         foe/resource/pool.h
         foe/resource/resource_fns.h
         foe/resource/resource.h
         foe/resource/result.h
         foe/resource/type_defs.h)
