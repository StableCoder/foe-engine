# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_physics)
add_test(NAME FoE-Physics-Test COMMAND test_foe_physics)

# Definition
target_sources(test_foe_physics PRIVATE result.cpp)

target_link_libraries(test_foe_physics PRIVATE catch foe_physics)

target_code_coverage(
  test_foe_physics
  AUTO
  ALL
  OBJECTS
  foe_physics
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*
  ${BULLET_INCLUDE_DIRS})
