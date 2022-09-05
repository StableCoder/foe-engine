# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_physics_yaml)
add_test(NAME FoE-Physics-Yaml-Test COMMAND test_foe_physics)

# Definition
target_sources(test_foe_physics_yaml PRIVATE c_header_compatibility.c
                                             result.cpp)

target_link_libraries(test_foe_physics_yaml PRIVATE catch foe_physics_yaml)

target_code_coverage(
  test_foe_physics_yaml
  AUTO
  ALL
  OBJECTS
  foe_physics_yaml
  EXCLUDE
  .*/test/.*
  ${BULLET_INCLUDE_DIRS})