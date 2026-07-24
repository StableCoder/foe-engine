# Copyright (C) 2022-2026 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_executable(test_foe_physics_binary)
add_test(NAME FoE-Physics-Binary-Test COMMAND test_foe_physics_binary)

set_target_properties(test_foe_physics_binary PROPERTIES FOLDER "Tests")

# Definition
target_sources(test_foe_physics_binary PRIVATE result.cpp)

target_link_libraries(test_foe_physics_binary PRIVATE Catch2::Catch2WithMain
                                                      foe_physics_binary)

target_code_coverage(
  test_foe_physics_binary
  AUTO
  ALL
  OBJECTS
  foe_physics_binary
  EXCLUDE
  .*/test/.*)
