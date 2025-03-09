# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(foe_position EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/position/export.h)

target_sources(
  foe_position
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/position/export.h
         foe/position/component/3d.hpp
         foe/position/component/3d_pool.h
         foe/position/binary.h
         foe/position/compare.h
         foe/position/registration.h
         foe/position/result.h
         foe/position/type_defs.h)
