# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(foe_model EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/model/export.h)

target_sources(
  foe_model
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/model/export.h
         foe/model/animation.hpp
         foe/model/armature.hpp
         foe/model/cube.hpp
         foe/model/ico_sphere.hpp
         foe/model/importer.hpp
         foe/model/vertex_component.hpp)
