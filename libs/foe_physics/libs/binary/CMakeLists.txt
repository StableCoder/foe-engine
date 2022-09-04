# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)
project(foe_physics_binary)

# Declaration
add_library(foe_physics_binary SHARED)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_physics_binary EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/physics/binary/export.h)

target_include_directories(foe_physics_binary
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe_physics_binary PUBLIC foe_ecs foe_physics
                                                foe_imex_binary)

target_code_coverage(foe_physics_binary)

# Auxiliary Targets
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
