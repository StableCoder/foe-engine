# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(
  foe_simulation EXPORT_MACRO_NAME FOE_SIM_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/simulation/export.h)

target_sources(
  foe_simulation
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/simulation/export.h
         foe/simulation/group_data.hpp
         foe/simulation/registration.hpp
         foe/simulation/resource_create_info_history.h
         foe/simulation/resource_create_info_pool.h
         foe/simulation/result.h
         foe/simulation/simulation.hpp
         foe/simulation/type_defs.h)
