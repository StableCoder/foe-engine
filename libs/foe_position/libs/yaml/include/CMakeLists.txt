# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(foe_position_yaml EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/position/yaml/export.h)

target_sources(
  foe_position_yaml
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/position/yaml/export.h
         foe/position/yaml/export_registration.h
         foe/position/yaml/import_registration.h
         foe/position/yaml/result.h
         foe/position/yaml/structs.hpp)
