# Copyright (C) 2022-2026 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_executable(test_foe_simulation)
add_test(NAME FoE-Simulation-Test COMMAND test_foe_simulation)

set_target_properties(test_foe_simulation PROPERTIES FOLDER "Tests")

# Definition
target_sources(test_foe_simulation PRIVATE group_data.cpp result.cpp
                                           simulation.cpp)

target_link_libraries(test_foe_simulation PRIVATE Catch2::Catch2WithMain
                                                  foe_simulation)

target_code_coverage(
  test_foe_simulation
  AUTO
  ALL
  OBJECTS
  foe_simulation
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)
