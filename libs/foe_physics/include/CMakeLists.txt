# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(foe_physics EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/physics/export.h)

target_sources(
  foe_physics
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/physics/export.h
         foe/physics/component/rigid_body.h
         foe/physics/component/rigid_body_pool.h
         foe/physics/resource/collision_shape_create_info.hpp
         foe/physics/resource/collision_shape.hpp
         foe/physics/binary.h
         foe/physics/compare.h
         foe/physics/registration.h
         foe/physics/result.h
         foe/physics/system.h
         foe/physics/type_defs.h)
