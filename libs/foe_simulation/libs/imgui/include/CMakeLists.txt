# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(
  foe_simulation_imgui EXPORT_MACRO_NAME FOE_SIM_IMGUI_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/simulation/imgui/export.h)

target_sources(
  foe_simulation_imgui
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/simulation/imgui/export.h
         foe/simulation/imgui/group_data.hpp
         foe/simulation/imgui/registrar.hpp
         foe/simulation/imgui/result.h)
