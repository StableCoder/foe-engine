# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_executable(test_foe_physics_yaml)
add_test(NAME FoE-Physics-Yaml-Test COMMAND test_foe_physics)

# Definition
target_sources(test_foe_physics_yaml PRIVATE c_header_compatibility.c
                                             result.cpp)

target_link_libraries(test_foe_physics_yaml PRIVATE Catch2::Catch2WithMain
                                                    foe_physics_yaml)

target_code_coverage(
  test_foe_physics_yaml
  AUTO
  ALL
  OBJECTS
  foe_physics_yaml
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*
  ${BULLET_INCLUDE_DIRS})
