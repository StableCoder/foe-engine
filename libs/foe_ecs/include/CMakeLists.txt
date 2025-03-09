# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(foe_ecs EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/ecs/export.h)

target_sources(
  foe_ecs
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/ecs/export.h
         foe/ecs/binary.h
         foe/ecs/component_pool.h
         foe/ecs/entity_list.h
         foe/ecs/group_translator.h
         foe/ecs/id.h
         foe/ecs/id_to_string.hpp
         foe/ecs/indexes.h
         foe/ecs/name_map.h
         foe/ecs/result.h)
