# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(foe_imex EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/imex/export.h)

target_sources(
  foe_imex
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/imex/export.h
         foe/imex/exporters.h
         foe/imex/importer.h
         foe/imex/result.h
         foe/imex/type_defs.h)
