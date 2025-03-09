# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(
  foe_physics_yaml EXPORT_MACRO_NAME FOE_PHYSICS_YAML_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/physics/yaml/export.h)

target_sources(
  foe_physics_yaml
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/physics/yaml/export.h
         foe/physics/yaml/export_registration.h
         foe/physics/yaml/import_registration.h
         foe/physics/yaml/result.h
         foe/physics/yaml/structs.hpp)
