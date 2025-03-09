# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(foe_model_assimp EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/model/assimp/export.h)

target_sources(
  foe_model_assimp
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/model/assimp/export.h
         foe/model/assimp/flags.hpp
         foe/model/assimp/importer.hpp)
