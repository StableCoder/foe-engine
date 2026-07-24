# Copyright (C) 2022-2026 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_executable(test_foe_position_yaml)
add_test(NAME FoE-Position-Yaml-Test COMMAND test_foe_position_yaml)

set_target_properties(test_foe_position_yaml PROPERTIES FOLDER "Tests")

# Definition
target_sources(test_foe_position_yaml PRIVATE result.cpp)

target_link_libraries(test_foe_position_yaml PRIVATE Catch2::Catch2WithMain
                                                     foe_position_yaml)

target_code_coverage(
  test_foe_position_yaml
  AUTO
  ALL
  OBJECTS
  foe_position_yaml
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*
  ${BULLET_INCLUDE_DIRS})
