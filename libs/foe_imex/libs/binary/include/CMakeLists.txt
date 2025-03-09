# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(foe_imex_binary EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/imex/binary/export.h)

target_sources(
  foe_imex_binary
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/imex/binary/export.h
         foe/imex/binary/exporter.h
         foe/imex/binary/importer.h
         foe/imex/binary/result.h
         foe/imex/binary/type_defs.h)
