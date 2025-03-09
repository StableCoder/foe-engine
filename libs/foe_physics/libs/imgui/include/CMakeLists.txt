# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(foe_physics_imgui EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/physics/imgui/export.h)

target_sources(
  foe_physics_imgui
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/physics/imgui/export.h
         foe/physics/imgui/registration.hpp)
