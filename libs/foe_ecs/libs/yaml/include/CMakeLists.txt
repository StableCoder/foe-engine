# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(foe_ecs_yaml EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/ecs/yaml/export.h)

target_sources(
  foe_ecs_yaml
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/ecs/yaml/export.h
         foe/ecs/yaml/id.hpp
         foe/ecs/yaml/indexes.hpp)
